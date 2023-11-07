#pragma once

#include "globals.h"
#include "imgui.h"

class Texture;
class Framebuffer;

class Assignment {
	public:
		bool initialized = false;
		bool useOpenGL = false;
		std::string name;

		Assignment() {}
		Assignment(const std::string& _name, bool openGL = false) : name(_name), useOpenGL(openGL) { }
		virtual ~Assignment() { }

		virtual void init() { }
		
		virtual void render(s_ptr<Texture> screen) { }
		virtual void render(s_ptr<Framebuffer> framebuffer) { }
		virtual void renderUI() { }
};