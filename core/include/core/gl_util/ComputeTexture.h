#pragma once
#include "core/gl_util/OpenGLdebugFuncs.h"

class ComputeTexture
{
private:
	unsigned int m_RendererID;
	int m_Width, m_Height;
	unsigned int m_binding_point;
public:
	ComputeTexture(int width, int height, unsigned int binding_point);
	~ComputeTexture();

	void Bind() const;
	void Unbind() const;
	void changeBindingPoint(unsigned int binding_point);

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline int ID() { return m_RendererID; }
};