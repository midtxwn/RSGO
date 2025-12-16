#include <iostream>
#include <stdio.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <random>

#include "esfera.h"


#include "definitions.h"
#include "network.h"
#include "lecturaShader_0_9.h"
#include "externObject.h"


double prevTime, prevTimeDecals;
std::vector<entity*> entityList;
GLuint shaderProgram;
//GLuint anilloShaderProgram;
GLuint noLightShaderProgram;
//GLuint noLightRingShaderProgram;

GLuint setShaders(const char* nVertx, const char* nFrag);
void keyEvents(GLFWwindow* window, int key, int scancode, int action, int mods);
void windowResize(GLFWwindow* window, int width, int height);
void openGlInit();
unsigned int vaoCuadrado();
void mouseEvents(GLFWwindow* window, double xpos, double ypos);
void mouseClick(GLFWwindow* window, int button, int action, int mods);
bool rayCast(glm::vec3& lineVector, glm::vec3& linePoint, glm::vec3& planeNormalVector, glm::vec3& planePoint, glm::vec3* intersectionPoint);
void checkPlayerHit(glm::vec3 lookingAt, glm::vec3 cameraPos, glm::vec3 position);
void shotReceived(enum entity_manager::ENDGAME_STATES state);


GLFWwindow* window;
int fmove = 0, smove = 0;
glm::vec2 intendedMovingDirection = { 0,0 };
glm::vec3 lookingAtVec = { 1,0,0 };
glm::vec3 cameraHeight;
glm::vec3 recoilPushback = { -1,0,0 };
glm::vec3 halfCameraHeight;
glm::vec2 accelerationVec = { 0 ,0 };
glm::vec2 speedVec = { 0,0 };
glm::mat4 staticLookAt = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
int SCR_WIDTH = 800;
int SCR_HEIGHT = 400;
unsigned int VAOCuadrado;
float lightColor[4] = { 1.f, 1.f, 1.f, 1.f };
float lightPos[3] = { 0.f, 5.f, 0.f };
bool ABeingHeld = false, DBeingHeld = false, WBeingHeld = false, SBeingHeld = false;
bool topview = false, fullScreen = false;
auto MAX_SPEED = 15;
int localHealth = PLAYER_HEALTH;
int foreignHealth = PLAYER_HEALTH;


GLuint fireDecals[45];
std::pair<unsigned int, unsigned int> endgameTextures;
enum entity_manager::ENDGAME_STATES endGame = entity_manager::NOT_FINISHED;


std::vector<GLuint> activeFireDecals; //textura del decal
GLuint hurtDecal;
std::vector<std::pair<GLuint,float>> activeDamageDecals; //textura del decal


inline float lenghtVec2(glm::vec2 vec)
{
	double squaredx, squaredy;
	squaredx = pow(vec.x, 2);
	squaredy = pow(vec.y, 2);

	return sqrt(((squaredx < 0) ? 0:squaredx) + ((squaredy < 0) ? 0 : squaredy));
		
}


bool collidesWithMap(std::pair<glm::vec3, glm::vec3>& playerBound)
{

	std::vector<component> mapComponents = entityList.at(entity_manager::MAP)->components;

	for (component c : mapComponents)
	{

		std::pair<glm::vec3, glm::vec3>& mapComponentBound = c.bounds;

		if (mapComponentBound.second.x < playerBound.first.x || mapComponentBound.first.x > playerBound.second.x) continue;
		if (mapComponentBound.second.y < playerBound.first.y || mapComponentBound.first.y > playerBound.second.y) continue;
		if (mapComponentBound.second.z < playerBound.first.z || mapComponentBound.first.z > playerBound.second.z) continue;

		// If none of the above conditions are met, the boxes must be colliding
		//printf("Colliding! %d\n", entityList.at(entity_manager::LOCAL_PLAYER)->eulerAngles.alpha);
		return true;
	
	}
	
	return false;
}




