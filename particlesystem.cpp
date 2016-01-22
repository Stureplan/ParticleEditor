#include "particlesystem.h"

ParticleSystem::ParticleSystem(){ }

ParticleSystem::ParticleSystem(ParticleSystemData* particleinfo, TextureData* textureinfo, glm::vec3 position, GLuint shader)
{
	vertexbuffer = 0;
	vtxpos = 0;

	m_shader = 0;

	this->m_textureinfo = textureinfo;
	//this->m_particleinfo = particleinfo;
	m_particleinfo.width = particleinfo->width;
	m_particleinfo.height = particleinfo->height;
	m_particleinfo.maxparticles = particleinfo->maxparticles;
	m_particleinfo.lifetime = particleinfo->lifetime;
	m_particleinfo.time_offset = particleinfo->time_offset;
	m_particleinfo.time_offset_total = particleinfo->time_offset_total;
	m_particleinfo.force = particleinfo->force;
	m_particleinfo.gforce = particleinfo->gforce;

	this->time_offset = particleinfo->time_offset;
	this->m_position = position;
	this->m_shader = shader;

	Initialize();
}

ParticleSystem::~ParticleSystem() { }

void ParticleSystem::Initialize()
{
	//Initiate random gen
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);



	//Fill the vertex data vector with [maxparticles] vertices
	for (int i = 0; i < m_particleinfo.maxparticles; i++)
	{
		glm::vec3 vertex = m_position;
		m_vertices.push_back(vertex);
		
		Particle p;
		p.pos = m_position;
		float x = dist(mt);
		float y = dist(mt);
		float z = dist(mt);
		p.dir = glm::vec3(x, y, z);
		p.ctime = 0.0f;
		p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
		p.dist = -1.0f;

		m_particles.push_back(p);
	}


/*	for (int i = 0; i < m_textureinfo.size(); i++)
	{
		glGenTextures(1, &m_textureinfo[i]);
		
		unsigned char* image = SOIL_load_image(
			m_textureinfo[i]->texturename,
			&m_textureinfo[i]->width,
			&m_textureinfo[i]->height,
			0, SOIL_LOAD_RGBA);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			m_textureinfo[i]->width,
			m_textureinfo[i]->height,
			0, GL_RGBA, GL_UNSIGNED_BYTE,
			image);

		SOIL_free_image_data(image);
	}
*/

	//Load texture
	glGenTextures (1, &texture);
	glBindTexture (GL_TEXTURE_2D, texture);
	unsigned char* image = SOIL_load_image(
		m_textureinfo->texturename,
		&m_textureinfo->width,
		&m_textureinfo->height,
		0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureinfo->width, m_textureinfo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(m_shader, "tex"), 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	Model = glm::mat4(1.0f);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
	vtxpos = glGetAttribLocation(m_shader, "vertex_position");
}

void ParticleSystem::Rebuild (TextureData* textureinfo)
{
	texture = 0;

	this->m_textureinfo = textureinfo;

	glGenTextures (1, &texture);
	glBindTexture (GL_TEXTURE_2D, texture);
	unsigned char* image = SOIL_load_image (
		m_textureinfo->texturename,
		&m_textureinfo->width,
		&m_textureinfo->height,
		0, SOIL_LOAD_RGBA);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_textureinfo->width, m_textureinfo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data (image);
	glUniform1i (glGetUniformLocation (m_shader, "tex"), 0);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ParticleSystem::Update(double deltaTime, glm::vec3 dir, float gravity, bool direction, ParticleSystemData* part, glm::vec3 campos)
{
	m_particleinfo.dir = part->dir;
	m_particleinfo.force = part->force;
	m_particleinfo.gforce = part->gforce;
	m_particleinfo.height = part->height;
	m_particleinfo.lifetime = part->lifetime;
	m_particleinfo.maxparticles = part->maxparticles;
	m_particleinfo.time_offset = part->time_offset;
	m_particleinfo.time_offset_total = part->time_offset_total;
	m_particleinfo.width = part->width;
	
	if (time_offset > 0.0f)
	{
		time_offset -= deltaTime;
	}
	
	for (int i = 0; i < m_particleinfo.maxparticles; i++)
	{
		//Get a reference to the current particle being processed
		Particle& p = m_particles.at(i);
		
		//Check for direction here: if (m_direction) p.vel += m_direction;

		//If the particle still has "time", it's alive and
		//needs to be updated and moved.
		if (p.ctime > 0.0f)
		{
			//If it still has life left, decrease life by dT
			p.ctime -= deltaTime;
			
			float percent = p.ctime / m_particleinfo.lifetime;

			//Increase velocity as time goes on
			if (direction)
			{
				p.vel += dir * (float)deltaTime;
			}
			else
			{
				p.vel += p.dir * (float)deltaTime;
			}


			p.pos.x = p.vel.x * m_particleinfo.force;
			//p.pos.y = p.vel.y * m_particleinfo.force;
			p.pos.z = p.vel.z * m_particleinfo.force;

			//p.pos.y += (-9.81f + (p.ctime * 2)) * (float)deltaTime;
			//p.pos.y += (m_particleinfo.gforce + p.ctime * 5) * (float)deltaTime;
			p.pos.y += ((-9.81f + percent * 5) * gravity) * (float)deltaTime;

			p.dist = glm::length(p.pos - campos);
			m_vertices.at(i) = p.pos;
		}

		//If current lifetime and offset time has been reached,
		//reset particle and move it back
		if (p.ctime <= 0.0f && time_offset <= 0.0f)
		{
			p.ctime = m_particleinfo.lifetime;
			p.pos = m_position;
			p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
			p.dist = -1.0f;

			time_offset = m_particleinfo.time_offset_total;
		}
	}

	std::sort(&m_particles[0], &m_particles[m_particleinfo.maxparticles-1]);
}

void ParticleSystem::Render()
{
	glUseProgram (m_shader);
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Texture
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, texture);
	glUniform1i (glGetUniformLocation (m_shader, "tex"), 0);

	glEnableVertexAttribArray(vtxpos);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STREAM_DRAW);
	glVertexAttribPointer(vtxpos, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glDrawArrays(GL_POINTS, 0, m_vertices.size());

	glDisableVertexAttribArray(vtxpos);
}

glm::mat4 ParticleSystem::GetModel()
{
	return this->Model;
}

ParticleSystemData ParticleSystem::GetPSData()
{
	return this->m_particleinfo;
}

TextureData ParticleSystem::GetTextureData()
{
	TextureData temp;
	temp.texturename = m_textureinfo->texturename;
	temp.width = m_textureinfo->width;
	temp.height = m_textureinfo->height;

	return temp;
}