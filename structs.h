#ifndef STRUCTS_H
#define STRUCTS_H

struct TextureData
{
	const char* texturename;
	int width;
	int height;
};

struct ExportHeader
{
	int totalsize;
	int texturesize;
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
	int continuous;
	int omni;
	int seed;
	float spread;
	int glow;
	int scaleDir;
	int fade;
	glm::vec3 color;
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