void updateDecals(double& prevTime)
{
	double currTime = glfwGetTime();
	double deltaTime = (currTime - prevTime) / SLOWDOWN_TIME;
	if (deltaTime < SHOOT_DECAL_TIME)
		return;
	prevTime = currTime;


	if (prevTime > INACCURATE_TIME_THRESHOLD)
	{
		glfwSetTime(0);
		prevTime = 0;
	}

	for (int i = activeDamageDecals.size() - 1; i >= 0; --i)
	{

		float& alpha = (activeDamageDecals.at(i)).second;
		alpha -= HURT_ALPHA_REDUCTION;
		if (alpha < TOLERANCE)
			activeDamageDecals.erase(activeDamageDecals.begin() + i);

	}

	for (int i = activeFireDecals.size() - 1; i >= 0; --i)
		{
			
			GLuint& decal = activeFireDecals.at(i);
			decal++;
			if (decal > SHOOT_DECAL_TEXTURE_NUM)
				activeFireDecals.erase(activeFireDecals.begin() + i);

		}
	recoilPushback = (float)0.8 * recoilPushback;
	
}
void updateLocalPlayerPhysics(double& prevTime, entity& localPlayer)
{
	//component& chest = localPlayer.components.at(entity_manager::)
	double currTime = glfwGetTime();
	double deltaTime = (currTime - prevTime) / SLOWDOWN_TIME;
	if (deltaTime < UPDATE_PHYSICS_INTERVAL)
		return;
	prevTime = currTime;


	if (prevTime > INACCURATE_TIME_THRESHOLD)
	{
		glfwSetTime(0);
		prevTime = 0;
	}



	glm::vec3& pos = localPlayer.position;

	glm::vec2 lookDireccion;
	glm::vec2 rightParallelLookDireccion;
	glm::vec2 res1, res2;
	intendedMovingDirection = { 0,0 };


	if(fmove != 0 || smove != 0)
	{
		double cosAlpha = cos(localPlayer.eulerAngles.alpha);
		double sinAlpha = sin(localPlayer.eulerAngles.alpha);
		lookDireccion = { cosAlpha , sinAlpha };
		rightParallelLookDireccion = { -sinAlpha, cosAlpha };

		if (fmove == 0)
			res1 = { 0,0 };
		else res1 = (float)fmove * lookDireccion;

		if (smove == 0)
			res2 = { 0,0 };
		else res2 = (float)smove * rightParallelLookDireccion;
		res1 = res1 + res2;
		if(glm::length(res1) > TOLERANCE)
			intendedMovingDirection = glm::normalize(res1);
		
	}
	glm::vec2 wishedSpeed = speedVec + ((float)PLAYER_ACCEL * ((float)deltaTime) * intendedMovingDirection);

	float speedModulus = lenghtVec2(wishedSpeed);
	if (speedModulus > MAX_SPEED)
	{
		speedVec = wishedSpeed * (MAX_SPEED / speedModulus);

	}
	else speedVec = wishedSpeed;
	//speedVec += (accelerationVec.x * direccion) + (accelerationVec.y * direccion)
	 //nota: entre intervalos de tiempo muy grande, puede dar problemas porque asume que la aceleracion lleva siendo tal desde el inicio del intervalo
	

	//double speedSquared = pow(speedVec.x, 2) + pow(speedVec.z, 2);
	//if (speedSquared > MAX_SPEED_SQUARED)
	//{
	
	//}
	glm::vec3 wishedPos = { 
		pos.x + speedVec.x * deltaTime,
		pos.y,
		pos.z + speedVec.y * deltaTime };

	std::pair<glm::vec3, glm::vec3> playerBound = localPlayer.components.at(entity_manager::AK_ARM_R).bounds;

	std::pair<glm::vec3, glm::vec3> bounds = std::make_pair(
			playerBound.first + wishedPos, 
			playerBound.second + wishedPos);
	
	if (!collidesWithMap(bounds))
		pos = wishedPos;
	else speedVec = { 0,0 };
	//printf("(%f,%f,%f)\n", pos.x, pos.y, pos.z);

	double retainedSpeed = pow((ONE_MINUS_FRICTION), deltaTime + EXTRA_TIME_FOR_FRICTION_CALC);
	//printf("FRICTION %lf\n", retainedSpeed);
	speedVec = { speedVec.x * retainedSpeed,  speedVec.y * retainedSpeed };

}



bool checkPlayerHit(glm::vec3 lookingAt, glm::vec3 position)
{

	entity* jugadorRemoto = entityList.at(entity_manager::FOREIGN_PLAYER);
	std::vector<component> componentesJugador = jugadorRemoto->components;
	glm::vec3 intersection = {INFINITY,0,0};
	glm::vec3 newIntersection = {0,0,0};
	glm::vec3 canonicalAxis[NUM_AXIS_COLLISION_CHECK] = { {1,0,0}, {0,0,1}, {0,1,0} };
	glm::vec2 index[] = { {1,2},{0,1},{0,2} };

	for (component& c : componentesJugador)  //TODO: Checkear con las boundings adecuadas dependiendo de la postura del muñeco
	{
		std::pair<glm::vec3, glm::vec3> bounds = { c.bounds.first + jugadorRemoto->position, c.bounds.second + jugadorRemoto->position };
		for(int i = 0; i < NUM_PLANES_FIRSTBOUND_COLLISION_CHECK; ++i){
			
			glm::vec2 ind = index[i];
			

			if (rayCast(lookingAt, position, canonicalAxis[i], bounds.first, &newIntersection) && 
				newIntersection[ind[0]] >= bounds.first[ind[0]] && newIntersection[ind[0]] <= bounds.second[ind[0]] &&
				newIntersection[ind[1]] >= bounds.first[ind[1]] && newIntersection[ind[1]] <= bounds.second[ind[1]]) //TODO CAMERA POS CREO K HAY  K SUMARLE EL POSITION DEL JUGADOR
				intersection = ((glm::length(newIntersection) < glm::length(intersection)) ? newIntersection : intersection);	
		}
		for(int i = 0 ; i < NUM_PLANES_SECONDBOUND_COLLISION_CHECK; ++i){

			glm::vec2 ind = index[i];

			if (rayCast(lookingAt, position, canonicalAxis[i], bounds.second, &newIntersection) &&
				newIntersection[ind[0]] >= bounds.first[ind[0]] && newIntersection[ind[0]] <= bounds.second[ind[0]] &&
				newIntersection[ind[1]] >= bounds.first[ind[1]] && newIntersection[ind[1]] <= bounds.second[ind[1]]) //TODO CAMERA POS CREO K HAY  K SUMARLE EL POSITION DEL JUGADOR
				intersection = ((glm::length(newIntersection) < glm::length(intersection)) ? newIntersection : intersection);
		}
	}

	if (intersection.x != INFINITY)
	{
		printf("ENEMY HIT!\n");
		return true;
	}
	return false;
		


	//for(component& c : (entityList.at(entity_manager::MAP))->components	)
	//	{
			




		//}

}


