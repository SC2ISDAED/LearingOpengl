#pragma once

#include<vector>

#include"GL/glew.h"
#include"glm/glm.hpp"
#include"glm/gtc/matrix_transform.hpp"

 enum class Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
 };
//Defalut camera values
const GLfloat YAW			= -90.0f;
const GLfloat PITCH			=  0.0f;
const GLfloat SPEED			=  3.0f;
const GLfloat SENSITIVITY	=  0.25f;
const GLfloat ZOOM			=  45.0f;
const GLfloat SCROLLSPEED	= 0.05f;
class Camera 
{
public:
	//Camera Attibutes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	//Eular Angles
	GLfloat Yaw;
	GLfloat Pitch;
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
		this->Yaw = yaw;
		this->Pitch = pitch;
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
		if(direction==Camera_Movement::BACKWARD)
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
		this->Yaw += xoffset;
		this->Pitch += yoffset;
		if (constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}
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
		glm::vec3 front;
		front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		front.y = sin(glm::radians(this->Pitch));
		front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		this->Front = glm::normalize(front);

		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		this->Up	= glm::normalize(glm::cross(this->Right, this->Front));
	}
};