#pragma once
#include"GL/glew.h"
#include"glm/glm.hpp"
#include "iostream"
struct Quaterion
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
	Quaterion(GLfloat t_x = 0, GLfloat t_y = 0, GLfloat t_z = 0, GLfloat t_w = 0.0) :x(t_x), y(t_y), z(t_z), w(t_w) {}
	Quaterion(glm::vec3 vector, GLfloat t_w)
	{
		x = vector.x;
		y = vector.y;
		z = vector.z;
		w = t_w;
	}
	Quaterion operator *(Quaterion q)
	{
		Quaterion result;
		result.w = w * q.w - x * q.x - y * q.y, -z * q.z;
		result.x = w * q.x + x * q.w + y * q.z - z * q.y;
		result.y = w * q.y - x * q.z + y * q.w + z * q.x;
		result.z = w * q.z + x * q.y - y * q.x + z * q.w;
		return result;
	}
	Quaterion reserve()
	{
		return Quaterion(-x, -y, -z, w);
	}
	void show()
	{
		std::cout << w << "+" << x << "i+" << y << "j+" << z << "k" << std::endl;
	}

};