bool rayCast(glm::vec3& lineVector, glm::vec3& linePoint, glm::vec3& planeNormalVector, glm::vec3& planePoint, glm::vec3* intersectionPoint)
{

	float lTimesN = glm::dot(lineVector, planeNormalVector);
	if (abs(lTimesN) < TOLERANCE)
		return false;
	float distance = glm::dot((planePoint - linePoint), planeNormalVector)/lTimesN;
	
	if (distance < 0) return false;

	*intersectionPoint = linePoint + distance * lineVector;

	return true;
}



void myCamara(glm::mat4& view, glm::mat4& projection)
{

	if (topview) {

		view = glm::lookAt(glm::vec3(0, 50, 0), glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0));

		projection = glm::ortho(-30.f, 30.f, -30.f, 30.f, .1f, 100.f);

	} else {

		glm::vec3 lookAtPoint = {
		(entityList.at(entity_manager::LOCAL_PLAYER)->position.x) + lookingAtVec.x,
		(entityList.at(entity_manager::LOCAL_PLAYER)->position.y) + lookingAtVec.y ,
		(entityList.at(entity_manager::LOCAL_PLAYER)->position.z) + lookingAtVec.z
		};

		view = glm::lookAt(
			entityList.at(entity_manager::LOCAL_PLAYER)->position + cameraHeight,
			lookAtPoint + cameraHeight,
			glm::vec3(0, 1, 0)
		);

		projection = glm::perspective(90.0f, 1.0f, 0.1f, 10000.0f);
	}
}

/*
* Decide si en base a una entidad, un índice de componente y un modo el componente se debe dibujar o no "0 local, 1 foreign"
*/
bool drawComponentCondition(entity e, int componentIndex, int mode){  //TODO: Decidir cómo hacer bounding box del jugador local (cilindro objeto que no se muestre?)

	switch(mode){
	case 0:
		//Switch de armas
		if (
			((
				componentIndex == entity_manager::AK_LOCAL || 
				componentIndex == entity_manager::AK_ARM_R || 
				componentIndex == entity_manager::AK_ARM_L
				) && e.weapon != entity_manager::WEAPON_AK) 
			||
			((
				componentIndex == entity_manager::KNIFE_LOCAL ||
				componentIndex == entity_manager::KNIFE_ARM_R ||
				componentIndex == entity_manager::KNIFE_ARM_L
				) && e.weapon != entity_manager::WEAPON_KNIFE)
		)
			return false;

		break;
	case 1:
		//Switch de armas
		if ((componentIndex == entity_manager::AK && e.weapon != entity_manager::WEAPON_AK) ||
			(componentIndex == entity_manager::KNIFE && e.weapon != entity_manager::WEAPON_KNIFE))
			return false;

		//Switch de posturas
		if ((e.crouching && (componentIndex == entity_manager::EXTENDED_LEG_L || componentIndex == entity_manager::EXTENDED_LEG_R)) ||
			(!(e.crouching) && (componentIndex == entity_manager::BENT_LEG_L || componentIndex == entity_manager::BENT_LEG_R)))
			return false;

		break;
	}	

	return true;
}



void drawShootDecals(glm::mat4 projection)
{
	if (activeFireDecals.empty()) return;
	glUseProgram(noLightShaderProgram);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	GLuint transformLoc = glGetUniformLocation(noLightShaderProgram, "transform");
	GLuint projectionLoc = glGetUniformLocation(noLightShaderProgram, "projection");
	GLuint viewLoc = glGetUniformLocation(noLightShaderProgram, "view");
	GLuint alphaLoc = glGetUniformLocation(noLightShaderProgram, "alphaChannelFactor");
	
	for (GLuint decal : activeFireDecals)
	{
		glm::mat4 translateMat = glm::mat4();
		
		//translateMat = glm::translate(translateMat, decalPair.second);
		translateMat = glm::translate(translateMat, glm::vec3(4, -0.75, 1));
		translateMat = glm::scale(translateMat, glm::vec3(0.7, 0.7, 1.2));
		//translateMat = glm::rotate(translateMat,entityList.at(entity_manager::LOCAL_PLAYER)->eulerAngles.alpha, glm::vec3(0, 1, 0));
		//glm::mat4 transformComponent = glm::translate(transformEntity, c.position);
		//glm::mat4 transformComponent = glm::translate(glm::mat4(1.0), -halfCameraHeight);

		glBindVertexArray(VAOCuadrado);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_LINES  |  GL_FILL
		//glDisableVertexAttribArray(3);
		//glVertexAttrib4d(3, 0, c.color.y, c.color.z, c.alphaFactor);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::lookAt(glm::vec3(0,0,0) - recoilPushback,glm::vec3(1,0,0),glm::vec3(0,1,0))));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(translateMat));
		glUniform1f(alphaLoc, 1);


		glBindTexture(GL_TEXTURE_2D, decal);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glEnableVertexAttribArray(3);
		glBindVertexArray(0);

	}
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}


