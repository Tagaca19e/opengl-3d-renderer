#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Framebuffer;

namespace GraphicsUtils
{
	void takeScreenshot(bool promptForName = false);
	void takeScreenshotPNG(const std::string & fname);
	void saveGBuffer(Framebuffer* gbuffer, GLuint attachment, const std::string & name = "");

	void renderTri();

	void renderQuad(bool wind = true);

	GLenum checkGLError(const char * source = nullptr);
}

