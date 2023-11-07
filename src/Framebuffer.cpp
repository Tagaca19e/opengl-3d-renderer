#include "Framebuffer.h"
#include "GraphicsUtils.h"
#include "Texture.h"

#include "imgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <thread>
#include <vector>

Framebuffer::Framebuffer(int w, int h, int numColors, std::vector<GBufferMode> _bufModes, bool useDepth, bool useStencil)
	: width(w), height(h)
{
	resizeResolution = ivec2(w, h);

	GLint maxBuffers, maxColorAttachments;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxBuffers);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);

	if (maxBuffers < GBufferMode::_size_constant && maxColorAttachments < GBufferMode::_size_constant)
	{
		log("Error: max buffers is less than GBuffer's Draw Buffers!\n");
		exit(1);
	}

	//// Create Buffers
	// Framebuffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	numTextures = numColors;

	bufferModes = _bufModes;

	if (bufferModes.empty()) {
		bufferModes.resize(numTextures, GBufferMode::Rendered);
	}

	// Create and bind textures
	//glGenTextures(numTextures, textures);

	if (useDepth && !useStencil) {
		depthIndex = numColors;
	}
	else if (useStencil && !useDepth) {
		stencilIndex = numColors;
	}
	else if (useDepth && useStencil) {
		depthIndex = numColors;
		stencilIndex = depthIndex + 1;
	}
	else {
		log("Framebuffer: no depth or stencil buffer\n");
	}

	initTextures();
}

Framebuffer::~Framebuffer() {
	destroyTextures();

	glDeleteFramebuffers(1, &framebuffer);
	framebuffer = 0;

	/*
	if (depthbuffer != 0) {
		glDeleteRenderbuffers(1, &depthbuffer);
		depthbuffer = 0;
	}
	
	if (stencilbuffer != 0) {
		glDeleteRenderbuffers(1, &stencilbuffer);
		stencilbuffer = 0;
	}
	*/
}

void Framebuffer::destroyTextures() {
	if (!textures.empty()) {

		textures.clear();

		uiVisibleTextures.clear();
	}
}

void Framebuffer::initTextures() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	destroyTextures();

	for(int i = 0; i < numTextures; i++)
	{
		initTexture(i, bufferModes[i]);
	}

	if (depthIndex != -1) {
		if (numTextures == 0) {
			initTexture(depthIndex, GBufferMode::Shadow);
		}
		else {
			//initTexture(depthIndex, GBufferMode::Shadow);
			initTexture(depthIndex, GBufferMode::Depth);
		}
	}
	if (stencilIndex != -1) {
		initTexture(stencilIndex, GBufferMode::Stencil);
	}

	if (numTextures == 0 && depthIndex != -1) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	GLenum fbStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		log("Error creating framebuffers! Error code {0}\n", fbStatus);
		exit(1);
	}

	GraphicsUtils::checkGLError(__FUNCTION__);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

void Framebuffer::initTexture(int index, GBufferMode indexMode)
{
	GLenum attachment = GL_COLOR_ATTACHMENT0 + index;
	if (indexMode == +GBufferMode::Depth || indexMode == +GBufferMode::Shadow) {
		attachment = GL_DEPTH_ATTACHMENT;
	}
	else if (indexMode == +GBufferMode::Stencil) {
		attachment = GL_STENCIL_ATTACHMENT;
	}

	if (auto tex = std::make_shared<Texture>(this, indexMode, attachment)) {
		//TextureRegistry::addTexture(tex);
		textures.push_back(tex);

		//GBufferMode indexMode = GBufferMode::_from_integral(index);

		glBindTexture(GL_TEXTURE_2D, tex->id);
		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex->id, 0);

		// Set buffer index
		if (index < numTextures) {
			buffers[index] = attachment;
		}
		
	}
	else {
		log("Unable to create texture {0} for framebuffer\n", index);
	}

	GraphicsUtils::checkGLError(__FUNCTION__);
}

