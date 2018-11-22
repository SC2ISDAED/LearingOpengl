#pragma once
#pragma once

#include<vector>
#include "Quaterionh.h"
#include"GL/glew.h"
#include"glm/glm.hpp"
#include"glm/gtc/matrix_transform.hpp"
#include "iostream"
enum class Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};
//Defalut camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVITY = 0.125f;
const GLfloat ZOOM = 45.0f;
const GLfloat SCROLLSPEED = 0.05f;
class Camera
{
public:
	//Camera Attibutes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	//Quaterion
	Quaterion quaterionrate;
	//Rotate
	GLfloat x_change;
	GLfloat y_change;
	GLfloat x_radius;
	GLfloat y_radius;
	//Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;
	GLfloat ScrollSpeed;
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		GLfloat yaw = YAW, GLfloat pitch = PITCH)
		:Front(glm::vec3(0.0f, 0.0f, -1.0f)),
		MovementSpeed(SPEED),
		MouseSensitivity(SENSITIVITY),
		Zoom(ZOOM),
		ScrollSpeed(SCROLLSPEED)
	{
		this->Position = position;
		this->WorldUp = up;
		
		this->x_radius = 0;
		this->y_radius = 0;
		quaterionrate = Quaterion(Front, 0.0);
		this->updateCameraVectors();
	}
	//Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
	}
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;
		if (direction == Camera_Movement::FORWARD)
			this->Position += this->Front *velocity;
		if (direction == Camera_Movement::BACKWARD)
			this->Position -= this->Front *velocity;
		if (direction == Camera_Movement::LEFT)
			this->Position -= this->Right *velocity;
		if (direction == Camera_Movement::RIGHT)
			this->Position += this->Right *velocity;
	}
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;
		x_radius += xoffset;
		y_radius += yoffset;
		x_change = -xoffset;
		y_change = yoffset;
		std::cout <<"x:"<<x_radius<<"y:" <<y_radius<<std::endl;
		this->updateCameraVectors();
	}

	void ProcessMouseScroll(GLfloat yoffset)
	{
		yoffset *= ScrollSpeed;
		if (this->Zoom >= 1.0f&&this->Zoom <= 45.0f)
			this->Zoom -= yoffset;
		if (this->Zoom <= 1.0f)
			this->Zoom = 1.0f;
		if (this->Zoom >= 45.0f)
			this->Zoom = 45.0f;
	}
private:
	void updateCameraVectors()
	{
		Quaterion y_pitch(0, sin(glm::radians(this->x_change / 2)), 0, cos(glm::radians(this->x_change)));
		Quaterion x_yaw(sin(glm::radians(this->y_change / 2)), 0, 0, cos(glm::radians(this->y_change)));
		quaterionrate = y_pitch * quaterionrate*y_pitch.reserve();
		quaterionrate = x_yaw * quaterionrate*x_yaw.reserve();
		glm::vec3 front(quaterionrate.x, quaterionrate.y, quaterionrate.z);

		this->Front = glm::normalize(front);
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
	}
};