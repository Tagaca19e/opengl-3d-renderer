#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "Application.h"
#include "Framebuffer.h"
#include "imgui.h"
#include "UIHelpers.h"
#include "Renderer.h"

//std::map<GLuint, s_ptr<Texture>> Texture::_registry;

/*
std::map<GLuint, s_ptr<Texture>>& Texture::registry() {
    static std::map<GLuint, s_ptr<Texture>> r;
    return r;
}
*/

/*
s_ptr<Texture> Texture::createTexture(const std::string& filename) {
    if (auto tex = std::make_shared<Texture>(filename)) {
        TextureRegistry::get()()[tex->id] = tex;
        return tex;
    }
    return nullptr;
}
*/

Texture::Texture(const std::string & fname) : filename(fname) {
    //auto& reg = registry();
    stbi_set_flip_vertically_on_load(1);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        resolution = uvec2(width, height);
        numChannels = nrChannels;

        if (numChannels == 3) {
            internalFormat = GL_RGB;
            format = GL_RGB;
        }

        glGenTextures(1, &id);
        glBindTexture(bindTarget, id);

        glTexParameteri(bindTarget, GL_TEXTURE_WRAP_S, wrapS._to_integral());
        glTexParameteri(bindTarget, GL_TEXTURE_WRAP_T, wrapT._to_integral());
        minFilter = TextureFilterMode::LinearMipMap;
        glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, minFilter._to_integral());
        glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, magFilter._to_integral());

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(bindTarget, lod, internalFormat, width, height, border, format, pixelDataType, data);
        glGenerateMipmap(bindTarget);

        stbi_image_free(data);
        data = nullptr;
    }
    else
    {
        log("Failed to load texture from file {0}. Not adding texture to registry - check for memleak\n", filename);
    }

    glBindTexture(bindTarget, 0);
}

Texture::Texture(Framebuffer* fb,  GBufferMode _usage, GLenum _attachment) 
    : framebuffer(fb), usage(_usage), attachment(_attachment) {
    if (framebuffer) {
        resolution = framebuffer->resolution;
    }
    //auto& reg = registry();
    
    glGenTextures(1, &id);
    glBindTexture(bindTarget, id);

    if (usage == +GBufferMode::Depth) {
        wrapS = TextureWrapMode::ClampToBorder;
        wrapT = TextureWrapMode::ClampToBorder;
        minFilter = TextureFilterMode::Nearest;
        magFilter = TextureFilterMode::Nearest;

        vec4 borderColor(1.f);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));
    }

    glTexParameteri(bindTarget, GL_TEXTURE_WRAP_S, wrapS._to_integral());
    glTexParameteri(bindTarget, GL_TEXTURE_WRAP_T, wrapT._to_integral());
    glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, minFilter._to_integral());
    glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, magFilter._to_integral());

    auto texDesc = textureDescriptions[usage];

    glTexImage2D(bindTarget,
        0,
        texDesc.internalFormat,
        framebuffer->resolution.x, framebuffer->resolution.y,
        0,
        texDesc.pixelDataFormat,
        texDesc.pixelDataType,
        nullptr);

    memory = std::make_shared<TextureMemory>(texDesc.pixelDataType, resolution.x, resolution.y, texDesc.stride);
}

Texture::~Texture() {
    if (framebuffer && framebuffer->textures.size() > 0) {
        for (auto it = framebuffer->textures.begin(); it != framebuffer->textures.end();) {
            if ((*it)->id == id) {
                it = framebuffer->textures.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    glDeleteTextures(1, &id);
    id = 0;
    resolution = uvec2();
    numChannels = 0;    

    
}

void Texture::renderUI() {
    ImGui::PushID((const void*)this);

    auto label = fmt::format("Texture {0} {1}", id, filename);

    if (ImGui::CollapsingHeader(label.c_str())) {
        if (ImGui::Button("Delete texture")) {
            auto texID = id;
            /*Application::get().addCommand([texID]() {
                TextureRegistry::removeTexture(texID);
            });*/
        }
        ImGui::Text("Texture ID: %u, Resolution: %u x %u (%u channels)", id, resolution.x, resolution.y, numChannels);
        ImGui::Text("Size in MB: %.4f", (resolution.x * resolution.y * numChannels) / (1024.f * 1024.f));

        renderEnumDropDown<TextureWrapMode>("Wrap S", wrapS);
        renderEnumDropDown<TextureWrapMode>("Wrap T", wrapT);

        renderEnumDropDown<TextureFilterMode>("Min Filter", minFilter);
        renderEnumDropDown<TextureFilterMode>("Mag Filter", magFilter);
        ImGui::Image(reinterpret_cast<ImTextureID>(id), ImVec2(resolution.x, resolution.y));
    }
    

    ImGui::PopID();
}

void Texture::copyToMemory()
{
    if (memory) {

        if (framebuffer) {
            glFinish();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->framebuffer);
            glReadBuffer(attachment);
            glReadPixels(0, 0, resolution.x, resolution.y, GL_RGBA, memory->type, memory->value);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            

        }
        else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(bindTarget, id);

            glGetTexImage(bindTarget, 0, format, pixelDataType, memory->value);

            glBindTexture(bindTarget, 0);
        }
    }
}

vec4 Texture::getColor(ivec2 pos) {
    vec4 result = vec4(0.f);
    if (memory) {
        if (!memory->read((GLvoid*)glm::value_ptr(result), pos.x, pos.y, sizeof(vec4))) {
            log("Unable to read texture {0} at location {1}, {2}\n", id, pos.x, pos.y);
        }
    }

    return result;
}

// Default texture descriptions for each gbuffer channel. Argument order is
// internal format, pixel data format, pixel data type, count

TextureDescription Texture::rgbaTextureDescription =
{
	GL_RGBA32F, GL_RGBA, GL_FLOAT, 4
};

TextureDescription Texture::vec3TextureDescription =
{
	GL_RGB32F, GL_RGB, GL_FLOAT, 3
};

TextureDescription Texture::depthTextureDescription =
{
	GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, 1
};

TextureDescription Texture::shadowTextureDescription =
{
    GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_FLOAT, 1
};

TextureDescription Texture::primitiveDataTextureDescription =
{
	GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 4
};

std::map<GBufferMode, TextureDescription> Texture::textureDescriptions =
{
	{ GBufferMode::Rendered, rgbaTextureDescription },
	{ GBufferMode::Color, rgbaTextureDescription },
    { GBufferMode::Position, vec3TextureDescription },
	{ GBufferMode::Normal, vec3TextureDescription },
    { GBufferMode::PrimitiveData, primitiveDataTextureDescription },
	{ GBufferMode::Depth, depthTextureDescription },
    { GBufferMode::Shadow, shadowTextureDescription },
};