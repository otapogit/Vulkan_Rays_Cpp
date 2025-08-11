#include <iostream>

#include <GlRenderer.h>
#include <string>
#include <chrono>

int main() {
	GLRenderer r;

	if (!r.init()) {
		std::cerr << "Failed to create GL context\n";
		exit(3);
	}
	else {
		std::cout << "GL context created\n";
	}

	//auto carMeshes = r.defineMesh("Debug/teapot.obj");
	//////auto carMeshes = r.defineMesh("Debug/Car.obj");
	//for (const auto m : carMeshes)
	//	r.addMesh(glm::translate(glm::mat4(1.0f), {0, -1.5f, 0}), glm::vec4(0, 1, 0, 1), m);

	constexpr auto plusZ = glm::vec3{ 0, 0, 1 };
	constexpr auto zero2 = glm::vec2{ 0 };
	auto meshToInspect = r.defineMesh(
		{
			{-2, -2, 0}, {2, -2, 0}, {2, 2, 0}, {-2, 2, 0}
		},
		{
			plusZ, plusZ, plusZ, plusZ
		},
		{
			zero2, zero2, zero2, zero2
		},
		{ 
			0, 1, 2, 0, 2, 3
		}
	);
	r.addMesh(glm::mat4{ 1.0F }, glm::vec4{ 1, 0, 0, 1 }, meshToInspect, true);

    // One light mesh, three instances
	auto lightMesh = r.defineMesh(
		{
			{-0.25f, -0.25f, 0}, {0.25f, -0.25f, 0}, {0.05f, 0.25f, 0}, { -0.05f, 0.25f, 0 }
		},
		{
			{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}
		},
		{
			{0, 0}, {0, 0}, {0, 0}, {0, 0}
		},
		{ 0, 3, 2, 0, 2, 1 }
	);
	r.addLight(
		glm::translate(glm::mat4{ 1.0f }, { 5, 0.5, 5 }) * glm::rotate(glm::mat4{ 1.0f }, glm::pi<float>() / 4, { 0, 0, 1}),
		lightMesh, glm::vec3{ 1, 0, 0 }, 100);
	r.addLight(
		glm::translate(glm::mat4{ 1.0f }, { 5, -0.5, 5 }) * glm::rotate(glm::mat4{ 1.0f }, -glm::pi<float>() / 4, { 0, 0, 1}),
		lightMesh, glm::vec3{ 0, 1, 0 }, 100);
	r.addLight(
		glm::translate(glm::mat4{ 1.0f }, { 6, 0, 5 }) * glm::rotate(glm::mat4{ 1.0f }, -glm::pi<float>() / 4, { 0, 0, 1}),
		lightMesh, glm::vec3{ 0, 0, 1 }, 100);

	// blocking geometry
	auto occludingMesh = r.defineMesh(
		{
			{-2, -2, 0.05}, {0, -2, 0.05}, {0, 2, 0.05}, {-2, 2, 0.05}
		},
		{
			plusZ, plusZ, plusZ, plusZ
		},
		{
			zero2, zero2, zero2, zero2
		},
		{
			0, 1, 2, 0, 2, 3
		}
	);
	r.addMesh(glm::mat4{ 1.0F }, glm::vec4{ 1, 0, 0, 1 }, occludingMesh, false);


		
		//r.setInspCamera(glm::lookAt(glm::vec3{ 2, 0 , 3 }, glm::vec3{ 0.0f, 2, 0 }, glm::vec3{ 0, 1, 0 }), glm::perspective(glm::radians(15.f), 1920.f / 1080, 1.f, 5.f));
	r.setInspCamera(glm::lookAt(glm::vec3{ -5, 0 , 5 }, glm::vec3{ 0.0f}, glm::vec3{ 0, 1, 0 }), glm::perspective(glm::radians(15.f), 1920.f / 1080, 1.f, 15.f));
	r.setOutputResolution(1920, 1080);

	constexpr float cameraAnimRadius = 5.0f;
	constexpr int frames = 10;
	for (auto i = 0; i < frames; i++) {
		r.setViewCamera(
			glm::lookAt(
				glm::vec3{
					cameraAnimRadius * std::cosf(i * glm::pi<float>() * 2 / frames),
					cameraAnimRadius * std::sin(i * glm::pi<float>() * 2 / frames),
					5 },
				 glm::vec3{0.0f}, glm::vec3{0, 1, 0}), glm::perspective(glm::radians(60.f), 1920.f / 1080, 1.f, 10.f));

		auto start = std::chrono::system_clock::now().time_since_epoch();

		if (!r.render()) {
			std::cerr << "Error";
			return -3;
		}

		auto end = std::chrono::system_clock::now().time_since_epoch();

		int64_t us = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

		std::cout << "Frame " << i << "  " << us << " us\n";
		r.saveResultToFile("result" + std::to_string(i) + ".png");
	}

	return 0;
}