void drawDamageDecals(glm::mat4 projection)
{
	if (activeDamageDecals.empty()) return;
	glUseProgram(noLightShaderProgram);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	GLuint alphaLoc = glGetUniformLocation(noLightShaderProgram, "alphaChannelFactor");
	GLuint transformLoc = glGetUniformLocation(noLightShaderProgram, "transform");
	GLuint projectionLoc = glGetUniformLocation(noLightShaderProgram, "projection");
	GLuint viewLoc = glGetUniformLocation(noLightShaderProgram, "view");

	for (std::pair<GLuint,float> pair : activeDamageDecals)
	{
		glm::mat4 translateMat = glm::scale(glm::mat4(),HURT_DECAL_SCALE);

		//translateMat = glm::translate(translateMat, decalPair.second);
		//translateMat = glm::translate(translateMat, glm::vec3(4, -0.75, 1));
		//translateMat = glm::scale(translateMat, glm::vec3(2, 2, 5));
		//translateMat = glm::rotate(translateMat,entityList.at(entity_manager::LOCAL_PLAYER)->eulerAngles.alpha, glm::vec3(0, 1, 0));
		//glm::mat4 transformComponent = glm::translate(transformEntity, c.position);
		//glm::mat4 transformComponent = glm::translate(glm::mat4(1.0), -halfCameraHeight);

		glBindVertexArray(VAOCuadrado);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_LINES  |  GL_FILL
		//glDisableVertexAttribArray(3);
		//glVertexAttrib4d(3, 0, c.color.y, c.color.z, c.alphaFactor);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0))));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(translateMat));
		glUniform1f(alphaLoc, pair.second);



		glBindTexture(GL_TEXTURE_2D, pair.first);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glEnableVertexAttribArray(3);
		glBindVertexArray(0);






	}
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}



void drawLocalPlayer(glm::mat4 view, glm::mat4 projection){

	glClear( GL_DEPTH_BUFFER_BIT);

	entity* e = entityList.at(entity_manager::LOCAL_PLAYER);
	//printf("%lf\n", e->eulerAngles.alpha);
	glm::mat4 transformEntity = glm::translate(glm::mat4(), e->position);

	
	transformEntity = glm::rotate(transformEntity, -e->eulerAngles.alpha, glm::vec3(0, 1, 0));
	//transformEntity = glm::rotate(transformEntity, -e->eulerAngles.beta, glm::vec3(0, 0, 1));


	e->transformToInherit = transformEntity;

	int componentsSize = e->components.size();
	for (int j = 0; j < componentsSize; ++j)
	{

		if (!drawComponentCondition(*e, j, 0)) continue;


		component c = e->components.at(j);

		GLuint transformLoc = glGetUniformLocation(c.associatedShader, "transform");
		GLuint projectionLoc = glGetUniformLocation(c.associatedShader, "projection");
		GLuint viewLoc = glGetUniformLocation(c.associatedShader, "view");
		GLuint lightColorLoc = glGetUniformLocation(c.associatedShader, "lightColor");
		GLuint lightPosLoc = glGetUniformLocation(c.associatedShader, "lightPos");

		GLuint widthLoc = glGetUniformLocation(c.associatedShader, "screen_width");
		GLuint heightLoc = glGetUniformLocation(c.associatedShader, "screen_height");


		glUseProgram(c.associatedShader);


		//glm::mat4 transformComponent = glm::translate(transformEntity, c.position);
		glm::mat4 transformComponent = glm::translate(glm::mat4(1.0), -halfCameraHeight);

		glBindVertexArray(c.VAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_LINES  |  GL_FILL
		glDisableVertexAttribArray(3);
		glVertexAttrib4d(3, c.color.x, c.color.y, c.color.z, c.alphaFactor);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::lookAt(glm::vec3(0, 0, 0) - recoilPushback, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0))));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformComponent));
		glUniform4fv(lightColorLoc, 1, lightColor);
		glm::vec3 newLightPos = { lightPos[0] - e->position.x,lightPos[1] - e->position.y,lightPos[2] - e->position.z };
		newLightPos = glm::rotate(glm::mat4(1.0), e->eulerAngles.alpha, glm::vec3(0, 1, 0)) * glm::vec4(newLightPos, 1);
		newLightPos = glm::rotate(glm::mat4(1.0), e->eulerAngles.beta, glm::vec3(0, 0, 1)) * glm::vec4(newLightPos, 1);
		float finalLightPos[3] = { newLightPos.x, newLightPos.y, newLightPos.z };
		glUniform3fv(lightPosLoc, 1, finalLightPos);
		
		glUniform1f(widthLoc, (float)SCR_WIDTH);
		glUniform1f(heightLoc, (float)SCR_HEIGHT);

		glBindTexture(GL_TEXTURE_2D, c.texture);
		glDrawArrays(GL_TRIANGLES, c.iniVert, c.numVert);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnableVertexAttribArray(3);
		glBindVertexArray(0);
	}
}

