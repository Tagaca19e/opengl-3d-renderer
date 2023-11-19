#include "Camera.h"

#include "Input.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"

// Updates orientation for cubemap environment.
void Camera::updateOrientation() {
  auto &input = Input::get();

  static vec2 lastMousePos, mousePos;
  if (input.current.mouseStates[GLFW_MOUSE_BUTTON_LEFT]) {
    mousePos = input.current.mousePos;
    vec2 deltaMouse = mousePos - lastMousePos;

    // Adjust sensitivity to make the camera less sensitive.
    const float sensitivity = 0.05f;  // Adjust this value as needed.
    const float dampingFactor = 0.1f; // Damping factor, adjust as needed.

    // Apply damping to reduce the effect of mouse movement gradually.
    static vec2 mouseVelocity = vec2(0.0f);
    mouseVelocity = (mouseVelocity + deltaMouse) * dampingFactor;
    deltaMouse = mouseVelocity;

    // Rotate the camera based on mouse movement.
    hRot -= deltaMouse.x * sensitivity; // Inverted here.
    vRot -= deltaMouse.y * sensitivity;

    // Clamp vertical rotation to avoid flipping the camera.
    vRot = glm::clamp(vRot, -glm::half_pi<float>(), glm::half_pi<float>());

    // Compute camera orientation based on hRot and vRot.
    cameraOrientation.x = glm::cos(vRot) * glm::sin(hRot);
    cameraOrientation.y = glm::sin(vRot);
    cameraOrientation.z = glm::cos(vRot) * glm::cos(hRot);

    lastMousePos = mousePos;
  }
}

void Camera::update(float width, float height) {
  static vec2 lastMousePos, mousePos;

	auto& input = Input::get();

	// Use cubemap camera to be able to move around inside the cubemap.
	if (useCubemapCamera) {
    updateOrientation();
	} else {
		if (input.current.keyStates[GLFW_KEY_F]) {
      cameraLookat = vec3(0);
    }

    if (useArcBall) {
      lastMousePos = mousePos;
      mousePos = input.current.mousePos;
      vec2 deltaMouse = mousePos - lastMousePos;

      mat4 vRotation = glm::rotate(vRot, vec3(0, 1, 0));
      mat4 hRotation = glm::rotate(hRot, vec3(1, 0, 0));

      mat4 totalRot = vRotation * hRotation;

      if (input.current.keyStates[GLFW_KEY_LEFT_ALT]) {
        if (input.current.mouseStates[GLFW_MOUSE_BUTTON_LEFT]) {
          hRot -= deltaMouse.y * rotSpeed;
          vRot -= deltaMouse.x * rotSpeed;

          if (hRot < 0) {
            hRot += glm::two_pi<float>();
          }
          if (vRot < 0) {
            vRot += glm::two_pi<float>();
          }

          if (hRot >= glm::two_pi<float>()) {
            hRot -= glm::two_pi<float>();
          }

          if (vRot >= glm::two_pi<float>()) {
            vRot -= glm::two_pi<float>();
          }
        } else if (input.current.mouseStates[GLFW_MOUSE_BUTTON_RIGHT]) {
          distance =
              glm::clamp(distance - deltaMouse.y, distRange.x, distRange.y);
        } else if (input.current.mouseStates[GLFW_MOUSE_BUTTON_MIDDLE] &&
                   glm::length(deltaMouse) > 0.f) {
          vec3 offset = totalRot * vec4(-deltaMouse.x, deltaMouse.y, 0, 1);
          vec3 panAmount = glm::normalize(offset) * panSpeed;

          cameraPosition += panAmount;
          cameraLookat += panAmount;
        }
      }

      vec4 cameraOffset = vec4(0, 0, distance, 1);

      cameraPosition = vec3(totalRot * cameraOffset);
    }

    view = glm::lookAt(cameraPosition, cameraLookat, cameraUp);

    float fovy = glm::radians(cameraFOVY);
    float aspectRatio = width / height;
    projection = glm::perspective(fovy, aspectRatio, nearFar.x, nearFar.y);

    if (cameraOrtho) {
      float r_fov = aspectRatio / fovy;
      float r_vfov = fovy;

      float d = glm::length(cameraLookat - cameraPosition);

      // Distances from the center to the left and right vertical planes
      float x = tan(r_fov * 0.5f) * d;
      float y = tan(r_vfov * 0.5f) * d;

      projection = glm::ortho(-x, x, -y, y, nearFar.y, nearFar.x);
    }
  }
}

void Camera::renderUI() {
  if (ImGui::CollapsingHeader("Camera")) {
    ImGui::SliderFloat3("Position", glm::value_ptr(cameraPosition), -100, 100);
    ImGui::InputFloat3("Position manual", glm::value_ptr(cameraPosition));
    ImGui::SliderFloat3("Lookat", glm::value_ptr(cameraLookat), -100, 100);
    ImGui::InputFloat3("Lookat manual", glm::value_ptr(cameraLookat));
    ImGui::InputFloat3("Up", glm::value_ptr(cameraUp));
    ImGui::SliderFloat("FOVY", &cameraFOVY, 0, 360);
    ImGui::InputFloat2("Near-far", glm::value_ptr(nearFar));

    if (ImGui::Button(cameraOrtho ? "Orthographic" : "Perspective")) {
      cameraOrtho = !cameraOrtho;
    }

    ImGui::Checkbox("Use cubemap camera", &useCubemapCamera);
    ImGui::Checkbox("Use arcball", &useArcBall);
    ImGui::InputFloat("Rotation speed", &rotSpeed);
    ImGui::SliderFloat("Horizontal rotation", &hRot, 0, glm::two_pi<float>());
    ImGui::SliderFloat("Vertical rotation", &vRot, 0, glm::two_pi<float>());
    ImGui::SliderFloat("Distance", &distance, distRange.x, distRange.y);
    ImGui::InputFloat2("Distance range", glm::value_ptr(distRange));
  }
}