#pragma once

#include "globals.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdlib>

class Texture;

class Framebuffer
{
	public:

		//static std::vector<GBufferMode> MeshBuffers;
		//static std::vector<GBufferMode> WindowBuffers;
		//static std::vector<GBufferMode> EdgeBuffers;
		//static std::vector<GBufferMode> BoneUnwrapBuffers;

		// Mesh that this GBuffer belongs to
		//Mesh * mesh = nullptr;

		union 
		{
			struct 
			{
				// Width and height hurr
				int width, height;
			};
			ivec2 resolution = ivec2(0);
		};
	private:
		ivec2 resizeResolution;
	public:

		// User-selected render mode
		GBufferMode renderMode = GBufferMode::Rendered;

		// The index of which framebuffer attachment to display (assumes the 0-index element is for GL_COLOR_ATTACHMENT0)
		int attachmentToDisplay = 0;

		// Handles to buffers
		GLuint framebuffer = 0;
		GLuint depthbuffer = 0;
		GLuint stencilbuffer = 0;

		// Number of textures that this framebuffer manages. 
		GLsizei numTextures = 0;
		int depthIndex = -1;
		int stencilIndex = -1;

		// 8 for color attachments, 1 for depth buffer, 1 for stencil buffer.
		// This may become invalidated if we want to deal with cubemaps and so on, but we'll see...
		static const int maxTextures = 10;

		//// GBuffer channels
		// Handles to textures
		//GLuint textures[maxTextures];

		// Buffer enum array. Index corresponds with textures and memory arrays and value is
		// GL_COLOR_ATTACHMENT0 + n, depth, or stencil
		GLenum buffers[maxTextures];

		std::vector<GBufferMode> bufferModes;

		s_ptr_vector<Texture> textures;

		// Constructor
		Framebuffer(int w, int h, int numColors = 1, std::vector<GBufferMode> _bufModes = {}, bool useDepth = true, bool useStencil = false);
		~Framebuffer();

		// Functions
		void bind(GLenum target = GL_DRAW_FRAMEBUFFER, bool setViewport = true);
		void unbind(GLenum target = GL_DRAW_FRAMEBUFFER);
		void setAllBuffers();
		// Requires the amount of target buffers to be known at runtime! be warned...
		void setBuffers(const std::vector<GBufferMode> & modes);

		void clearBuffer(GBufferMode mode);
		void clearBufferfv(GLenum buffer, GBufferMode mode, float clearVal = 0.0f);
		void clearTexture(GBufferMode mode, const vec4& clearColor = vec4(0.0f));
		void clearDepth(int slot);

		GLuint getRenderModeTexture();
		GLuint getRenderModeFormat();

		void saveTextureToFile(std::string filename);

		void resize(ivec2 newRes);

		void renderUI(const std::string & menuTitle = "");

	private:
		void destroyTextures();
		void initTextures();
		void initTexture(int index, GBufferMode indexMode);

		std::map<s_ptr<Texture>, bool> uiVisibleTextures;
};