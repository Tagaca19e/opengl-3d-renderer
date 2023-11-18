#include "Application.h"

#include "Input.h"
#include "Prompts.h"
#include "Renderer.h"

#include "Tool.h"
#include "UIHelpers.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <random>

#include "ImCurveEdit.h"
#include "ImSequencer.h"

#include "Lab02.h"
#include "Lab03.h"
#include "Lab04.h"
#include "Lab05.h"
#include "Lab06.h"
#include "Lab07.h"
#include "Project.h"

void Application::init(GLFWwindow* w)
{
	startedAt = getTime();
	lastFrameAt = startedAt;

	window = w;

	// renderer = std::make_shared<SoftwareRenderer>();
	renderer = std::make_shared<OpenGLRenderer>();
	// OGLRenderer = std::make_shared<OpenGLRenderer>();

	if (!renderer) {
		log("Unable to initialize renderer!\n");
		exit(1);
	}

	renderer->init();


	tools.push_back(std::make_shared<Tool>("CommandTool", Tool::EventActions({
		{
			InputEvent::KeyEvent(GLFW_KEY_V, GLFW_PRESS),
			[&](Tool* tool, const InputEvent& ie)
			{
				renderer->showImGui = !renderer->showImGui;
				return true;
			}
		}
		})));

	/*assignments.push_back(std::make_shared<Lab02>());
	assignments.push_back(std::make_shared<Lab03>());
	assignments.push_back(std::make_shared<Lab04>());
	assignments.push_back(std::make_shared<Lab05>());*/
	//assignments.push_back(std::make_shared<Lab06>());
	//assignments.push_back(std::make_shared<Lab07>());
	assignments.push_back(std::make_shared<Project>());

	log("Ready to rock!\n");
}

void Application::update()
{
	// Gets the time (in seconds) since GLFW was initialized
	double nowish = getTime();

	// Gets the time since the application began updating
	timeSinceStart = nowish - startedAt;

	// TODO: fix our timesteps? https://gafferongames.com/post/fix_your_timestep/
	deltaTime = nowish - lastFrameAt;


	glfwGetWindowSize(window, &windowSize.x, &windowSize.y);
	

	// Handle mouse/keyboard/device input events
	auto & input = Input::get();
	// Clear input values from last frame
	input.clear();
	glfwPollEvents();
	input.update();
	
	// Check for quit event
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		quit = true;
	}

	// Process any pending commands
	if (!commands.empty()) {
		for (auto& c : commands) {
			c();
		}

		commands.clear();
	}

	// Handle tool updates and possibly consume input events

	auto events = input.getEvents();

	for(auto & tool : tools) {
		tool->update(events);
	}

	input.setEvents(events);

		
	updateTime = getTime() - nowish;
	lastFrameAt = nowish;
	frameCounter++;
}

void Application::render()
{
	if (!renderer) return; 

	auto t = getTime();

	renderer->ImGuiNewFrame();

	if (renderer->showImGui) {
		renderUI();
	}
	
	
	renderTime = renderer->Render(*this);

	renderer->endRender();

	renderTime = getTime() - t;

	//Profiler::get().add(rendererType._to_string(), Profiled{ renderTime, frameCounter });

	
}


void Application::renderUI()
{
	if (!renderer) return;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				this->quit = true;
			}
			ImGui::EndMenu();
		}
		
		ImGui::EndMainMenuBar();
	}
	

	ImGui::Begin(PROJECT_NAME);

	if (ImGui::CollapsingHeader("Stats"))
	{
		ImGui::Text("Frame counter: %lu, time since start: %.4f s", frameCounter, timeSinceStart);
		ImGui::Text("Update time: %.2f ms", updateTime * 1000);
		ImGui::Text("Render time: %.2f ms", renderTime * 1000);
		ImGui::Text("Delta time: %.2f ms", deltaTime * 1000);
		ImGui::Text("FPS (from delta time): %.2f", 1.0 / deltaTime);

		ImGui::InputDouble("Target FPS", &targetFPS);
		ImGui::InputDouble("Sleep time (ms)", &sleepTime);
	}

	Input::get().renderUI();

	if (ImGui::CollapsingHeader("Tools")) {
		IMDENT;
		for (auto & t : tools)
		{
			ImGui::PushID((const void*)t.get());
			if (ImGui::CollapsingHeader(t->name.c_str()))
			{
				t->renderUI();
			}
			ImGui::PopID();
		}
		IMDONT;
	}
	

	if (renderer) renderer->renderUI();

	//SimpleShapeRenderer::get().renderUI();
	//LineRenderer::get().renderUI();


	//Input::get().renderUI();

	if (!assignments.empty()) {
		if (ImGui::CollapsingHeader("Assignments")) {
			IMDENT;
			for (auto& assignment : assignments) {
				ImGui::PushID((const void*)assignment.get());
				if (ImGui::CollapsingHeader(assignment->name.c_str())) {
					IMDENT;
					assignment->renderUI();
					IMDONT;
				}
				ImGui::PopID();
			}

			IMDONT;
		}
	}

	if (console && renderer)
	{
		static uvec2 screenSize = renderer->resolution;
		static vec2 consoleSize = vec2(screenSize.x * 0.3f, screenSize.y * 0.4f);

		ImGui::SetNextWindowSize({ consoleSize.x, consoleSize.y }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos({ (screenSize.x - consoleSize.x) * 0.5f, screenSize.y - consoleSize.y * 1.1f },
			ImGuiCond_FirstUseEver);
		console->Draw("Console");
	}

	ImGui::End();

	//ECS::get().updateUI();
}