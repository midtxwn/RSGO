#include "externObject.h"
#include <iostream>
#include <fstream>
#include <string>
#include <glad.h>

#include <stb_image.h>

unsigned int externObject::loadSingleTexture(const char* fileName) {

	GLuint textura;

	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	
	stbi_set_flip_vertically_on_load(true);
	
	unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 0);
	if (data)
	{
		switch (nrChannels) {
		case 3:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case 4:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	else
	{
		printf("Failed to load texture: %s\n", fileName);
	}
	stbi_image_free(data);

	return textura;
}

std::map<std::string, unsigned int> externObject::parseMtl(std::string fileName) {

	std::map<std::string, unsigned int> mtlTextures;
	std::string s;

	//Opening the file
	std::ifstream file(fileName);
	if (!file) {
		std::cout << "Error: Failed to load the .OBJ file" << std::endl;
		return mtlTextures;
	}

	//Parsing the data out
	while (file >> s) {
		if (!s.compare("newmtl")) {
			std::string materialName;

			file >> materialName;

			while (file >> s) {
				if (!s.compare("map_Kd")) {
					std::string textureFileName;

					file >> textureFileName;

					mtlTextures.insert(std::pair<std::string, unsigned int>(
						materialName, loadSingleTexture(textureFileName.c_str())
					));
					break;
				}
			}
			continue;
		}
	}

	return mtlTextures;
}

externObject::objFileFullData externObject::parseObj(const char* fileName) {

	std::string s;
	std::string mtlFileName;
	objFileFullData fileData;
	int offsetPreviousVertices = 0;
	int offsetPreviousNormals = 0;
	int offsetPreviousTexturePoints = 0;
	bool firstFound = false;

	//Opening the file
	std::ifstream file(fileName);
	if (!file) {
		std::cout << "Error: Failed to load the .OBJ file" << std::endl;
		return fileData;
	}

	//Parsing the data out
	while (file >> s) {
		if (!s.compare("mtllib")) {
			file >> mtlFileName;
			continue;
		}
		if (!s.compare("usemtl")) {
			file >> fileData.componentsData.back().associatedMaterialName;
			continue;
		}
		if (!s.compare("o")) 
		{

			objectData partData = {};
			file >> s;
			partData.objName = s;
			fileData.componentsData.push_back(partData);
			if (firstFound) {
				offsetPreviousVertices += fileData.componentsData.rbegin()[1].vertices.size();
				offsetPreviousNormals += fileData.componentsData.rbegin()[1].normals.size();
				offsetPreviousTexturePoints += fileData.componentsData.rbegin()[1].texturePoints.size();
			}
			firstFound = true;
			continue;
		}
		if (!s.compare("v")) {
			vertex v;
			file >> v.x;
			file >> v.y;
			file >> v.z;
			fileData.componentsData.back().vertices.push_back(v);
			continue;
		}
		if (!s.compare("vn")) {
			normal n;
			file >> n.x;
			file >> n.y;
			file >> n.z;
			fileData.componentsData.back().normals.push_back(n);
			continue;
		}
		if (!s.compare("vt")) {
			texture t;
			file >> t.u;
			file >> t.v;
			fileData.componentsData.back().texturePoints.push_back(t);
			continue;
		}
		if (!s.compare("f")) {
			face f;
			char a;        //Logra evitar la barra '/' entre números
			file >> f.v1; file >> a;
			file >> f.t1; file >> a;
			file >> f.n1;

			file >> f.v2; file >> a;
			file >> f.t2; file >> a;
			file >> f.n2;

			file >> f.v3; file >> a;
			file >> f.t3; file >> a;
			file >> f.n3;

			f.v1 -= offsetPreviousVertices;
			f.v2 -= offsetPreviousVertices;
			f.v3 -= offsetPreviousVertices;

			f.n1 -= offsetPreviousNormals;
			f.n2 -= offsetPreviousNormals;
			f.n3 -= offsetPreviousNormals;

			f.t1 -= offsetPreviousTexturePoints;
			f.t2 -= offsetPreviousTexturePoints;
			f.t3 -= offsetPreviousTexturePoints;
			fileData.componentsData.back().faces.push_back(f);
			continue;
		}
	}
	file.close();

	fileData.mtlTextures = parseMtl(mtlFileName);

	return fileData;
}


float externObject::sumReduceVertex(glm::vec3 v)
{
	return v.x + v.y + v.z;
}

entity* externObject::loadEntity(const char* fileName, unsigned int shader) {

	entity* retEnt = new entity{};
	objFileFullData fileData = parseObj(fileName);
	int numComponents = fileData.componentsData.size();

	if (!numComponents) {
		delete retEnt;
		return NULL;
	}

	for (int i = 0; i < numComponents; ++i) {
		

		
		objectData auxCompData = fileData.componentsData.at(i);
		bool mapObject = (auxCompData.objName.find("MAP") != std::string::npos);
		
		if (auxCompData.objName.find("Bound") != std::string::npos)// si es bounding, nos saltamos el objeto
			continue;
		
		std::pair<glm::vec3,glm::vec3> boundingBox;

		if(!mapObject)
			for (int j = 0; j < numComponents; ++j)
			{
			
				if (fileData.componentsData.at(j).objName != (std::string("Bound") + auxCompData.objName))
					continue;

				glm::vec3 minVertex = 
				{ 
					fileData.componentsData.at(j).vertices.at(0).x,
					fileData.componentsData.at(j).vertices.at(0).y,
					fileData.componentsData.at(j).vertices.at(0).z
				},
					maxVertex = minVertex;
				for (vertex v : fileData.componentsData.at(j).vertices)
				{
					glm::vec3 auxV = glm::vec3(v.x, v.y, v.z);
					if (sumReduceVertex(auxV) < sumReduceVertex(minVertex)){
						minVertex = auxV;
						continue;
					}
						
					if (sumReduceVertex(auxV) > sumReduceVertex(maxVertex))
						maxVertex = auxV;
				}
				boundingBox = std::make_pair( minVertex, maxVertex );
				break;
			}

		else
		{
			glm::vec3 minVertex =
			{
				fileData.componentsData.at(i).vertices.at(0).x,
				fileData.componentsData.at(i).vertices.at(0).y,
				fileData.componentsData.at(i).vertices.at(0).z
			},
				maxVertex = minVertex;
			for (vertex v : fileData.componentsData.at(i).vertices)
			{
				glm::vec3 auxV = glm::vec3(v.x, v.y, v.z);
				if (sumReduceVertex(auxV) < sumReduceVertex(minVertex)) {
					minVertex = auxV;
					continue;
				}

				if (sumReduceVertex(auxV) > sumReduceVertex(maxVertex))
					maxVertex = auxV;
			}
			boundingBox = std::make_pair(minVertex, maxVertex);

		}
		component c = 
		{
			{},
			{},
			glm::mat4(),
			{1.f, 1.f, 1.f},
			{1.f, 1.f, 1.f},
			1.f,
			loadVao(
				auxCompData.vertices,
				auxCompData.texturePoints,
				auxCompData.normals,
				auxCompData.faces
			),
			0,
			auxCompData.faces.size() * 3, // *3 porque solo se admiten caras triangulares
			fileData.mtlTextures[auxCompData.associatedMaterialName],
			shader,
			boundingBox,
			boundingBox
		};
		retEnt->components.push_back(c);
	}



	retEnt->escala = glm::vec3(1.f, 1.f, 1.f);
	retEnt->eulerAngles = { 0.f, 0.f };
	retEnt->position = { 0.f, 0.f, 0.f };
	retEnt->transformToInherit = glm::mat4();
	retEnt->weapon = entity_manager::WEAPON_KNIFE;
	retEnt->crouching = false;

	return retEnt;
}

unsigned int* externObject::loadObject(const char* fileName) {

	std::vector<vertex> vertices;
	std::vector<texture> texturas;
	std::vector<normal> normales;
	std::vector<face> faces;
	unsigned int texturaAsociada = 0;

	std::string s;
	std::ifstream file(fileName);
	if (!file) {
		std::cout << "Error: Failed to load the .OBJ file" << std::endl;
		return NULL;
	}

	while (file >> s) {
		switch (*(s.c_str())) {
		case 'v':
			switch (s.size()) {
			case 1: {
				vertex v;
				file >> v.x;
				file >> v.y;
				file >> v.z;
				vertices.push_back(v);
				break;
			}
			case 2:
				switch (s[1]) {
				case 't': {
					texture t;
					file >> t.u;
					file >> t.v;
					texturas.push_back(t);
					break;
				}
				case 'n': {
					normal n;
					file >> n.x;
					file >> n.y;
					file >> n.z;
					normales.push_back(n);
					break;
				}
				}
			}
			break;
		case 'f': {
			face f;
			char a;        //Logra evitar la barra '/' entre números
			file >> f.v1;
			file >> a;
			file >> f.t1;
			file >> a;
			file >> f.n1;

			file >> f.v2;
			file >> a;
			file >> f.t2;
			file >> a;
			file >> f.n2;

			file >> f.v3;
			file >> a;
			file >> f.t3;
			file >> a;
			file >> f.n3;
			faces.push_back(f);
			break;
		}
		case 'm': {
			if (!s.compare("mtllib")) {
				std::string textureFile;
				file >> textureFile;
				texturaAsociada = loadTexture(textureFile);
			}
		}
		}
	}
	file.close();

	unsigned int ret[2] = { loadVao(vertices, texturas, normales, faces), texturaAsociada };
	return ret;
}

unsigned int externObject::loadVao(std::vector<vertex> vertices, std::vector<texture> texturas, std::vector<normal> normales, std::vector<face> faces) {

	std::vector<float> data;
	unsigned int VAO, VBO;
	int vInd, tInd, nInd, numCaras = faces.size();

	for (int i = 0; i < numCaras; ++i) {

		vInd = faces[i].v1 - 1;
		tInd = faces[i].t1 - 1;
		nInd = faces[i].n1 - 1;

		data.insert(data.end(), {
			vertices[vInd].x, vertices[vInd].y, vertices[vInd].z,
			texturas[tInd].u, texturas[tInd].v,
			normales[nInd].x, normales[nInd].y, normales[nInd].z
			});


		vInd = faces[i].v2 - 1;
		tInd = faces[i].t2 - 1;
		nInd = faces[i].n2 - 1;

		data.insert(data.end(), {
			vertices[vInd].x, vertices[vInd].y, vertices[vInd].z,
			texturas[tInd].u, texturas[tInd].v,
			normales[nInd].x, normales[nInd].y, normales[nInd].z
			});


		vInd = faces[i].v3 - 1;
		tInd = faces[i].t3 - 1;
		nInd = faces[i].n3 - 1;

		data.insert(data.end(), {
			vertices[vInd].x, vertices[vInd].y, vertices[vInd].z,
			texturas[tInd].u, texturas[tInd].v,
			normales[nInd].x, normales[nInd].y, normales[nInd].z
			});
	}


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &(data[0]), GL_STATIC_DRAW);

	//Vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Texturas
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	//glDeleteBuffers(1, &VBO);

	return VAO;
}


unsigned int externObject::loadRawTexture(std::string fileName) 
{
	GLuint textura;

	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(true);

	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		switch (nrChannels) {
		case 3:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case 4:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	else
	{
		printf("Failed to load texture: %s\n", fileName.c_str());
	}
	stbi_image_free(data);

	return textura;

}


unsigned int externObject::loadTexture(std::string fileName) {

	//////////////////Obtener el nombre del archivo de textura
	std::string s;
	std::ifstream file(fileName);
	if (!file) {
		std::cout << "Error: Failed to load the .MTL file" << std::endl;
		return 0;
	}

	const char* imagen = 0;
	std::string auxImage;

	while (file >> s) {
		if (!s.compare("map_Kd")) {
			file >> auxImage;
			imagen = auxImage.c_str();
			std::cout << "Archivo de textura externa a abrir: " << imagen << std::endl;
		}
	}
	file.close();


	///////////////////Cargar la textura del archivo
	GLuint textura;

	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load(imagen, &width, &height, &nrChannels, 0);
	if (data)
	{
		switch (nrChannels) {
		case 3:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case 4:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	else
	{
		printf("Failed to load texture: %s\n", imagen);
	}
	stbi_image_free(data);

	return textura;
}