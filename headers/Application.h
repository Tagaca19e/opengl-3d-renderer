#pragma once

#include "globals.h"

class Mover;
class Renderer;
class ImGuiConsole;
class FullscreenFilter;
struct Tool;

struct GLFWwindow;

// Forward declarations
class Assignment;

class Application
{
	public:
		GLFWwindow * window = nullptr;
		ivec2 windowSize;

		bool quit = false;

		double startedAt;
		double lastFrameAt;
		double pausedAt;
		double resumedAt;

		double totalPauseTime = 0.;
		double timeSinceStart = 0.;
		double deltaTime = 0.;
		double updateTime = 0.;
		double renderTime = 0.;
		double targetFPS = 60.;
		double sleepTime = 0.;

		unsigned long frameCounter = 0;

		static Application& get()
		{
			static Application g;
			return g;
		}

		s_ptr<Renderer> renderer;

		std::vector<Command> commands;

		// A list of assignments
		s_ptr_vector<Assignment> assignments;

		// A list of tools
		s_ptr_vector<Tool> tools;

		void init(GLFWwindow * w = nullptr);

		virtual void update();
		virtual void render();
		virtual void renderUI();

		virtual void addCommand(const Command& c) {
			commands.push_back(c);
		}

		template <typename T>
		s_ptr<T> getRenderer() {
			return std::dynamic_pointer_cast<T>(renderer);
		}

		template<typename T>
		s_ptr<T> getTool() {
			for (auto& tool : tools) {
				if (auto tt = std::dynamic_pointer_cast<T>(tool)) {
					return tt;
				}
			}

			return nullptr;
		}

		s_ptr<ImGuiConsole> console;
};
