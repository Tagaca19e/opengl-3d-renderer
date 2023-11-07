#define FORCE_FORWARD_COMPAT
#if defined(__APPLE__) || defined(FORCE_FORWARD_COMPAT)
#define FORWARD_ONLY ;
#endif
#include "globals.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "implot.h"

#include <GL/glew.h>

#include "GLFW/glfw3.h"
//#include <GL/gl.h>

#include "Application.h"
//#include "Shader.h"
#include "UIHelpers.h"
#include "Renderer.h"

std::string Renderer::shaderVersion = "#version 130";

static void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
	const void* userParam)
{
	if (id == GL_INVALID_ENUM) return;

	log("OpenGL error callback:\n\tSource: {0}\n\tType: {1}\n\tId: {2}\n\tSeverity: {3}\n\tMessage: {4}\n",
		source, type, id, severity, message);
}

double getTime()
{
	return glfwGetTime();
}

void logString(const std::string& s)
{
	Application& application = Application::get();

	if (application.console == nullptr)
	{
		application.console = std::make_shared<ImGuiConsole>();
	}

	if (!application.quit) {
		application.console->AddLog(s);
	}
}

GLFWwindow* createGlfwWindow(int major, int minor) 
{
	GLFWmonitor* primary = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primary);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
#ifdef FORWARD_ONLY
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // 3.2+ only
#endif
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only

#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif


	// This is for making borderless fullscreen. It's nice, but it makes alt-tabbing harder while debugging.
/*	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate)*/;

	GLFWwindow* window = glfwCreateWindow(
		mode->width - 100,
		mode->height - 100,
		"3480 main",
		nullptr,
		//primary, // Use primary instead of nullptr when you want to initialize borderless fullscreen.
		nullptr);

	return window;
}

int main(int argc, char** argv)
{
	if (!glfwInit())
	{
		log("Could not create GLFW window\n");
		exit(1);
	}

	ivec2 openGLVersionsToTest[] = {
		//{3, 0},
		//{3, 1},
		//{3, 2},
		//{3, 3},
		//{4, 0},
		{4, 1},	
		//{4, 2},
		//{4, 3},
		//{4, 4},
		//{4, 5},
		//{4, 6}
	};

	GLFWwindow * window = nullptr;
	for (const auto& version : openGLVersionsToTest) {
		int major = version.x;
		int minor = version.y;
		log("GLFW: creating window with OpenGL context {0}.{1}...", major, minor);
		auto nextWindow = createGlfwWindow(major, minor);
		if (nextWindow) {
			if (window) {
				glfwDestroyWindow(window);
			}
			window = nextWindow;
			//Shader::setVersion(major, minor);
			Renderer::shaderVersion = fmt::format("#version {0}{1}0", major, minor);
			log("SUCCESS!\n");
		}
		else {
			log("FAILED!\n");
			//log("\tUnable to create window with OpenGL context {0}.{1}\n", major, minor);
			//break;
		}
	}

	

	if (!window)
	{
		log("Could not create GLFW window\n");
		exit(1);
	}

    glfwMakeContextCurrent(window);

	// Initialize GLEW for accessing modern OpenGL extension functions
	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
        log("glewInit failed :(\n{0}", (const GLchar*)glewGetErrorString(err));
        exit(EXIT_FAILURE);
	}

    auto version = glGetString(GL_SHADING_LANGUAGE_VERSION);

    log("GLEW initialized. Shading language version: {0}\n", (const char*)version);

    // Set up GL debug callbacks, but filter out some of the noise
#if DEBUG && GL_ARB_debug_output
	if (GLEW_ARB_debug_output) {
		glDebugMessageCallback(gl_error_callback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, 0, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB, GL_DONT_CARE, 0, 0, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER_ARB, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
		glDebugMessageControl(GL_DEBUG_SOURCE_API_ARB, GL_DEBUG_TYPE_PERFORMANCE_ARB, GL_DEBUG_SEVERITY_MEDIUM, 0, 0, GL_FALSE);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	// Simple printer for GLFW errors
	glfwSetErrorCallback([](int error, const char* description)
	{
		log("GLFW error {0}: {1}\n", error, description);
	});

    
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	float SCALE = 2.0f;
	ImFontConfig cfg;
	cfg.SizePixels = 13 * SCALE;
	io.Fonts->AddFontDefault(&cfg);

	// Context info. Feel free to comment out if it becomes noise.
	//fmt::print("Located asset directory at {0}\n", IO::getAssetRoot());

	Application& application = Application::get();
	application.init(window);


	while (!application.quit && !glfwWindowShouldClose(window))
	{
		application.update();
		application.render();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}
