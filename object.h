#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdio.h>
#include <cstring>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <gl/SOIL.h>

#include "structs.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")

using namespace glm;

class Object
{
//Constructors & destructor
public:
	Object ();
	Object (const char*, TextureData*, glm::vec3, GLuint, bool);
	~Object ();

//Functions
public:
	void Initialize ();
	void Rebuild (TextureData*);
	void Update();
	void Render ();
	void Translate  (glm::vec3);
	void Rotate		(glm::vec3);
	void Rescale	(glm::vec3);
	glm::mat4 GetModel();
	glm::vec3 GetScale();
	bool IsActive();
	void SetActive(bool);

private:
	bool LoadOBJ (const char*);

//Variables
private:
	//object stats
	float movespeed;
	bool m_active;
	bool m_textured;

	//matrices
	glm::mat4 Model;
	glm::mat4 LookAt = glm::mat4(1.0f);

	//vectors
	glm::vec3 m_position  = glm::vec3 (0.0f, 0.0f, 0.0f);	//world position
	glm::vec3 m_rotation  = glm::vec3 (0.0f, 0.0f, 0.0f);	//direction vector
	glm::vec3 m_scale	  = glm::vec3 (1.0f, 1.0f, 1.0f);	//local scale
	glm::vec3 m_up		  = glm::vec3 (0.0f, 1.0f, 0.0f);	//up vector

	//buffers and pointers
	GLuint texture;
	GLuint vertexbuffer;
	GLuint colorbuffer;
	GLuint uvbuffer;

	GLuint vtxpos;
	GLuint vtxcol;
	GLuint vtxuv;

	GLuint m_shader;

	//file data
	const char* m_filename;
	TextureData* m_textureinfo;
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec2> m_uvs;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_colors;
};

#endif


