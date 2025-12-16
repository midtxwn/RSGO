#pragma once

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <glad.h>
#include <glfw3.h>
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//RENDERING:
constexpr auto SLOWDOWN_TIME = 1;
constexpr auto INACCURATE_TIME_THRESHOLD = 10000;
constexpr auto AVG_FRAMERATE = 300;
constexpr auto UPDATE_PHYSICS_INTERVAL = (1.0 / AVG_FRAMERATE) / SLOWDOWN_TIME;
constexpr double EXTRA_TIME_FOR_FRICTION_CALC = (240 * 0.3) / AVG_FRAMERATE;


//MATH:
constexpr auto TOLERANCE = 1e-6;
constexpr auto PI = 3.14159265358979323846;
constexpr auto TWO_TIMES_PI = 2 * PI;
constexpr auto PI_HALVES = PI / 2;

//PHYSICS:
constexpr auto PLAYER_ACCEL = 500;
constexpr auto FRICTION = 0.1;
constexpr auto ONE_MINUS_FRICTION = 1 - FRICTION;
constexpr auto RECOIL_PUSHBACK = glm::vec3(-0.7, 0, 0);

//DECALS
constexpr auto SHOOT_DECAL_TIME = 0.01;
constexpr auto SHOOT_DECAL_TEXTURE_NUM = 44;
constexpr auto HURT_ALPHA_REDUCTION = 0.05;
constexpr auto HURT_DECAL_SCALE = glm::vec3(2, 2, 6);

//NETWORK:
constexpr auto IP4_CHAR_SZ = 16 + 1;

//MOUSEMAPPING:
constexpr auto EYE_MODULUS = 4000;
constexpr auto MAPPER_CONSTANT = (2.0 * PI) / (EYE_MODULUS);

//MODELING:
constexpr auto OFFSET_PLAYER_HEIGHT_STANDING = 3.25f;
constexpr auto OFFSET_PLAYER_HEIGHT_DIFF_WHEN_CROUCHING = 1.56f;
constexpr auto MAX_VIEW_HEIGHT_ANGLE_SEMIRANGE = 88 * (PI / 180);  //TODO: Usar esta constante para limitar la visión hacia arriba y abajo con la cámara

//RAYCASTING
constexpr auto NUM_PLANES_FIRSTBOUND_COLLISION_CHECK = 2;
constexpr auto NUM_PLANES_SECONDBOUND_COLLISION_CHECK = 3;
constexpr auto NUM_AXIS_COLLISION_CHECK = 3;

//GAME
constexpr auto PLAYER_HEALTH = 10;




enum POSICION_ASTROS  //OJO: El código es dependiente del orden de los valores en el enum por el orden de introducción de los objetos en el vector de astros y por las cámaras
{
	SOL = 0, MERCURIO, VENUS, TIERRA, MARTE, JUPITER, SATURNO, URANO, NEPTUNO, LUNA, ISS, SATURNORING, SUNCOVER
};

//NOTA: El orden de los enums es crucial para el funcionamiento del código
namespace entity_manager
{

	const DWORD FOREIGN_PLAYER_TYPE = 0x32;

	enum ENTITY_INDEX
	{
		MAP = 0, LOCAL_PLAYER, FOREIGN_PLAYER
	};

	enum LOCAL_PLAYER_COMPONENT_INDEX
	{
		 AK_LOCAL = 0, AK_ARM_R, AK_ARM_L, KNIFE_ARM_R, KNIFE_LOCAL, KNIFE_ARM_L
	};

	enum FOREIGN_PLAYER_COMPONENT_INDEX
	{
		BODY = 0, EXTENDED_LEG_R, HEAD, BENT_LEG_R, EXTENDED_LEG_L, BENT_LEG_L, AK, KNIFE
	};

	enum MAP_COMPONENT_INDEX
	{
		FLOOR = 0, COLUMN1, WALL1, WALL2, COLUMN2, WALL3, WALL4, WALL5, COLUMN3, SBOX1, LBOX1, LBOX2, LBOX3, LBOX4, COLUMN4, HBOX1, SKYBOX, DISPLAY
	};

	enum PLAYER_WEAPONS
	{
		WEAPON_AK = 1, WEAPON_KNIFE = 2
	};

	enum ENDGAME_STATES
	{
		NOT_FINISHED = 0, WIN, LOST
	};
};


typedef struct {
	float alpha;
	float beta;
}eulerAngles;

struct entity;

struct component
{
	glm::vec3 position;
	eulerAngles eulerAngles;
	glm::mat4 transformToInherit;       //TODO: Lo usaríamos para mandar el resultado final de movimientos exclusivos de los componentes (como doblar la cabeza hacia abajo)
	glm::vec3 escala;
	glm::vec3 color;
	float alphaFactor;
	unsigned int VAO, iniVert, numVert;
	unsigned int texture;
	unsigned int associatedShader;
	std::pair<glm::vec3,glm::vec3> bounds;
	std::pair<glm::vec3, glm::vec3> traslatedBounds;
};

struct entity
{
	glm::vec3 position;
	eulerAngles eulerAngles;
	glm::mat4 transformToInherit;
	glm::vec3 escala;
	std::vector<component> components;
	int weapon;
	bool crouching;
};

#endif // DEFINITIONS_H//