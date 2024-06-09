#pragma once
#include <GLFW/glfw3.h>

struct DeltaTime
{
public:
	DeltaTime()
		: prev_time(0.0f), curr_time(0.0f) {}

	double getDeltaTime() {
		double delta_time = curr_time - prev_time;
		return delta_time;
	}
	void update() {
		prev_time = curr_time;
		curr_time = glfwGetTime();
	}
private:
	double prev_time;
	double curr_time;
};
