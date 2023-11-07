#include "globals.h"
#include "GraphicsUtils.h"
#include "Application.h"
#include "Renderer.h"
#include "Framebuffer.h"
#include "InputOutput.h"
#include "Camera.h"

#include "Prompts.h"

//#include <png.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

//using namespace System;
//using namespace System::IO;

namespace GraphicsUtils
{
	void takeScreenshot(bool promptForName)
	{
		std::string fname;
		std::string ext = "png";

		if (promptForName)
		{
			fname = Util::SaveFile({ "png file (.png)", "*.png", "All files (*)", "*" }, IO::assetPath(""));
		}
		else
		{
			fname = fmt::format("{0}/{1}", IO::assetPath(""), IO::getCurrentTimeFilename("", ext));
		}

		takeScreenshotPNG(fname);
	}

	void saveGBuffer(Framebuffer* gbuffer, GLuint attachment, const std::string& name)
	{
		if (!gbuffer) return;

		std::string fname;
		std::string ext = "png";

		if (not name.empty())
		{
			fname = IO::assetPath("") + "/" + name + "." + ext;
		}
		else
		{
			fname = Util::SaveFile({ "png file (.png)", "*.png", "All files (*)", "*" }, IO::assetPath(""));
		}

		log("Saving GBuffer to {0}\n", fname);

		int width = gbuffer->width;
		int height = gbuffer->height;

		/*
		FILE * fp = fopen(fname.c_str(), "wb");

		if (!fp)
		{
			printf("Error creating file handle to %s!\n", fname.c_str());
			return;
		}

		

		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png)
		{
			printf("Error creating png!\n");
			return;
		}

		png_infop info = png_create_info_struct(png);
		if (!info)
		{
			printf("Error creating png info!\n");
			png_destroy_write_struct(&png, &info);
			return;
		}

		png_init_io(png, fp);

		png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
		if (!palette)
		{
			fclose(fp);
			png_destroy_write_struct(&png, &info);
			return;
		}

		png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);

		png_write_info(png, info);

		png_set_packing(png);

		//png_set_filler(png, 0, PNG_FILLER_AFTER);
		
		*/

		GLubyte * mem = new GLubyte[width * height * 4];

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer->framebuffer);
		glReadBuffer(gbuffer->buffers[attachment]);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, mem);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		stbi_write_png(fname.c_str(), width, height, 4, mem, 4);

		delete[] mem;

		/*
		// Allocate row pointers
		png_bytep * row_pointers = (png_bytep*)malloc(sizeof(png_bytep)* height);

		if (!row_pointers)
		{
			fclose(fp);
			png_destroy_write_struct(&png, &info);
			return;
		}

		for (int y = 0; y < height; y++)
		{
			//row_pointers[y] = (png_byte*)malloc(sizeof(png_byte)* 3 * width);
			row_pointers[y] = (png_bytep)(mem + (height - y - 1) * width * 4);
		}
		*/

		// Populate row pointers

		//for (int i = 0; i<width; i++)
		//{
		//	for (int j = 0; j<height; j++)
		//	{
		//		int x = j;
		//		int y = (height - 1) - i;

		//		int index = (i * width + j) * 3;

		//		png_byte r = mem[index + 0];
		//		png_byte g = mem[index + 1];
		//		png_byte b = mem[index + 2];

		//		row_pointers[y][x] = r;
		//		row_pointers[y][x + 1] = g;
		//		row_pointers[y][x + 2] = b;
		//	}
		//}

		/*
		png_write_image(png, row_pointers);
		png_write_end(png, NULL);
		*/

		/*for (int y = 0; y < height; y++)
		{
		free(row_pointers[y]);
		}*/

		/*
		free(row_pointers);

		png_free(png, palette);

		fclose(fp);

		png_destroy_write_struct(&png, &info);
		*/
	}

	void takeScreenshotPNG(const std::string & fname)
	{
		log("Saving screenshot to {0}\n", fname);

		Camera * camera = &Application::get().renderer->camera;
		int width = camera->viewport[2];
		int height = camera->viewport[3];

		/*
		FILE * fp = fopen(fname.c_str(), "wb");

		if (!fp)
		{
			printf("Error creating file handle to %s!\n", fname.c_str());
			return;
		}

		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png)
		{
			printf("Error creating png!\n");
			return;
		}

		png_infop info = png_create_info_struct(png);
		if (!info)
		{
			printf("Error creating png info!\n");
			png_destroy_write_struct(&png, &info);
			return;
		}

		png_init_io(png, fp);

		png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
		if (!palette)
		{
			fclose(fp);
			png_destroy_write_struct(&png, &info);
			return;
		}

		png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);

		png_write_info(png, info);

		png_set_packing(png);

		//png_set_filler(png, 0, PNG_FILLER_AFTER);

		*/

		GLubyte * mem = new GLubyte[width * height * 3];

		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, mem);

		stbi_write_png(fname.c_str(), width, height, 4, mem, 4);

		/*
		// Allocate row pointers
		png_bytep * row_pointers = (png_bytep*)malloc(sizeof(png_bytep)* height);

		if (!row_pointers)
		{
			fclose(fp);
			png_destroy_write_struct(&png, &info);
			return;
		}

		for (int y = 0; y < height; y++)
		{
			//row_pointers[y] = (png_byte*)malloc(sizeof(png_byte)* 3 * width);
			row_pointers[y] = (png_bytep)(mem + (height - y - 1) * width * 3);
		}

		*/

		// Populate row pointers

		//for (int i = 0; i<width; i++)
		//{
		//	for (int j = 0; j<height; j++)
		//	{
		//		int x = j;
		//		int y = (height - 1) - i;

		//		int index = (i * width + j) * 3;

		//		png_byte r = mem[index + 0];
		//		png_byte g = mem[index + 1];
		//		png_byte b = mem[index + 2];

		//		row_pointers[y][x] = r;
		//		row_pointers[y][x + 1] = g;
		//		row_pointers[y][x + 2] = b;
		//	}
		//}

		/*
		png_write_image(png, row_pointers);
		png_write_end(png, NULL);

		//for (int y = 0; y < height; y++)
		//{
		//	free(row_pointers[y]);
		//}

		free(row_pointers);

		png_free(png, palette);

		fclose(fp);

		png_destroy_write_struct(&png, &info);
		*/
	}

	void renderTri()
	{
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0, 0);
		glColor3f(1, 0, 0);
		glVertex3f(-1, -1, 0);

		glColor3f(0, 1, 0);
		glTexCoord2f(1, 0);
		glVertex3f(1, -1, 0);

		glColor3f(0, 0, 1);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0.67f, 0);
		glEnd();
	}

	void renderQuad(bool wind)
	{
		glBegin(GL_QUADS);

		// CLOCKWISE ORDER?! STUPID TEXTURES DSASGFKGHJKHGVCHGDRYTFGXSRSWDGFAS
		if (wind)
		{
			glTexCoord2f(0, 1);
			glVertex3f(-1, 1, 0);

			glTexCoord2f(1, 1);
			glVertex3f(1, 1, 0);

			glTexCoord2f(1, 0);
			glVertex3f(1, -1, 0);

			glTexCoord2f(0, 0);
			glVertex3f(-1, -1, 0);
		}

		else
		{
			glTexCoord2f(0, 0);
			glVertex3f(-1, -1, 0);

			glTexCoord2f(1, 0);
			glVertex3f(1, -1, 0);

			glTexCoord2f(1, 1);
			glVertex3f(1, 1, 0);

			glTexCoord2f(0, 1);
			glVertex3f(-1, 1, 0);
		}
		glEnd();
	}
	
	GLenum checkGLError(const char * source)
	{
		GLenum err = glGetError();

		if (err != GL_NO_ERROR)
		{
			const char * errStr = (const char *)glewGetErrorString(err);
			log("GL Error! Code {0:x}:{1}\n{2}", err, errStr ? errStr : "",
				source ? fmt::format("Called from {0}\n", source) : "");
		}

		return err;
	}
}