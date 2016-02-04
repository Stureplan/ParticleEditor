#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Camera
{
public:
	void Initialize (glm::vec3, glm::vec3, int, int);
	void UpdateView	();
	void SetPos		(glm::vec3);
	void SetDir		(glm::vec3);

	glm::vec3 GetPos ();
	glm::mat4 GetView();
	glm::mat4 GetProj();
	glm::mat4 GetOrtho ();
	
private:
	glm::mat4 View			= glm::mat4(1.0f);
	glm::mat4 Projection	= glm::mat4(1.0f);
	glm::mat4 Ortho			= glm::mat4 (1.0f);

	glm::vec3 m_position	= glm::vec3 (0.0f, 4.0f, -8.0f);
	glm::vec3 m_direction	= glm::vec3 (0.0f, 0.0f, -1.0f);
	glm::vec3 m_up			= glm::vec3 (0.0f, 1.0f, 0.0f);

};

#endif