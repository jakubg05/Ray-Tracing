# Ray Tracing from scratch in C++

![image](https://raw.githubusercontent.com/jakubg05/images/main/RayTracing.png)

## Building The Project
### Windows (Visual Studio)

1. Clone this repository:
```
git clone --recursive https://github.com/jakubg05/ray-tracing.git your-project-name
cd your-project-name
git submodule update --init --recursive 
```
2. Right-click the cloned folder and select "Open with Visual Studio," or open Visual Studio and navigate to File -> Open -> CMake... and choose the root CMakeLists.txt.
3. Run the Debug build.
4. Create a Release configuration by clicking the x64-Debug dropdown and adding x64-Release.
5. Choose raytracing.exe from the box on the right and hit the run button.

## Project Structure and CMake

* **Core as a Static Library:** The `core` directory contains its own CMakeLists.txt file and is configured to be built as a static library. This means the compiled code from the `core` directory is packaged into a single `.lib` (Windows) or `.a` (Linux) file.

* **Application Linking:** The `app` directory doesn't have its own CMakeLists.txt. Instead, the root CMakeLists.txt handles:
* Building the executable for the application
* Linking the application's executable against the `core` static library
* Linking the application's executable against the third-party libraries (GLFW, GLEW, etc.)

![alt text](https://raw.githubusercontent.com/jakubg05/images/main/RayTracingDepGraph.png)
