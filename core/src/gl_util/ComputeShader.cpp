#include "core/gl_util/ComputeShader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "core/gl_util/OpenGLdebugFuncs.h"

ComputeShader::ComputeShader(const std::string& filepath)
 : m_Filepath(filepath), m_RendererID(0)
{
	m_RendererID = CreateShader();
}

ComputeShader::~ComputeShader()
{
	GLCall(glDeleteProgram(m_RendererID));
}


void ComputeShader::Bind()
{
	GLCall(glUseProgram(m_RendererID));
}

void ComputeShader::Unbind()
{
	GLCall(glUseProgram(0));
}

void ComputeShader::DrawCall(unsigned int workGroups_x, unsigned int workGroups_y, unsigned int workGroups_z)
{
	this->workGroups_x = workGroups_x;
	this->workGroups_y = workGroups_y;
	this->workGroups_z = workGroups_z;
	glDispatchCompute(workGroups_x, workGroups_y, workGroups_z);
	GLCall(glMemoryBarrier(GL_ALL_BARRIER_BITS))
}

std::string ComputeShader::ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	std::stringstream ss;
	std::string line;
	while (getline(stream, line)) // loop through every line and write it to a string stream
	{
		ss << line << '\n';
	}

	std::string source = ss.str();
	return source; // converting to c style string
}

unsigned int ComputeShader::CreateShader()
{
	std::string computeShaderSource = ParseShader(m_Filepath);
	const char* src = &computeShaderSource[0];
	GLuint computeShaderID = glCreateShader(GL_COMPUTE_SHADER);
	
	GLCall(glShaderSource(computeShaderID, 1, &src, NULL));
	GLCall(glCompileShader(computeShaderID));

	// checking for errors for debugging
	int result;
	GLCall(glGetShaderiv(computeShaderID, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(computeShaderID, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCall(glGetShaderInfoLog(computeShaderID, length, &length, message));
		std::cout << "Failed to compile the compute shader" << std::endl;
		std::cout << message << std::endl;
		GLCall(glDeleteShader(computeShaderID));
		return 0;
	}

	GLuint computeProgram = glCreateProgram(); // is m_RendererID
	GLCall(glAttachShader(computeProgram, computeShaderID));
	GLCall(glLinkProgram(computeProgram));
	// cleanup
	GLCall(glValidateProgram(computeProgram));
	GLCall(glDeleteShader(computeShaderID));

	return computeProgram;
}
