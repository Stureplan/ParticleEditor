#include "camera.h"

void Camera::Initialize ()
{
	View = glm::lookAt
	(
		glm::vec3 (0, 4.0f, -8.0f),	//pos
		glm::vec3 (0, 0, 1),		//at
		glm::vec3 (0, 1, 0)			//up
	);

	Projection = glm::perspective
	(
		glm::radians (45.0f),		//FOV
		16.0f / 9.0f,				//aspect ratio
		0.1f,						//near
		100.0f						//far
	);
}

void Camera::UpdateView ()
{
	View = glm::lookAt
	(
		m_position,
		m_direction,
		m_up
	);
}

void Camera::SetPos (glm::vec3 pos)
{
	m_position = pos;
}

void Camera::SetDir (glm::vec3 dir)
{
	m_direction = dir;
}

glm::vec3 Camera::GetPos ()
{
	return m_position;
}

glm::mat4 Camera::GetView ()
{
	return View;
}

glm::mat4 Camera::GetProj ()
{
	return Projection;
}