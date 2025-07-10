#pragma once
#include <GL/glew.h>

#if defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #include <signal.h>
    #define DEBUG_BREAK() raise(SIGTRAP)
#else
    #define DEBUG_BREAK() ((void)0)
#endif

#define ASSERT(x) if (!(x)) DEBUG_BREAK()
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
