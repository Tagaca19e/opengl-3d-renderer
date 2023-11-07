#pragma once

#include "Camera.h"

class Application;
class Texture;

class Renderer
{
	public:
		Camera camera;

		std::string name = "Renderer";

		ivec2 resolution = ivec2(800, 600);

		bool lockResolutionToWindow = true;

		ivec4 viewport;

		vec4 clearColor = vec4(0.f);

		bool initialized = false;

		bool showImGui = true;

		virtual void initImGui() {}

		virtual void init() {}

		virtual void ImGuiNewFrame() {};

		// Returns render time in seconds
		virtual double Render(Application&) = 0;

		virtual void endRender() { }

		// Calls all of the ImGui:: widget code to generate the rendering data
		virtual void renderUI() { }

		// Puts the rendering data on the screen
		virtual void presentUI() {}

		virtual ~Renderer() { }

		static std::string shaderVersion;
};

class SoftwareRenderer : public Renderer {
	public:
		struct Settings;
		s_ptr<Settings> settings;

		s_ptr<Framebuffer> gbuffer;

		bool refresh = true;

		virtual void initImGui();

		virtual void init();

		virtual void ImGuiNewFrame();

		virtual double Render(Application&);

		virtual void endRender();

		double drawOnCPU();

		virtual void renderUI();

		virtual void presentUI();

		virtual ~SoftwareRenderer();

		static bool readyToRock;
		static SoftwareRenderer* instance;
		
};

class OpenGLRenderer : public Renderer {
public:
	struct Settings;
	s_ptr<Settings> settings;

	s_ptr<Framebuffer> gbuffer;

	std::map<unsigned int, s_ptr<Texture>> textures;

	bool refresh = true;

	virtual void initImGui();

	virtual void init();

	virtual void ImGuiNewFrame();

	virtual double Render(Application&);

	virtual void endRender();

	virtual void renderUI();

	virtual void presentUI();

	virtual ~OpenGLRenderer();

	static bool readyToRock;
	static OpenGLRenderer* instance;
};