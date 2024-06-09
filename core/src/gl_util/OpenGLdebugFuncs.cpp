#include "core/gl_util/OpenGLdebugFuncs.h"
#include <iostream>

void GLClearError() {
	while (glGetError() != GL_NO_ERROR) {}
}

bool GLLogCall(const char* function, const char* file, int line) {
	if (GLenum error = glGetError())
	{
		std::cout << "[OpenGL error] " << error << " on line " << line << " in " << function << " " << file << std::endl;
		return false;
	}
	return true;
}