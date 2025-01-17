#ifndef SHADER_H
#define SHADER_H

//#include <stdio.h>
#include <iostream>

#include <glad/glad.h>
#include <glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

enum class ShaderType {
	VERTEX = 0,
	FRAGMENT = 1
};

struct ShadersData {
	std::string vertexSource;
	std::string fragmentSource;
};

class Shader {

private:
	unsigned int rendererID;

public:
	Shader() : rendererID(0){};
	Shader(const char* filepath);
	~Shader();

private:
	ShadersData Parse(const char* filepath);
	void Compile(ShadersData sources, unsigned int* shaders);
	void Create(ShadersData sources);


	inline unsigned int GetUniformLocation(std::string name) { return glGetUniformLocation(this->rendererID, name.c_str()); }
public:
	void Bind();
	//void Unbind();

	void SetUniformMat4(const std::string& name, const glm::mat4& matrix) {
		unsigned int location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
	}

	void SetUniform3f(const std::string& name, float f1, float f2, float f3) {
		unsigned int location = GetUniformLocation(name);
		glUniform3f(location, f1, f2, f3);
	}

	void SetUniform2f(const std::string& name, float f1, float f2) {
		unsigned int location = GetUniformLocation(name);
		glUniform2f(location, f1, f2);
	}

	void SetUniformFloat(const std::string& name, float value) {
		unsigned int location = GetUniformLocation(name);
		glUniform1f(location, value);
	}

	void SetUniformInt(const std::string& name, int value) {
		unsigned int location = GetUniformLocation(name);
		glUniform1i(location, value);
	}

	void SetUniformUInt(const std::string& name, unsigned int value) {
		unsigned int location = GetUniformLocation(name);
		glUniform1ui(location, value);
	}
};

class SSBO {
	uint32_t buffer = 0;

public:
	SSBO();
	~SSBO();

	void Bind(unsigned int bind = 0);
	void SendData(uint32_t size, void* data);
	void Unbind();
};


#endif // SHADER_H