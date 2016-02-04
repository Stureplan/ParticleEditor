#include "particlesystem.h"

ParticleSystem::ParticleSystem(){ }

ParticleSystem::ParticleSystem(ParticleSystemData* particleinfo, TextureData* textureinfo, glm::vec3 position, GLuint shader, GLuint lshader)
{
	vertexbuffer = 0;
	vtxpos = 0;

	m_shader = 0;
	m_lshader = 0;

	//Set texture
	this->m_textureinfo = textureinfo;
	this->m_particleinfo = particleinfo;
	this->m_currentCD = m_particleinfo->rate;
	this->m_position = position;
	this->m_shader = shader;
	this->m_lshader = lshader;

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
	for (int i = 0; i < m_particleinfo->maxparticles; i++)
	{
		glm::vec3 vertex = m_position;
		m_vertices.push_back(vertex);
		
		Particle p;
		p.pos = m_position;
		float x = dist(mt);
		float y = dist(mt);
		float z = dist(mt);
		p.rdir = glm::vec3(x, y, z);
		p.ctime = m_particleinfo->lifetime;
		p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
		p.dist = -1.0f;
		p.alive = false;
		p.firsloop = true;

		m_particles.push_back(p);
		m_directions.push_back(p.rdir);
	}

	m_deadparticles = 0;
	m_playing = true;

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

	glGenBuffers(1, &dirbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, dirbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_directions.size() * sizeof(glm::vec3), &m_directions[0], GL_STATIC_DRAW);
	vtxdir = glGetAttribLocation(m_shader, "vertex_direction");
}

void ParticleSystem::Rebuild (ParticleSystemData* particleinfo)
{	
	this->m_particleinfo = particleinfo;
	this->m_vertices.resize(m_particleinfo->maxparticles);
	this->m_particles.resize(m_particleinfo->maxparticles);
	this->m_directions.resize(m_particleinfo->maxparticles);

	//Initiate random gen
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	//Fill the vertex data vector with [maxparticles] vertices
	for (int i = 0; i < m_particleinfo->maxparticles; i++)
	{
		Particle p;
		p.pos = m_position;
		float x = dist(mt);
		float y = dist(mt);
		float z = dist(mt);
		p.rdir = glm::vec3(x, y, z);
		p.ctime = m_particleinfo->lifetime;
		p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
		p.dist = -1.0f;
		p.alive = false;
		p.firsloop = true;
		m_currentCD = m_particleinfo->rate;

		m_particles.at(i) = p;
		m_vertices.at(i) = p.pos;
		m_directions.at(i) = p.rdir;
	}

	m_deadparticles = 0;

	Model = glm::mat4(1.0f);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
	vtxpos = glGetAttribLocation(m_shader, "vertex_position");

	glGenBuffers(1, &dirbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, dirbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_directions.size() * sizeof(glm::vec3), &m_directions[0], GL_STATIC_DRAW);
	vtxdir = glGetAttribLocation(m_shader, "vertex_direction");
}

void ParticleSystem::Retexture(TextureData* textureinfo)
{
	texture = 0;

	this->m_textureinfo = textureinfo;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
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
}

void ParticleSystem::Play()
{
	m_playing = true;
}

void ParticleSystem::Pause()
{
	m_playing = false;
}
//TODO: Make sure particles aren't rendered when dead. (Priority)