void drawForeignPlayers(glm::mat4 view, glm::mat4 projection) {

	int vecSize = entityList.size();
	for (int i = entity_manager::FOREIGN_PLAYER ; i < vecSize; ++i)
	{

		entity* e = entityList.at(i);
		glm::mat4 transformEntity = e->transformToInherit;


		int componentsSize = e->components.size();
		for (int j = 0; j < componentsSize; ++j)
		{
			
			if (!drawComponentCondition(*e, j, 1)) continue;


			component c = e->components.at(j);

			GLuint transformLoc = glGetUniformLocation(c.associatedShader, "transform");
			GLuint projectionLoc = glGetUniformLocation(c.associatedShader, "projection");
			GLuint viewLoc = glGetUniformLocation(c.associatedShader, "view");
			GLuint lightColorLoc = glGetUniformLocation(c.associatedShader, "lightColor");
			GLuint lightPosLoc = glGetUniformLocation(c.associatedShader, "lightPos");

			GLuint widthLoc = glGetUniformLocation(c.associatedShader, "screen_width");
			GLuint heightLoc = glGetUniformLocation(c.associatedShader, "screen_height");


			glUseProgram(c.associatedShader);


			glm::mat4 transformComponent = glm::translate(transformEntity, c.position);


			if (j == entity_manager::HEAD)
			{
				transformComponent = glm::translate(transformComponent, cameraHeight);
				transformComponent = glm::rotate(transformComponent, -e->eulerAngles.beta, glm::vec3(0, 0, 1));
				transformComponent = glm::translate(transformComponent, -cameraHeight);
			}

				


			glBindVertexArray(c.VAO);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_LINES  |  GL_FILL
			glDisableVertexAttribArray(3);
			glVertexAttrib4d(3, c.color.x, c.color.y, c.color.z, c.alphaFactor);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformComponent));
			glUniform4fv(lightColorLoc, 1, lightColor);
			glUniform3fv(lightPosLoc, 1, lightPos);

			glUniform1f(widthLoc, (float)SCR_WIDTH);
			glUniform1f(heightLoc, (float)SCR_HEIGHT);

			glBindTexture(GL_TEXTURE_2D, c.texture);
			glDrawArrays(GL_TRIANGLES, c.iniVert, c.numVert);
			glBindTexture(GL_TEXTURE_2D, 0);
			glEnableVertexAttribArray(3);
			glBindVertexArray(0);
		}
	}
}

void drawMap(glm::mat4 view, glm::mat4 projection){

	entity* e = entityList.at(entity_manager::MAP);
	glm::mat4 transformEntity = e->transformToInherit;


	int componentsSize = e->components.size();
	for (int j = 0; j < componentsSize; ++j)
	{

		component c = e->components.at(j);

		GLuint transformLoc = glGetUniformLocation(c.associatedShader, "transform");
		GLuint projectionLoc = glGetUniformLocation(c.associatedShader, "projection");
		GLuint viewLoc = glGetUniformLocation(c.associatedShader, "view");
		GLuint lightColorLoc = glGetUniformLocation(c.associatedShader, "lightColor");
		GLuint lightPosLoc = glGetUniformLocation(c.associatedShader, "lightPos");

		GLuint widthLoc = glGetUniformLocation(c.associatedShader, "screen_width");
		GLuint heightLoc = glGetUniformLocation(c.associatedShader, "screen_height");


		glUseProgram(c.associatedShader);


		glm::mat4 transformComponent = transformEntity;
		//glm::mat4 transformComponent = glm::translate(transformEntity, c.position);


		glBindVertexArray(c.VAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_LINES  |  GL_FILL
		glDisableVertexAttribArray(3);
		glVertexAttrib4d(3, c.color.x, c.color.y, c.color.z, c.alphaFactor);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformComponent));
		glUniform4fv(lightColorLoc, 1, lightColor);
		glUniform3fv(lightPosLoc, 1, lightPos);

		glUniform1f(widthLoc, (float)SCR_WIDTH);
		glUniform1f(heightLoc, (float)SCR_HEIGHT);

		glBindTexture(GL_TEXTURE_2D, c.texture);
		glDrawArrays(GL_TRIANGLES, c.iniVert, c.numVert);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnableVertexAttribArray(3);
		glBindVertexArray(0);
	}
}


