#pragma once
#include "vector"
#include "string"
#include <fstream>
#include <sstream>
#include <iostream>
#include"glm/glm.hpp"
#include "gl/glew.h"
#include "shader.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "src/SOIL.h"
GLint TextureFromFile(const char * path, std::string directory);
class Model
{
public:
	Model(const GLchar * path)
	{
		this->loadModel(path);
	}
	void Draw(Shader shader)
	{
		//为每个meshs调用着色器
		for (GLuint i = 0; i < this->meshes.size(); i++)
		{
			this->meshes[i].Draw(shader);
		}
	}
private:
	/*Model index*/
	std::vector<Mesh> meshes;
	std::string directory;
	//存贮已经加载过的材质
	std::vector<Texture> textures_loaded;
	/*private function*/
	void loadModel(std::string path)
	{
		//读取模型路径
		Assimp::Importer importModel;
		const aiScene * scene = importModel.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << importModel.GetErrorString() << std::endl;
			return;
		}
		this->directory = path.substr(0, path.find_last_of('/'));
		//递归处理ASSIMP的数据结点
		this->processNode(scene->mRootNode, scene);
	}
	void processNode(aiNode * node, const aiScene * scene)
	{
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			//结点只是保存了对实际对象的索引
			aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh * mesh, const aiScene * scene)
	{
		//存储数据
		std::vector<Vertex> vertices;
		std::vector<GLuint> inidices;
		std::vector<Texture> textures;
		//Assimp 中的 vector 并不能直接转换为glm中的vector，首先进行转换
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			//Position
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;
			//Normal（法向量）
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			//Texture Coordinates(纹理)
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			// tangent
			if (mesh->mTangents)
			{
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
			}
			// bitangent
			if (mesh->mBitangents) 
			{
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			
			vertices.push_back(vertex);
		}
		// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		//每一个网格都有一个三角形（面片），我们将索引每一个mesh的面，并且取得这些面的索引
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
			{
				inidices.push_back(face.mIndices[j]);
			}
		}
		//处理纹理
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];
			// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// Diffuse: texture_diffuseN
			// Specular: texture_specularN
			// Normal: texture_normalN
			//假设了我们遵循着色器采样器的命名的原则。
			//例如命名为'texture_diffuseN' 是第N个到第一个MAX_SAMPLER_NUMBER的序号；
			//其他的纹理也遵循同样的规则
			//1.Diffuse maps;
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			//2.Specular maps
			std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			//3.normal.map
			std::vector<Texture> normalmaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			//4.height maps
			std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}
		return Mesh(vertices, inidices, textures);
	}
	std::vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;
		GLuint i = 0;
		for (; i < mat->GetTextureCount(type); i++);
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			//如果当前纹理已经加载过了，则跳到下一个迭代结点加载，
			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				//如果没有加载过则load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);
			}

		}
		return textures;
	}
};
GLint TextureFromFile(const char * path, std::string directory)
{
	//生成纹理ID并且加载材质数据
	std::string filename = std::string(path);
	filename = directory + '/' + filename;
	GLuint textrueID;
	glGenTextures(1, &textrueID);
	int width, height;
	unsigned char * image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	//Assign texture to ID;
	glBindTexture(GL_TEXTURE_2D, textrueID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	//Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textrueID;
}