void ParticleSystem::Update(double deltaTime, bool directional, ParticleSystemData* part, glm::vec3 campos)
{
	if (m_playing)
	{
		m_activeparticles = m_particleinfo->maxparticles;
		float dT = (float)deltaTime;

		m_particleinfo = part;

		if (m_currentCD > 0.0f)
		{
			m_currentCD -= dT;
		}

		for (int i = 0; i < m_particles.size(); i++)
		{
			//Get a reference to the current particle being processed
			Particle& p = m_particles.at(i);

			//If the particle still has "time", it's alive and needs to be updated and moved.
			if (p.ctime > 0.0f && p.alive == true)
			{
				//If it still has life left, decrease life by dT
				p.ctime -= dT;

				//How many percent of the particle's lifetime has been travelled
				float percent = p.ctime / m_particleinfo->lifetime;

				//Increase velocity as time goes on
				if (directional)
				{
					m_directions.at(i) = glm::normalize(glm::vec3(p.dir));
					p.vel = m_particleinfo->dir * dT;
				}
				else
				{
					m_directions.at(i) = glm::normalize(glm::vec3(p.dir));

					//Velocity is correct
					p.vel = p.rdir * dT;
				}
				//Separate container for random dir
				glm::vec3 oldPos = p.pos;

				//Lastly, add gravity
				p.pos.y += ((-9.81f + percent * 10) * m_particleinfo->gravity) * dT;

				//Add the velocity to the position
				p.pos.x -= p.vel.x * m_particleinfo->force;
				p.pos.y += p.vel.y * m_particleinfo->force;
				p.pos.z -= p.vel.z * m_particleinfo->force;
				
				p.dir = oldPos - p.pos;


				p.dist = glm::length(p.pos - campos);
				m_vertices.at(i) = p.pos;
			}
 
			//If current lifetime is reached and particle still alive,
			//kill it and reset particle lifetime
			else if (p.ctime <= 0.0f && p.alive == true)
			{
				p.alive = false;
				p.firsloop = false;
				if (m_particleinfo->continuous == true)
				{
					p.ctime = m_particleinfo->lifetime;
				}
				p.pos = m_position;
				p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
				p.dist = -1.0f;

				m_vertices.at(i) = p.pos;
			}

			//If cooldown is reached and particle dead, wake particle
			else if (m_currentCD <= 0.0f && p.alive == false)
			{
				if (m_particleinfo->continuous)
				{
					m_activeparticles++;

					p.alive = true;
					p.ctime = m_particleinfo->lifetime;
					p.pos = m_position;
					p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
					p.dist = -1.0f;

					m_currentCD = m_particleinfo->rate;

					m_vertices.at(i) = p.pos;
				}
				else
				{
					if (p.firsloop == true)
					{
						m_activeparticles++;

						p.alive = true;
						p.ctime = m_particleinfo->lifetime;
						p.pos = m_position;
						p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
						p.dist = -1.0f;

						m_currentCD = m_particleinfo->rate;

						m_vertices.at(i) = p.pos;
					}
				}


			}

			if (p.alive == false)
			{
				if (m_particleinfo->continuous)
				{
					m_activeparticles--;
				}
				p.pos = glm::vec3(0.0f, 2.0f, 0.0f);
				p.vel = glm::vec3(0, 0, 0);

				m_vertices.at(i) = p.pos;
			}
		}

	}

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
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vtxpos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);

	glEnableVertexAttribArray(vtxdir);
	glBindBuffer(GL_ARRAY_BUFFER, dirbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_directions.size() * sizeof(glm::vec3), &m_directions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(vtxdir, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*) 0);
	

	glDrawArrays(GL_POINTS, 0, m_vertices.size());

	glDisableVertexAttribArray(vtxpos);
	glDisableVertexAttribArray(vtxdir);
}

void ParticleSystem::RenderLightning()
{
	glUseProgram(m_lshader);
	glEnableVertexAttribArray(vtxpos);
	
	
	//Initiate random gen
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int32_t> dist(1, m_vertices.size());
	
	std::uniform_real_distribution<float> distf(1.0f, 5.0f);
	glLineWidth(distf(mt));

	glDrawArrays(GL_LINE_STRIP, 0, dist(mt));
	

	
	glDisableVertexAttribArray(vtxpos);
}

glm::mat4 ParticleSystem::GetModel()
{
	return this->Model;
}

ParticleSystemData* ParticleSystem::GetPSData()
{
	return this->m_particleinfo;
}

TextureData* ParticleSystem::GetTextureData()
{
	return this->m_textureinfo;
}

int ParticleSystem::GetActiveParticles()
{
	return this->m_activeparticles;
}

bool ParticleSystem::IsPlaying()
{
	return this->m_playing;
}