#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <gl/SOIL.h>
#include <ctime>
#include <random>
#include <algorithm>

#include "structs.h"

#pragma comment(lib, "SOIL.lib")

using namespace glm;

class ParticleSystem
{
public:
	ParticleSystem();
	ParticleSystem(ParticleSystemData*, TextureData*, glm::vec3, GLuint, GLuint);
	~ParticleSystem();

public:
	void Initialize();
	void Rebuild (ParticleSystemData*);
	void Retexture (TextureData*);
	void Update(double, bool, ParticleSystemData*, glm::vec3);
	void Render();
	void RenderLightning();

	glm::mat4 GetModel();
	ParticleSystemData* GetPSData();
	TextureData* GetTextureData();
	int GetActiveParticles();
	bool IsPlaying();

	void Play();
	void Pause();

private:
	GLuint texture;
	GLuint vertexbuffer;
	GLuint dirbuffer;
	GLuint vtxpos;
	GLuint vtxdir;
	GLuint m_shader;
	GLuint m_lshader;

	ParticleSystemData* m_particleinfo;
	TextureData* m_textureinfo;

	std::vector<glm::vec3> m_vertices;
	std::vector<Particle> m_particles;
	std::vector<glm::vec3> m_directions;

	glm::vec3 m_position;
	glm::vec3 m_direction;
	float m_currentCD;
	bool m_continuous;
	int m_deadparticles;
	int m_activeparticles;

	bool m_playing;

	glm::mat4 Model;
	glm::mat4 LookAt = glm::mat4(1.0f);
};



#endif