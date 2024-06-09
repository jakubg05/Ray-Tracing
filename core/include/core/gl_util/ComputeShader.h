#pragma once
#include <string>

class ComputeShader {
public:
	ComputeShader(const std::string& filepath);
	~ComputeShader();

	void Bind();
	void Unbind();
	void DrawCall(unsigned int workGroups_x, unsigned int workGroups_y, unsigned int workGroups_z);

	unsigned int m_RendererID;
	
	unsigned int workGroups_x;
	unsigned int workGroups_y;
	unsigned int workGroups_z;

private:
	const std::string& m_Filepath;

	

	unsigned int CreateShader();
	std::string ParseShader(const std::string& filepath);

};
