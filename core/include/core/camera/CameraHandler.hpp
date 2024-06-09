#pragma once
#include "core/camera/Camera.hpp"
#include <string>
#include <GLFW/glfw3.h>

class CameraHandler 
{
public:
    CameraHandler(Camera& camera);
    
    static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        CameraHandler* cameraHandler = reinterpret_cast<CameraHandler*>(glfwGetWindowUserPointer(window));
        if (cameraHandler) {
            cameraHandler->keyCallback(window, key, scancode, action, mods);
        }
    }

    static void GLFWMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
        CameraHandler* cameraHandler = reinterpret_cast<CameraHandler*>(glfwGetWindowUserPointer(window));
        if (cameraHandler) {
            cameraHandler->mouseCursorPositionCallback(window, xpos, ypos);
        }
    }

    void mouseCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    Camera& camera;

public:
    bool CameraControllMode = false;
};