/*
void Framebuffer::initTexture(int index, GBufferMode indexMode)
{
	glBindTexture(GL_TEXTURE_2D, textures[index]);
	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Set buffer index
	if (indexMode == +GBufferMode::Depth)
	{
		buffers[index] = GL_DEPTH_ATTACHMENT;
	}
	else
	{
		buffers[index] = GL_COLOR_ATTACHMENT0 + index;
	}

	auto textureDesc = textureDescriptions[indexMode];

	glTexImage2D(GL_TEXTURE_2D,
		0,
		textureDesc.internalFormat,
		width, height,
		0,
		textureDesc.pixelDataFormat,
		textureDesc.pixelDataType,
		nullptr);

	memory[index] = new TextureMemory(textureDesc.pixelDataType, width, height, textureDesc.stride);
}
*/

void Framebuffer::bind(GLenum target, bool setViewport) {
	glBindFramebuffer(target, framebuffer);

	if (setViewport) {
		glViewport(0, 0, width, height);
	}
}

void Framebuffer::unbind(GLenum target) {
	glBindFramebuffer(target, 0);
}

void Framebuffer::setAllBuffers()
{
	if (numTextures == 0 && depthIndex != -1) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else {
		glDrawBuffers(numTextures, buffers);
	}
}

void Framebuffer::setBuffers(const std::vector<GBufferMode> & modes)
{
	GLenum modeBuffers[GBufferMode::_size_constant];
	int i = 0;

	for (const GBufferMode & mode : modes)
	{
		modeBuffers[i++] = buffers[mode];
	}

	glDrawBuffers(modes.size(), modeBuffers);
}


void Framebuffer::clearBuffer(GBufferMode mode)
{
	GLenum buffer = buffers[mode];
	
	GLint clearValue = 0;

	GLenum bufs[] = { buffers[mode] };
	glDrawBuffers(1, bufs);
	glClearBufferiv(GL_COLOR, 0, &clearValue);
}

void Framebuffer::clearBufferfv(GLenum buffer, GBufferMode mode, float clearVal)
{
	glClearBufferfv(buffer, 0, &clearVal);
}

void Framebuffer::clearDepth(int slot)
{
	float clearVal = 1.0f;
	clearBufferfv(GL_DEPTH, GBufferMode::Depth, clearVal);

	//GLenum bufs[] = { buffers[GBufferMode::Depth] };
	glClearBufferfv(GL_COLOR, slot, &clearVal);
}

GLuint Framebuffer::getRenderModeTexture()
{
	return textures[renderMode]->id;
}

GLuint Framebuffer::getRenderModeFormat()
{
	return Texture::textureDescriptions[renderMode].internalFormat;
}

void Framebuffer::saveTextureToFile(std::string filename)
{
	FILE * f = fopen(filename.c_str(), "wb");

	unsigned char *img = nullptr;
	int filesize = 54 + 3*width*height;  //w is your image width, h is image height, both int
	if( img )
		free( img );
	img = (unsigned char *)malloc(3*width*height);
	memset(img,0,sizeof(img));

	int x, y;
	float r, g, b;

	int yres = height;


	//GLfloat * mem = (GLfloat *)memory[renderMode]->value;
	//TODO: fix the reference
	GLfloat * mem = (GLfloat *)textures[0]->memory->value;

	for(int i=0; i<width; i++)
	{
		for(int j=0; j<height; j++)
	{
		x=j; 
		y=(yres-1)-i;

		int index = (i * width + j) * 3;

		r = mem[index + 0] * 255;
		g = mem[index + 1] * 255;
		b = mem[index + 2] * 255;

		if (r > 255) r=255;
		if (g > 255) g=255;
		if (b > 255) b=255;
		img[(x+y*width)*3+2] = (unsigned char)(r);
		img[(x+y*width)*3+1] = (unsigned char)(g);
		img[(x+y*width)*3+0] = (unsigned char)(b);
	}
	}

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] = {0,0,0};

	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	bmpinfoheader[ 4] = (unsigned char)(       width    );
	bmpinfoheader[ 5] = (unsigned char)(       width>> 8);
	bmpinfoheader[ 6] = (unsigned char)(       width>>16);
	bmpinfoheader[ 7] = (unsigned char)(       width>>24);
	bmpinfoheader[ 8] = (unsigned char)(       height    );
	bmpinfoheader[ 9] = (unsigned char)(       height>> 8);
	bmpinfoheader[10] = (unsigned char)(       height>>16);
	bmpinfoheader[11] = (unsigned char)(       height>>24);

	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	for(int i = 0; i < height; i++)
	{
		fwrite(img+(width*(height-i-1)*3),3,width,f);
		fwrite(bmppad,1,(4-(width*3)%4)%4,f);
	}

	free(img);
	fclose(f);
}