void mainLoop(GLFWwindow* window, double& prevTime, std::vector<entity*> entityList)
{
	while (!glfwWindowShouldClose(window))
	{
		if (endGame != entity_manager::NOT_FINISHED){
			shotReceived(endGame);
			glfwPollEvents();
			continue;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = glm::mat4();
		glm::mat4 projection = glm::mat4();
		myCamara(view, projection);
		

		drawMap(view, projection);

		drawForeignPlayers(view, projection);
		drawShootDecals(projection);

		drawLocalPlayer(view, projection);
		drawDamageDecals(projection);


		updateLocalPlayerPhysics(prevTime, *entityList.at(entity_manager::LOCAL_PLAYER));
		updateDecals(prevTimeDecals);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}


void initializeDecals()
{
	//fireDecals
	VAOCuadrado = vaoCuadrado();
	for (int i = 0; i <= SHOOT_DECAL_TEXTURE_NUM; ++i) 
	{
			char var[MAX_PATH];
			sprintf(var,"./textures/fireDecals/%03d.png", i);
			fireDecals[i] = externObject::loadRawTexture(var);
	}
	hurtDecal = externObject::loadRawTexture("./textures/hurt.png");
}



std::vector<entity*> initializeEntitiesAndShaders()
{
	//Carga de los shaders
	shaderProgram = setShaders("Solarshader.vert", "Solarshader.frag");
	//anilloShaderProgram = setShaders("Ringshader.vert", "Ringshader.frag");
	noLightShaderProgram = setShaders("Solarshader.vert", "NoLightShader.frag");
	//noLightRingShaderProgram = setShaders("Ringshader.vert", "RingNoLightShader.frag");

	if (!shaderProgram || !noLightShaderProgram){//|| !anilloShaderProgram) {
		std::cout << "Failed to link the shaders: 'shaderProgram' || 'noLightShaderProgram'" << std::endl; //|| 'anilloShaderProgram'
		exit(EXIT_FAILURE);
	}

	unsigned int auxEndgameT1, auxEndgameT2;

	auxEndgameT1 = externObject::loadRawTexture("./textures/winner.png");
	auxEndgameT2 = externObject::loadRawTexture("./textures/loser.jpg");

	endgameTextures = std::make_pair(auxEndgameT1, auxEndgameT2);
	
	initializeDecals();



	/*
	component defaultSphere1 = { {0,0,0},{0,0},glm::mat4(1.0f),{1,1,1},{1,0,0},1,VAOEsfera, 0, SPHERE_VERTEXES, orbitalTexture, shaderProgram, std::make_pair(glm::vec3(0,0,0),glm::vec3(0,0,0)),NULL };
	component defaultSphere2 = { {0,0,0},{0,0},glm::mat4(1.0f),{1,1,1},{0,1,0},1,VAOEsfera, 0, SPHERE_VERTEXES, orbitalTexture, shaderProgram, std::make_pair(glm::vec3(0,0,0),glm::vec3(0,0,0)),NULL };
	component defaultSphere3 = { {0,0,0},{0,0},glm::mat4(1.0f),{1,1,1},{0,1,1},1,VAOEsfera, 0, SPHERE_VERTEXES, orbitalTexture, shaderProgram, std::make_pair(glm::vec3(0,0,0),glm::vec3(0,0,0)),NULL };
	component defaultSphere4 = { {0,0,0},{0,0},glm::mat4(1.0f),{1,1,1},{1,1,0},1,VAOEsfera, 0, SPHERE_VERTEXES, orbitalTexture, shaderProgram, std::make_pair(glm::vec3(0,0,0),glm::vec3(0,0,0)),NULL };


	std::vector<component> localSphere1 = { defaultSphere1 };
	std::vector<component> localSphere2 = { defaultSphere2 };
	std::vector<component> localSphere3 = { defaultSphere3 };
	std::vector<component> localSphere4 = { defaultSphere4 };
	*/

	//TODO: Checkear error al cargar objetos
	entity* JugadorExterno = externObject::loadEntity("player.obj", shaderProgram);

	JugadorExterno->transformToInherit = glm::translate(glm::mat4(), glm::vec3(3, 2, 0));
	JugadorExterno->position = {3,2,0};
	
	entity* map = externObject::loadEntity("mapa.obj", shaderProgram);
	//TODO BORRAR
	
	//map->components.push_back(square);
	//activeFireDecals.push_back(fireDecals[11]);
	entity* JugadorLocal = externObject::loadEntity("player_firstPerson.obj", shaderProgram);
	//TODO BORRAR
	
	JugadorLocal->position += glm::vec3(8, 0, 0);
	//JugadorExterno->transformToInherit = glm::translate(glm::mat4(), glm::vec3(0, OFFSET_PLAYER_HEIGHT_STANDING, 0));
	JugadorLocal->position.y = OFFSET_PLAYER_HEIGHT_STANDING;

	std::pair<glm::vec3, glm::vec3> headBounds = JugadorExterno->components.at(entity_manager::HEAD).bounds;
	cameraHeight = { 0, (headBounds.first.y + headBounds.second.y)/2, 0};
	halfCameraHeight = (float)(1.0 / 2.0) * cameraHeight;
	map->components.at(entity_manager::DISPLAY).associatedShader = noLightShaderProgram;




	//NOTA: Se tiene que seguir el orden de entidades dictado por el enum del namespace entity_manager
	std::vector<entity*> entList = { map, JugadorLocal, JugadorExterno};

	return entList;
}





int main(int argc, char** argv) {


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RSGO Alpha", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	openGlInit();



	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);
	printf("GLFW version: %d.%d.%d\n", major, minor, rev);
	glfwSetFramebufferSizeCallback(window, windowResize);       //Indica la función a llamar al cambiar de tamaño la pantalla
	glfwSetKeyCallback(window, keyEvents);                      //Indica la función a llamar al presionar una tecla, mantenerla o soltarla
	glfwSetMouseButtonCallback(window, mouseClick);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, mouseEvents);
	glfwSetCursorPos(window, 0, 0);


	windowResize(window, SCR_WIDTH, SCR_HEIGHT); //Inicializa la ventana con los tamaños y viewport que queremos

	std::pair<std::thread*, std::thread*> threads;

	entityList = initializeEntitiesAndShaders();
	// Lazo principal
	char ip[IP4_CHAR_SZ];

	printf("Introduce 0 para ser cliente, 1 para ser servidor, 2 para LOCAL:\n");
	int State;
	scanf("%d", &State);



	if (State != 2)
	{
		uint16_t port;
		if (State == 1)
			printf("Introduce el puerto para el servidor:\n");
		else
		{
			printf("Introduce la IP del servidor:\n");
			scanf("%s", ip);
			printf("Introduce el puerto remoto:\n");
		}

		scanf("%hu", &port);
		network::setupMultiplayerConnection(ip, port, entityList, (State == 1));
	}

	mainLoop(window, prevTime, entityList);

	//Liberación de recursos
	std::set<unsigned int> freedTextures = {};  //NOTA: usamos "set" en vez de "vector" porque para buscar un elemento en él es más rápido de esta forma

	if (State != 2) {
		(*(threads.first)).join();
		(*(threads.second)).join();
	}

	if (freedTextures.find(endgameTextures.first) == freedTextures.end()) {
		freedTextures.insert(endgameTextures.first);
		glDeleteTextures(1, &(endgameTextures.first));
	}
	if (freedTextures.find(endgameTextures.second) == freedTextures.end()) {
		freedTextures.insert(endgameTextures.second);
		glDeleteTextures(1, &(endgameTextures.second));
	}

	if (freedTextures.find(hurtDecal) == freedTextures.end()) {
		freedTextures.insert(hurtDecal);
		glDeleteTextures(1, &hurtDecal);
	}
	
	glDeleteVertexArrays(1, &VAOCuadrado);

	for (int i = 0; i <= SHOOT_DECAL_TEXTURE_NUM; ++i)
	{
		if (freedTextures.find(fireDecals[i]) == freedTextures.end()) {
			freedTextures.insert(fireDecals[i]);
			glDeleteTextures(1, &(fireDecals[i]));
		}
	}

	for (entity* e : entityList) {
		std::vector<component> entityComponents = e->components;
		for (component c : entityComponents) {
			if (freedTextures.find(c.texture) == freedTextures.end()) { //No encontró el objeto en el set
				freedTextures.insert(c.texture);
				glDeleteTextures(1, &(c.texture));
			}
			glDeleteVertexArrays(1, &(c.VAO));
		}

		delete e;
	}

	// Termino todo.
	glfwTerminate();
	return 0;
}

