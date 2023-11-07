#pragma once

#include "globals.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Framebuffer;

struct TextureDescription
{
	// Number of color components
	GLint internalFormat = 0;
	GLenum pixelDataFormat = 0;
	GLenum pixelDataType = 0;
	size_t stride = 0;

	TextureDescription() { }

	TextureDescription(GLint _if, GLenum pdf, GLenum pdt, size_t s)
		: internalFormat(_if), pixelDataFormat(pdf), pixelDataType(pdt), stride(s) { }
};

// Internal storage for texture-backed framebuffers
class TextureMemory
{
	public:
		GLvoid* value = nullptr;
		GLuint width = 0;
		GLuint height = 0;
		GLuint stride = 0;
		GLenum type = 0;

		size_t size = 0;
		size_t typeSize = 0;

		TextureMemory() { }
		TextureMemory(GLenum t, GLuint w, GLuint h, GLuint s = 1) : type(t), width(w), height(h), stride(s)
		{
			switch (type)
			{
			case GL_INT:
				typeSize = sizeof(GLint);
				break;
			case GL_FLOAT:
				typeSize = sizeof(GLfloat);
				break;
			default:
				break;
			}

			size = width * height * stride * typeSize;

			if (size > 0)
			{
				value = malloc(size);
			}
		}

		~TextureMemory()
		{
			if (value)
			{
				free(value);
				value = nullptr;
			}
		}

		bool read(GLvoid* result, int x, int y, size_t length)
		{
			if (x < 0 || y < 0 || x >= width || y >= height) return false;

			if (!result || !value) return false;

			// Size of one item
			size_t unitSize = typeSize * stride;

			size_t index = ((y * width) + x) * unitSize;

			GLvoid* pos = (GLvoid*)((GLchar*)value + index);

			memcpy(result, pos, length);

			return true;
		}
};

MAKE_ENUM(TextureWrapMode, GLenum, Repeat = GL_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE, ClampToBorder = GL_CLAMP_TO_BORDER);
MAKE_ENUM(TextureFilterMode, GLenum,  Nearest = GL_NEAREST, Linear = GL_LINEAR, NearestMipMap = GL_LINEAR_MIPMAP_NEAREST,
    LinearMipMap = GL_LINEAR_MIPMAP_LINEAR);

class Texture {
    public:
		// A registry of all textures allocated through this class. Useful for tracking things like memory usage.
		/*
		static std::map<GLuint, s_ptr<Texture>>& registry() {
			return _registry;
		}
		*/

        // The OpenGL texture name
        GLuint id = 0;

        // These are presented in the same order as expected for glTexImage2D
        GLenum bindTarget = GL_TEXTURE_2D;
        GLint lod = 0;
        GLenum internalFormat = GL_RGBA;
        uvec2 resolution;
        GLint border = 0;
        GLenum format = GL_RGBA;
        GLenum pixelDataType = GL_UNSIGNED_BYTE;

        // Number of channels in each pixel. 3 would be RGB, 4 would be RGBA. 
        unsigned int numChannels = 3;

        // Parameters for binding
        TextureWrapMode wrapS = TextureWrapMode::Repeat;
        TextureWrapMode wrapT = TextureWrapMode::Repeat;
        TextureFilterMode minFilter = TextureFilterMode::Linear;
        TextureFilterMode magFilter = TextureFilterMode::Linear;

        // If this is loaded from an image
        std::string filename;
        std::string friendlyName = "texture";

        // If this texture belongs to a framebuffer
		Framebuffer* framebuffer = nullptr;

		// If attached to a framebuffer, the attachment point (GL_COLOR_ATTACHMENT0+n, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT)
		GLenum attachment = 0;

		// If attached to a framebuffer, how it's going to be used
		GBufferMode usage = GBufferMode::Rendered;

		// Memory allocation for this texture (if needed for copying to CPU memory, etc.)
		s_ptr<TextureMemory> memory;

		//static s_ptr<Texture> createTexture(const std::string& filename);

        Texture(const std::string & fname);
		Texture(Framebuffer* fb, GBufferMode _usage, GLenum _attachment);
        ~Texture();

        void renderUI();

		void copyToMemory();

		vec4 getColor(ivec2 pos);

	//private:
		// Standard texture formats and allocations to use for different attachments
		static TextureDescription rgbaTextureDescription;
		static TextureDescription vec3TextureDescription;
		static TextureDescription depthTextureDescription;
		static TextureDescription shadowTextureDescription;
		static TextureDescription primitiveDataTextureDescription;
		
		static std::map<GBufferMode, TextureDescription> textureDescriptions;

		//static std::map<GLuint, s_ptr<Texture>> _registry;
};