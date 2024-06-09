#pragma once
#include <glm/glm.hpp>


#ifndef RTX_MATERIAL
#define RTX_MATERIAL
struct RaytracingMaterial
{
    // Thanks to https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL

    glm::vec3 color;              // offset 0   // alignment 16 // size 12 // total 12 bytes
    float emissionStrength;       // offset 12  // alignment 4  // size 4  // total 16 bytes
    glm::vec3 emissionColor;      // offset 16  // alignment 16 // size 12 // total 28 bytes
    float std140padding;          // offset 28  // alignment 4  // size 4  // total 32 bytes
};
#endif

struct Sphere {
	RaytracingMaterial material;  // offset 0   // alignment 16 // size 32 // total 32 bytes
	glm::vec3 position;           // offset 32  // alignment 16 // size 12 // total 44 bytes
	float radius;                 // offset 44  // alignment 4  // size 4  // total 48 bytes
};  

SceneData defaultScene() {

    Sphere* spheres = new Sphere[4];

    Sphere sphere1;
    sphere1.position = glm::vec3(0.0f, -10.0f, 5.0f);
    sphere1.radius = 9.0f;
    sphere1.material.color = glm::vec3(0.807f, 0.2588f, 0.2588f); // Red color
    sphere1.material.emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
    sphere1.material.emissionStrength = 0.0f;

    Sphere sphere2;
    sphere2.position = glm::vec3(-476.0f, 513.0f, 0.0f);
    sphere2.radius = 50.0f;
    sphere2.material.color = glm::vec3(0.0f, 0.0f, 0.0f);
    sphere2.material.emissionColor = glm::vec3(1.0f, 0.9f, 1.0f);
    sphere2.material.emissionStrength = 3.0f;

    Sphere sphere3;
    sphere3.position = glm::vec3(0.3f, 2.0f, 10.0f);
    sphere3.radius = 0.7f;
    sphere3.material.color = glm::vec3(1.0f, 1.0f, 0.9f); // skin color
    sphere3.material.emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
    sphere3.material.emissionStrength = 0.0f;

    Sphere sphere4;
    sphere4.position = glm::vec3(-3.0f, -0.9f, 5.0f);
    sphere4.radius = 0.8f;
    sphere4.material.color = glm::vec3(0.0f, 0.0f, 0.0f); // Yellow color
    sphere4.material.emissionColor = glm::vec3(1.0f, 1.0f, 1.0f);
    sphere4.material.emissionStrength = 4.0f;

    spheres[0] = sphere1;
    spheres[0] = sphere2;
    spheres[2] = sphere3;
    spheres[3] = sphere4;

    SceneData sceneData;
    sceneData.sceneObjects = spheres;
    sceneData.numberOfObjects = 4;
    sceneData.size = sceneData.numberOfObjects * sizeof(Sphere);

	return sceneData;
}

SceneData stanford_dragon_scene() {

    Sphere* spheres = new Sphere[4];

    Sphere ground_sphere;
    ground_sphere.position = glm::vec3(0.0f, -999.95f, 5.0f);
    ground_sphere.radius = 1000.0f;
    ground_sphere.material.color = glm::vec3(0.807f, 0.2588f, 0.2588f); // Red color
    ground_sphere.material.emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
    ground_sphere.material.emissionStrength = 0.0f;

    Sphere light_sphere;
    light_sphere.position = glm::vec3(0.0f, 700.0f, 0.0f);
    light_sphere.radius = 500.0f;
    light_sphere.material.color = glm::vec3(0.0f, 0.0f, 0.0f);
    light_sphere.material.emissionColor = glm::vec3(1.0f, 0.9f, 0.8f);
    light_sphere.material.emissionStrength = 3.0f;

    spheres[1] = ground_sphere;
    spheres[0] = light_sphere;

    SceneData sceneData;
    sceneData.sceneObjects = spheres;
    sceneData.numberOfObjects = 2;
    sceneData.size = sceneData.numberOfObjects * sizeof(Sphere);

    return sceneData;
}

SceneData stanford_bunny_scene() {

    Sphere* spheres = new Sphere[4];

    Sphere ground_sphere;
    ground_sphere.position = glm::vec3(0.0f, -999.95f, 5.0f);
    ground_sphere.radius = 1000.0f;
    ground_sphere.material.color = glm::vec3(0.807f, 0.2588f, 0.2588f); // Red color
    ground_sphere.material.emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
    ground_sphere.material.emissionStrength = 0.0f;

    Sphere light_sphere;
    light_sphere.position = glm::vec3(0.0f, 700.0f, 500.0f);
    light_sphere.radius = 500.0f;
    light_sphere.material.color = glm::vec3(0.0f, 0.0f, 0.0f);
    light_sphere.material.emissionColor = glm::vec3(1.0f, 0.9f, 0.8f);
    light_sphere.material.emissionStrength = 3.5f;

    spheres[0] = ground_sphere;
    spheres[1] = light_sphere;

    SceneData sceneData;
    sceneData.sceneObjects = spheres;
    sceneData.numberOfObjects = 2;
    sceneData.size = sceneData.numberOfObjects * sizeof(Sphere);

    return sceneData;
}


SceneData sponza_lights_scene() {

    Sphere* spheres = new Sphere[4];

    Sphere light_sphere;
    light_sphere.position = glm::vec3(1000.0f, 50.0f, 0.0f);
    light_sphere.radius = 30.0f;
    light_sphere.material.color = glm::vec3(0.0f, 0.0f, 0.0f);
    light_sphere.material.emissionColor = glm::vec3(1.0f, 1.0f, 1.0f);
    light_sphere.material.emissionStrength = 8.0f;

    spheres[0] = light_sphere;

    SceneData sceneData;
    sceneData.sceneObjects = spheres;
    sceneData.numberOfObjects = 1;
    sceneData.size = sceneData.numberOfObjects * sizeof(Sphere);

    return sceneData;
}