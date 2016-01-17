#ifndef STRUCTS_H
#define STRUCTS_H

struct TextureData
{
	const char* texturename;
	int width;
	int height;
};

struct ParticleSystemData
{
	float width;
	float height;
	int maxparticles;
	float lifetime;
	float time_offset;
	float time_offset_total;
	float force;
	float gforce;
};

struct Particle
{
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 vel;
	float ctime;
};

#endif


//define all the structs here