unsigned int vaoCuadrado() {

	GLfloat vertices[] = {
	1.f,  1.f, -1.f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f, // Top-left
	1.f,  1.f,  1.f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f, // Top-right
	1.f, -1.f,  1.f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
	1.f, -1.f, -1.f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f  // Bottom-left
	};

	// The indices for the square
	GLuint indices[] = {
		0, 1, 2, // First triangle
		2, 3, 0  // Second triangle
	};

	GLuint VAO,VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Color attribute
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VAO
	return VAO;
}

void mouseClick(GLFWwindow* window, int button, int action, int mods){

	if (endGame != entity_manager::NOT_FINISHED) return;


	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){

		entity * e = entityList.at(entity_manager::LOCAL_PLAYER);

		if (e->weapon == entity_manager::WEAPON_KNIFE) return; //Evita "dispara" con el cuchillo //TODO: permitir ataque diferente con cuchillo

		DWORD status = network::messageTypes::PLAYER_SHOT;
		glm::vec3 localPlayerPosition = e->position;
		if(checkPlayerHit(lookingAtVec, localPlayerPosition + cameraHeight))
		{
			status |= network::messageTypes::ENEMY_HIT;
			shotReceived(entity_manager::WIN);
		}
		
		activeFireDecals.push_back(fireDecals[0]);
		recoilPushback = RECOIL_PUSHBACK;
		network::sendGameEvent(status, entityList);
	}
		
} 

void mouseEvents(GLFWwindow* window, double xpos, double ypos)
{
	if (endGame != entity_manager::NOT_FINISHED) return;


	if (xpos > EYE_MODULUS)
		glfwSetCursorPos(window, xpos - EYE_MODULUS, ypos);
	else if (xpos < -EYE_MODULUS)
		glfwSetCursorPos(window, xpos + EYE_MODULUS, ypos);
	if (ypos > EYE_MODULUS)
		glfwSetCursorPos(window, xpos, ypos - EYE_MODULUS);
	else if (ypos < -EYE_MODULUS)
		glfwSetCursorPos(window, xpos, ypos + EYE_MODULUS);

	float& mappedXpos = entityList.at(entity_manager::LOCAL_PLAYER)->eulerAngles.alpha;
	float& mappedYpos = entityList.at(entity_manager::LOCAL_PLAYER)->eulerAngles.beta;

	if (ypos > EYE_MODULUS/4 - 10)
		glfwSetCursorPos(window, xpos, ypos = EYE_MODULUS/4 - 10);
	else if (ypos < -EYE_MODULUS/4 + 10)
		glfwSetCursorPos(window, xpos, ypos = -EYE_MODULUS/4 + 10);

	mappedXpos = xpos * MAPPER_CONSTANT;
	mappedYpos = ypos * MAPPER_CONSTANT;
	//printf("angulo: %f EYEPOS %lf\n", 90 * mappedYpos / (3.1415 / 2),ypos);

	lookingAtVec = { 1,0,0 };
	//lookingAtVec = * lookingAtVec;

	glm::mat4 rotation = glm::rotate(glm::mat4(), (float)-mappedYpos, glm::vec3(0, 0, 1));
	lookingAtVec = rotation * glm::vec4(lookingAtVec, 1);
	rotation = glm::rotate(glm::mat4(), (float)mappedXpos, glm::vec3(0, -1, 0));
	lookingAtVec = rotation * glm::vec4(lookingAtVec, 1);

}

