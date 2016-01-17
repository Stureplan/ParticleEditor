#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <gl/SOIL.h>

#include "structs.h"

#pragma comment(lib, "SOIL.lib")

using namespace glm;

class ParticleSystem
{
public:
	ParticleSystem();
	ParticleSystem(ParticleSystemData*, TextureData*, glm::vec3, GLuint);
	~ParticleSystem();

public:
	void Initialize();
	void Rebuild (TextureData*);
	void Update(double, glm::vec3);
	void Render();

	glm::mat4 GetModel();
	
	void Rebuild(int, int, int, float, glm::vec3, glm::vec3);
	void Shutdown();
	void Play();

private:
	GLuint texture;
	GLuint vertexbuffer;
	GLuint vtxpos;
	GLuint m_shader;

	ParticleSystemData m_particleinfo;
	TextureData* m_textureinfo;

	std::vector<glm::vec3> m_vertices;
	std::vector<Particle> m_particles;

	glm::vec3 m_position;
	glm::vec3 m_direction;
	float grav = 0.0f;

	glm::mat4 Model;
	glm::mat4 LookAt = glm::mat4(1.0f);
};



#endif