void Framebuffer::clearTexture(GBufferMode mode, const vec4 & clearColor)
{
	// TODO: 1) see if we still need this, ever. 2) Import fullscreen filters if we do.
	/*auto blankerShader = FullscreenFilters::get().blanker;
	if (!blankerShader)
	{
		sts::log("blanker shader not found\n");
		return;
	}

	auto colorLoc = glGetUniformLocation(blankerShader->program, "clearColor");

	blankerShader->start();

	setBuffers({ mode });
	glUniform4fv(colorLoc, 1, (const GLfloat*)glm::value_ptr(clearColor));
	glDrawElements(GL_TRIANGLES, FullscreenFilters::get().fsQuadIndexVBO->data.size(), GL_UNSIGNED_INT, (const GLvoid*)0);

	blankerShader->stop();*/
}

void Framebuffer::resize(ivec2 newRes) {
	if (newRes.x > 0 && newRes.y > 0 && newRes != resolution) {
		resolution = newRes;

		initTextures();
	}
}

void Framebuffer::renderUI(const std::string& menuTitle)
{
	ImGui::PushID((void*)this);
	auto label = fmt::format("{2}: Framebuffer {0} ({1} textures)", framebuffer, numTextures, menuTitle);
	if (ImGui::CollapsingHeader(label.c_str())) {

		ImGui::InputInt2("Resolution", glm::value_ptr(resizeResolution));

		if (resizeResolution != resolution && ImGui::Button("Resize?")) {
			resize(resizeResolution);
		}

		if (ImGui::BeginMenu(menuTitle.empty() ? "G-Buffer channels" : menuTitle.c_str()))
			{
				//for (auto& gbufferMode : GBufferMode::_values())
				for(auto & tex : textures)
				{
					auto texName = fmt::format("Texture {0}", tex->id);
					if (ImGui::MenuItem(texName.c_str())) {
						uiVisibleTextures[tex] = true;
					}
				}

				ImGui::EndMenu();
			}

			for (auto& oc : uiVisibleTextures)
			{
				if (!oc.second) continue;

				float my_tex_w = (float)width;
				float my_tex_h = (float)height;

				ImGui::SetNextWindowSize(ImVec2{ my_tex_w * 0.5f, my_tex_h * 0.5f }, ImGuiCond_Once);
				auto texName = fmt::format("Texture {0}", oc.first->id);
				ImGui::Begin(texName.c_str(), &oc.second);

				ImGuiIO& io = ImGui::GetIO();

				ImTextureID my_tex_id = reinterpret_cast<ImTextureID>(oc.first->id);

				//ImVec2 pos = ImGui::GetCursorScreenPos();
				auto winSize = ImGui::GetWindowSize();

				ImGui::Text("Texture ID: %u", oc.first->id);
				ImGui::SameLine();
				if (ImGui::Button("Save"))
				{
					GraphicsUtils::saveGBuffer(this, oc.first->attachment, texName);
				}

				ImGui::Image(my_tex_id, ImVec2(winSize.x - 100, winSize.y - 100), { 0, 1 }, { 1, 0 }, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				//if (ImGui::IsItemHovered())
				//{
				//	auto pos = ImGui::GetCursorPos();
				//	ImGui::BeginTooltip();
				//	float region_sz = 32.0f;
				//	float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
				//	float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
				//	float zoom = 4.0f;
				//	ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
				//	ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
				//	ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
				//	ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
				//	ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				//	ImGui::EndTooltip();
				//}

				ImGui::End();
			}
	}

	

	
	
	ImGui::PopID();
}