#include "Rendering/Camera.h"

Camera::Camera(float aspectRatio, float yFOV, float nearClip, float farClip)
{
	m_AspectRatio = aspectRatio;
	m_FOV = yFOV;
	m_NearClip = nearClip;
	m_FarClip = farClip;

	m_Position = glm::vec3(0.0);

	UpdateProjectionMatrix();
	UpdateViewMatrix();
}

glm::vec3 Camera::Forward()
{
	return m_Orientation * glm::vec3(0, 0, -1);
}
//
//glm::vec3 Camera::Right()
//{
//	return glm::rotate(glm::vec3(1.f, 0.f, 0.f), -m_Yaw, glm::vec3(0.f, 1.f, 0.f));
//}

//glm::mat4 Camera::Orientation()
//{
//	glm::mat4 orientation(1.f);
//	orientation = glm::rotate(orientation, m_Pitch, glm::vec3(1.f, 0.f, 0.f));
//	orientation = glm::rotate(orientation, m_Yaw, glm::vec3(0.f, 1.f, 0.f));
//	return orientation;
//}

void Camera::SetPosition(glm::vec3 val)
{
	m_Position = val;
	UpdateViewMatrix();
}


void Camera::SetOrientation(glm::quat val)
{
	m_Orientation = val;
	UpdateViewMatrix();
}

//void Camera::Pitch(float val)
//{
//	m_Pitch = val;
//	UpdateViewMatrix();
//}
//
//void Camera::Yaw(float val)
//{
//	m_Yaw = val;
//	UpdateViewMatrix();
//}

void Camera::UpdateProjectionMatrix()
{
//	m_ProjectionMatrix = glm::ortho(
//		-16.f,
//		16.f,
//		-9.f,
//		9.f,
//		m_NearClip,
//		m_FarClip
//		);

	m_ProjectionMatrix = glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
}

void Camera::UpdateViewMatrix()
{
	m_ViewMatrix = glm::toMat4(glm::inverse(m_Orientation))
		* glm::translate(-m_Position);
}

void Camera::SetAspectRatio(float val)
{
	m_AspectRatio = val;
	UpdateProjectionMatrix();
}

void Camera::SetFOV(float val)
{
	m_FOV = val;
	UpdateProjectionMatrix();
}

void Camera::SetNearClip(float val)
{
	m_NearClip = val;
	UpdateProjectionMatrix();
}

void Camera::SetFarClip(float val)
{
	m_FarClip = val;
	UpdateProjectionMatrix();
}