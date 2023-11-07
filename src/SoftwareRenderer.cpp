#include "Renderer.h"

#include "Application.h"
#include "Assignment.h"
#include "Framebuffer.h"
#include "Input.h"
#include "InputOutput.h"
#include "Texture.h"
#include "Prompts.h"
#include "Properties.h"
#include "Tool.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

bool SoftwareRenderer::readyToRock = false;
SoftwareRenderer* SoftwareRenderer::instance = nullptr;

struct SoftwareRenderer::Settings : public IProperties
{
	BoolProp runEveryFrame = BoolProp("runEveryFrame", true);
	IntProp swapInterval = IntProp("swapInterval", 1);

	Settings() : IProperties("Settings")
	{
		runEveryFrame.AddTo(this);
		swapInterval.AddTo(this).AddChangeEvent([](IProperties* p, const int& oldval, const int& newval) {
			glfwSwapInterval(newval);
		});
	}
};

void SoftwareRenderer::initImGui() {
	auto window = Application::get().window;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = shaderVersion.c_str();
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void SoftwareRenderer::init()
{
	instance = this;

	lockResolutionToWindow = false;
	readyToRock = true;
	name = "Software Renderer";
	Application& application = Application::get();
	glfwGetFramebufferSize(application.window, &resolution.x, &resolution.y);
	glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));

	gbuffer = std::make_shared<Framebuffer>(256, 128, 2, std::vector<GBufferMode>{ GBufferMode::Rendered, GBufferMode::Rendered });

	settings = std::make_shared<SoftwareRenderer::Settings>();

	glfwSwapInterval(settings->swapInterval);

	glfwSetFramebufferSizeCallback(application.window, [](GLFWwindow* glfw, int newWidth, int newHeight) {
		if (newWidth <= 0 || newHeight <= 0) return;

		if (auto sr = SoftwareRenderer::instance) {
			sr->resolution = ivec2(newWidth, newHeight);
			sr->viewport = ivec4(0, 0, newWidth, newHeight);
			if (sr->lockResolutionToWindow) {
				//ogl->gbuffer = std::make_shared<Framebuffer> (newWidth, newHeight);
				sr->gbuffer = std::make_shared<Framebuffer>(newWidth, newHeight, 2, std::vector<GBufferMode>{ GBufferMode::Rendered, GBufferMode::Rendered });
			}
			sr->refresh = true;
		}
		});

	Input::get().init();

	initImGui();

	initialized = true;
}

void SoftwareRenderer::ImGuiNewFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

}

double SoftwareRenderer::Render(Application& application)
{
	_time nowish = _clock::now();

	if (!initialized)
	{
		init();
	}

	if (settings->runEveryFrame || refresh) {
		gbuffer->bind(GL_DRAW_FRAMEBUFFER);
		gbuffer->setAllBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//drawOnCPU();

		// Only render assignments that don't use OpenGL
		for (auto& assignment : application.assignments) {
			if (!assignment->useOpenGL) {
				assignment->render(gbuffer->textures[0]);
			}
		}

		if (!settings->runEveryFrame) {
			refresh = false;
		}

		gbuffer->unbind(GL_DRAW_FRAMEBUFFER);
		gbuffer->bind(GL_READ_FRAMEBUFFER);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glBlitFramebuffer(0, 0, gbuffer->width, gbuffer->height,
			0, 0, resolution.x, resolution.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		gbuffer->unbind(GL_READ_FRAMEBUFFER);

		glFinish();


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}



	_elapsed renderDiff = _clock::now() - nowish;
	return renderDiff.count();
}

void SoftwareRenderer::endRender() {
	presentUI();

	glfwSwapBuffers(Application::get().window);
}

void SoftwareRenderer::renderUI()
{
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		ImGui::Indent(16.0f);

		if (ImGui::Button("Force refresh")) {
			refresh = true;
		}

		ImGui::Checkbox("Lock resolution to window size", &lockResolutionToWindow);

		if (gbuffer) gbuffer->renderUI("Framebuffer");

		ImGui::ColorEdit4("Clear color", glm::value_ptr(clearColor));

		//camera.renderUI();

		if (ImGui::CollapsingHeader("Settings"))
		{
			settings->renderUI();
		}

		ImGui::Unindent(16.0f);
	}
}

void SoftwareRenderer::presentUI() {
	ImGui::Render();

	if (!showImGui) return;

	int display_w, display_h;
	glfwGetFramebufferSize(Application::get().window, &display_w, &display_h);

	glViewport(0, 0, display_w, display_h);
	ImVec4 clear_color = { 0, 0, 0, 0 };
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

SoftwareRenderer::~SoftwareRenderer() {
	readyToRock = false;
}

double SoftwareRenderer::drawOnCPU() {

	auto tex = gbuffer->textures[0];
	auto mem = tex->memory;
	auto value = mem->value;
	auto pixels = (float*)mem->value;

	double nowish = getTime();

	size_t numPixels = (size_t)tex->resolution.x * tex->resolution.y;

	for (size_t y = 0; y < tex->resolution.y; y++) {
		for (size_t x = 0; x < tex->resolution.x; x++) {

			size_t offset = ((y * tex->resolution.x) + x) * mem->stride;

			auto pixel = pixels + offset;

			if (x % 2 == y % 2) {
				pixel[0] = 1.0f;
				pixel[1] = 1.0f;
				pixel[2] = 1.0f;
			}
			
			else {
				pixel[0] = clearColor.x;
				pixel[1] = clearColor.y;
				pixel[2] = clearColor.z;
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->resolution.x, tex->resolution.y, 0, GL_RGBA, GL_FLOAT, (const void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	return getTime() - nowish;
}