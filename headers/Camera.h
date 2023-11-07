#pragma once

#include "globals.h"

struct Camera {
	vec3 cameraPosition = vec3(0, 0, 5);
	vec3 cameraLookat = vec3(0, 0, 0);
	vec3 cameraUp = vec3(0, 1, 0);
	bool cameraOrtho = false;
	float cameraFOVY = 60.0f;
	vec2 nearFar = vec2(0.01, 1000);

	bool useArcBall = true;
	float hRot = 0.0f;
	float vRot = 0.0f;
	float distance = 5.0f;
	float rotSpeed = 0.01f;
	float panSpeed = 0.1f;
	vec2 distRange = vec2(0.01f, 1000.f);

	mat4 view;
	mat4 projection;

	vec4 viewport;

	void update(float width, float height);

	void renderUI();
};