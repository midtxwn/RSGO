#pragma once

#ifndef EXTERNOBJECT_H
#define EXTERNOBJECT_H

#include <vector>
//#include <glad.h>
#include <iostream>
#include <map>
#include "definitions.h"

class externObject
{
	struct vertex {
		float x, y, z;
	};

	struct texture {
		float u, v;
	};

	struct normal {
		float x, y, z;
	};

	struct face {
		int v1, v2, v3;
		int t1, t2, t3;
		int n1, n2, n3;
	};

	struct objectData {
		std::string objName;
		std::vector<vertex> vertices;
		std::vector<texture> texturePoints;
		std::vector<normal> normals;
		std::vector<face> faces;
		std::string associatedMaterialName;
	};

	struct objFileFullData {
		std::vector<objectData> componentsData;
		std::map<std::string, unsigned int> mtlTextures;
	};
	

private:
	//NUEVO:
	//En caso de error devuelve un vector vacío (size = 0  |  empty)
	static objFileFullData parseObj(const char* fileName);
	//En caso de error devuelve un mapa vacío
	static std::map<std::string, unsigned int> parseMtl(std::string fileName);
	static float sumReduceVertex(glm::vec3 v);
	static unsigned int loadSingleTexture(const char* fileName);

	//ANTIGUO:
	static unsigned int loadTexture(std::string fileName);
	static unsigned int loadVao(std::vector<vertex> vertices, std::vector<texture> texturas, std::vector<normal> normales, std::vector<face> faces);

public:
	//NUEVO:
	//En caso de error se devuelve NULL
	static entity* loadEntity(const char* fileName, unsigned int shader);
	//Carga una imagen en GPU y devuelve el identificador
	static unsigned int loadRawTexture(std::string fileName);

	//ANTIGUO:
	//En principio de 2 elementos
	static unsigned int* loadObject(const char* fileName);
};

#endif // EXTERNOBJECT_H
