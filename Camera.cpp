#include "camera.h"

void Camera::Initialize(glm::vec3 pos, glm::vec3 at, int width, int height)
{
	View = glm::lookAt
		(
			pos,							//pos
			at,								//at
			glm::vec3(0, 1, 0)				//up
			);

	Projection = glm::perspective
		(
			glm::radians(45.0f),			//FOV
			(float)width / (float)height,	//aspect ratio
			0.1f,							//near
			100.0f							//far
			);

	Ortho = glm::ortho
		(
			0.0f,
			(float)width,
			(float)height,
			0.0f
			);

	m_position = pos;
	m_direction = at;
}

void Camera::UpdateView()
{
	View = glm::lookAt
		(
			m_position,
			glm::vec3(0, 0, 0),
			m_up
			);
}

void Camera::SetPos(glm::vec3 pos)
{
	m_position = pos;
}

void Camera::SetDir(glm::vec3 dir)
{
	m_direction = dir;
}

void Camera::SetView(glm::mat4 v)
{
	View = v;
}

glm::vec3 Camera::GetPos()
{
	return m_position;
}

glm::mat4 Camera::GetView()
{
	return View;
}

glm::mat4 Camera::GetProj()
{
	return Projection;
}

glm::mat4 Camera::GetOrtho()
{
	return Ortho;
}