void keyEvents(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if(endGame != entity_manager::NOT_FINISHED){
		//Salir:
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		return;
	}


	//CAMBIOS de CÁMARA:
	if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		if (!fullScreen){
			fullScreen = true;
			glfwMaximizeWindow(window);
		}else{
			fullScreen = false;
			glfwRestoreWindow(window);
		}
	}
	
	//CAMBIOS DE ARMA:
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {

		int *weapon = &(entityList.at(entity_manager::LOCAL_PLAYER)->weapon);

		if (*weapon != entity_manager::WEAPON_AK) {
			network::sendGameEvent(network::messageTypes::PLAYER_SELECTED_WEAPON_AK, entityList);
			MAX_SPEED = 15;
			*weapon = entity_manager::WEAPON_AK;
		}
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {

		int* weapon = &(entityList.at(entity_manager::LOCAL_PLAYER)->weapon);

		if (*weapon != entity_manager::WEAPON_KNIFE) {
			network::sendGameEvent(network::messageTypes::PLAYER_SELECTED_WEAPON_KNIFE, entityList);
			MAX_SPEED = 18;
			*weapon = entity_manager::WEAPON_KNIFE;
		}
	}

	//CAMBIOS DE POSTURA:
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		network::sendGameEvent(network::messageTypes::PLAYER_CROUCHED_SWITCHED, entityList);
		entity* e = entityList.at(entity_manager::LOCAL_PLAYER);
		e->crouching = true;
		e->position.y -= OFFSET_PLAYER_HEIGHT_DIFF_WHEN_CROUCHING;
	}
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
		network::sendGameEvent(network::messageTypes::PLAYER_CROUCHED_SWITCHED, entityList);
		entity* e = entityList.at(entity_manager::LOCAL_PLAYER);
		e->crouching = false;
		e->position.y += OFFSET_PLAYER_HEIGHT_DIFF_WHEN_CROUCHING;
	}



	
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		WBeingHeld = true;
		
	}
	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		WBeingHeld = false;
		
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		SBeingHeld = true;

	}
	if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		SBeingHeld = false;

	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		ABeingHeld = true;

	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		DBeingHeld = true;
	}

	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		ABeingHeld = false;

	}

	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		DBeingHeld = false;
	
	}


	if (WBeingHeld && !SBeingHeld) fmove = 1;
	else if (WBeingHeld && SBeingHeld) fmove = 0;
	else if (!WBeingHeld && SBeingHeld) fmove = -1;
	else if (!WBeingHeld && !SBeingHeld) fmove = 0;

	if (ABeingHeld && !DBeingHeld) smove = -1;
	else if (ABeingHeld && DBeingHeld) smove = 0;
	else if (!ABeingHeld && DBeingHeld) smove = 1;
	else if (!ABeingHeld && !DBeingHeld) smove = 0;


	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		topview = !topview;
	}

	//Salir:
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	return;
}

void shotReceived(enum entity_manager::ENDGAME_STATES state) {

	if (state == entity_manager::NOT_FINISHED) return;

	if(localHealth > 1 && state == entity_manager::LOST){
		activeDamageDecals.push_back(std::make_pair(hurtDecal, 1.f));
		--localHealth;
		return;
	}
	if(foreignHealth > 1 && state == entity_manager::WIN){
		--foreignHealth;
		return;
	}


	endGame = state;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	component display = entityList.at(entity_manager::MAP)->components.at(entity_manager::DISPLAY);

	glm::mat4 view = glm::lookAt(glm::vec3(0, 110, 0), glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0));
	glm::mat4 projection = glm::ortho(-30.f, 30.f, -30.f, 30.f, .1f, 100.f);


	switch (state) {
	case (entity_manager::WIN):
		display.texture = endgameTextures.first;
		break;
	case (entity_manager::LOST):
		display.texture = endgameTextures.second;
		break;
	}

	GLuint transformLoc = glGetUniformLocation(display.associatedShader, "transform");
	GLuint projectionLoc = glGetUniformLocation(display.associatedShader, "projection");
	GLuint viewLoc = glGetUniformLocation(display.associatedShader, "view");
	GLuint alphaLoc = glGetUniformLocation(display.associatedShader, "alphaChannelFactor");

	glUseProgram(display.associatedShader);

	glBindVertexArray(display.VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_LINES  |  GL_FILL
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
	glUniform1f(alphaLoc, display.alphaFactor);
	glBindTexture(GL_TEXTURE_2D, display.texture);
	glDrawArrays(GL_TRIANGLES, display.iniVert, display.numVert);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	glfwSwapBuffers(window);
}

void windowResize(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;

	int size, pW, pH;
	if (SCR_WIDTH < SCR_HEIGHT * 2) {             //Adapta el viewport a las órbitas elípticas dejando ver una relación 2 a 1 de ancho respecto a alto
		size = SCR_WIDTH;
		pW = 0;
		pH = SCR_HEIGHT / 2 - SCR_WIDTH / 2;
	}
	else {
		size = SCR_HEIGHT * 2;
		pW = SCR_WIDTH / 2 - SCR_HEIGHT;
		pH = -SCR_HEIGHT / 2;
	}
	glViewport(pW, pH, size, size);
}

void openGlInit() {
	//Habilito aqui el depth en vez de arriba para los usuarios de linux y mac mejor arriba
	//Incializaciones varias
	glClearDepth(1.0f); //Valor z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // valor limpieza buffer color
	glEnable(GL_DEPTH_TEST); // z-buffer
	glEnable(GL_CULL_FACE); //ocultacion caras back
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);                                 //Para usar los alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //Para usar los alpha
	glEnable(GL_TEXTURE_2D);
}
