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

glm::vec3 Camera::Right()
{
	return m_Orientation * glm::vec3(1, 0, 0);
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


void Camera::SetProjectionMatrix(glm::mat4 val)
{
    m_ProjectionMatrix = val;
}


void Camera::SetViewMatrix(glm::mat4 val)
{
    m_ViewMatrix = val;
}


glm::mat4 Camera::BillboardMatrix()
{
    glm::mat4 matrix = glm::toMat4(m_Orientation);
    return matrix; 
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
	m_ProjectionMatrix = glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
}

glm::vec2 Camera::WorldToScreen(glm::vec3 worldCoord, Rectangle resolution)
{
    glm::vec4 screenCoord = m_ProjectionMatrix * m_ViewMatrix * glm::vec4(worldCoord, 1.f);
    if (screenCoord.w != 0) {
        screenCoord.x /= screenCoord.w;
        screenCoord.y /= screenCoord.w;
        screenCoord.z /= screenCoord.w;
    }
    screenCoord.x = screenCoord.x * (resolution.Width / 2.f);
    screenCoord.y = screenCoord.y * (resolution.Height / 2.f);
    return glm::vec2(screenCoord);
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