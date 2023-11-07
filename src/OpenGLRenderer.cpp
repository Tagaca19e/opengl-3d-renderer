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

bool OpenGLRenderer::readyToRock = false;
OpenGLRenderer* OpenGLRenderer::instance = nullptr;

struct OpenGLRenderer::Settings : public IProperties
{
	BoolProp runEveryFrame = BoolProp("runEveryFrame", true);
	IntProp swapInterval = IntProp("swapInterval", 1);
	BoolProp glEnableCulling = BoolProp("glEnableCulling", true);
	Property<GLenum> glFrontFace = Property<GLenum>("glFrontFace", GL_CCW)
		.SetRange({ { "GL_CCW", GL_CCW }, { "GL_CW", GL_CW} });
	Property<GLenum> glCullFace = Property<GLenum>("glCullFace", GL_BACK)
		.SetRange({ { "GL_BACK", GL_BACK }, { "GL_FRONT", GL_FRONT }, { "GL_FRONT_AND_BACK", GL_FRONT_AND_BACK } })
		;
	

	Settings() : IProperties("Settings")
	{
		runEveryFrame.AddTo(this);
		swapInterval.AddTo(this).AddChangeEvent([](IProperties* p, const int& oldval, const int& newval) {
			glfwSwapInterval(newval);
			});
		glEnableCulling.AddTo(this);
		glFrontFace.AddTo(this);
		glCullFace.AddTo(this);
	}
};

void OpenGLRenderer::initImGui() {
	auto window = Application::get().window;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = shaderVersion.c_str();
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void OpenGLRenderer::init()
{
	instance = this;
	lockResolutionToWindow = true;
	readyToRock = true;
	name = "OpenGL Renderer";
	Application& application = Application::get();
	glfwGetFramebufferSize(application.window, &resolution.x, &resolution.y);
	glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));

	gbuffer = std::make_shared<Framebuffer>(resolution.x, resolution.y, 2, std::vector<GBufferMode>{ GBufferMode::Rendered, GBufferMode::Rendered });

	settings = std::make_shared<OpenGLRenderer::Settings>();

	glfwSwapInterval(settings->swapInterval);

	glfwSetFramebufferSizeCallback(application.window, [](GLFWwindow* glfw, int newWidth, int newHeight) {
		if (newWidth <= 0 || newHeight <= 0) return;

		if (auto sr = OpenGLRenderer::instance) {
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

void OpenGLRenderer::ImGuiNewFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

}

double OpenGLRenderer::Render(Application& application)
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

		if (settings->glEnableCulling) {
			glEnable(GL_CULL_FACE);
			glCullFace(settings->glCullFace);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		glFrontFace(settings->glFrontFace);

		// Only render assignments that use OpenGL
		for (auto& assignment : application.assignments) {
			if (assignment->useOpenGL) {
				assignment->render(gbuffer);
			}
			assignment->render(gbuffer->textures[0]);
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

void OpenGLRenderer::endRender() {
	presentUI();

	glfwSwapBuffers(Application::get().window);
}

void OpenGLRenderer::renderUI()
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

		if (ImGui::CollapsingHeader("Textures")) {
			if (ImGui::Button("Load new texture")) {
				auto texFile = Util::LoadFile({ "Common image files (.jpg .png .bmp .gif)", "*.jpg *.png *.bmp *.gif", "All files (*)", "*" }, ".");
				if (texFile != "") {
					if (s_ptr<Texture> t = std::make_shared<Texture>(texFile)) {
						textures[t->id] = t;
					}
				}
			}

			for (auto& t : textures) {
				t.second->renderUI();
			}
		}

		if (ImGui::CollapsingHeader("Settings"))
		{
			settings->renderUI();
		}

		ImGui::Unindent(16.0f);
	}
}

void OpenGLRenderer::presentUI() {
	ImGui::Render();

	if (!showImGui) return;

	int display_w, display_h;
	glfwGetFramebufferSize(Application::get().window, &display_w, &display_h);

	glViewport(0, 0, display_w, display_h);
	ImVec4 clear_color = { 0, 0, 0, 0 };
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

OpenGLRenderer::~OpenGLRenderer() {
	readyToRock = false;
}