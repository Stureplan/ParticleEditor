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
	glm::vec3 dir;
	float width;
	float height;
	int maxparticles;
	float lifetime;
	float emission;
	float force;
	float drag;
	float gravity;
	bool continuous;
};

//TODO: Separate ParticleSystemData from PlayerData
//to handle playback and stuff
struct PlayerData
{
	bool playing;

};

struct Particle
{
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 rdir;
	glm::vec3 vel;
	float ctime;
	float dist;
	bool alive;
	bool firsloop;
};


#endif


//define all the structs here