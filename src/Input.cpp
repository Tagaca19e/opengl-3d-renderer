#include "Input.h"
#include "Application.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"


bool InputEvent::operator<(const Event &e) const
{
	if (type != e.type)
	{
		return type < e.type;
	}

	if (type == +InputEventType::Move || type == +InputEventType::Scroll)
	{
		return false;
	}

	InputEvent us = *this;
	InputEvent them = e;

	InputEvent::Equalize(us, them);

	if (us.action < them.action || us.mods < them.mods || us.button < them.button)
	{
		return true;
	}

	return false;
}

bool InputEvent::operator==(const InputEvent &e) const
{
	if (type != e.type)
	{
		return false;
	}
	if (type == +InputEventType::Move || type == +InputEventType::Scroll)
	{
		return true;
	}

	InputEvent us = *this;
	InputEvent them = e;

	InputEvent::Equalize(us, them);

	return std::hash<InputEvent>(us) == std::hash<InputEvent>(them);
}

bool InputEvent::Equals(const Event & e) const
{
	return (type == e.type)
		&& (button == e.button)
		&& (action == e.action)
		&& (mods == e.mods);
}

void InputEvent::Equalize(Event & a, Event & b)
{
	if (a.button == -1)
	{
		a.button = b.button;
	}
	else if (b.button == -1)
	{
		b.button = a.button;
	}

	if (a.action == -1)
	{
		a.action = b.action;
	}
	else if (b.action == -1)
	{
		b.action = a.action;
	}

	if (a.mods == -1)
	{
		a.mods = b.mods;
	}
	else if (b.mods == -1)
	{
		b.mods = a.mods;
	}
}

void Input::addEvent(const Event & e)
{
	eventMutex.lock();
	events.push_back(e);
	eventMutex.unlock();
}

void Input::addVREvent(const Event & e)
{
	eventMutex.lock();
	vrMousePos = e.pos;
	vrOverride = true;
	vrEvents.push_back(e);

	if (e.type == +InputEventType::Click)
	{
		vrMouseState[e.button] = e.action;
	}
	eventMutex.unlock();
}

void Input::clearEvents()
{
	eventMutex.lock();
	events.clear();
	eventMutex.unlock();
}

void Input::clear()
{
	clearEvents();
	current.mouseScroll = vec2(0.0f);
}

std::vector<InputEvent> Input::getEvents()
{
	std::vector<Event> _events;
	eventMutex.lock();
	_events = events;
	eventMutex.unlock();

	return _events;

}
void Input::setEvents(const std::vector<Event> & _events)
{
	eventMutex.lock();
	events = _events;
	eventMutex.unlock();
}

void Input::init() {
	// Cursor position
	auto glfwWindow = Application::get().window;

	glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow * glfw, double x, double y) {
		//auto & win = Window::get();
		Input::get().addEvent({ InputEventType::Move, 0, InputEventType::Move, 0, 0, vec2(x, y), vec2(0.0f), _clock::now() });
	});

	// Key callback
	glfwSetKeyCallback(glfwWindow, [](GLFWwindow * glfw, int key, int scancode, int action, int mods)
	{
		//auto & win = Window::get();
		//if (win.hasFocus)
		{
			double x, y, sx, sy;
			glfwGetCursorPos(glfw, &x, &y);

			Input::get().addEvent({ InputEventType::Key, key, action, mods, scancode, vec2(x, y), vec2(0.0f), _clock::now() });

			ImGuiIO& io = ImGui::GetIO();
			/*
			if (action == GLFW_PRESS)
				io.KeysDown[key] = true;
			if (action == GLFW_RELEASE)
				io.KeysDown[key] = false;

			(void)mods; // Modifiers are not reliable across systems
			io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
			*/
		}
	});

	// Char callback?
	/*
	glfwSetCharCallback(glfwWindow, [](GLFWwindow * glfw, unsigned int c)
	{
		ImGuiIO & io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
		{
			io.AddInputCharacter((unsigned short)c);
		}
	});
	*/

	glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow * glfw, int button, int action, int mods)
	{
		//auto & win = Window::get();
		//if (win.hasFocus)
		{
			double x, y;
			glfwGetCursorPos(glfw, &x, &y);

			Input::get().addEvent({ InputEventType::Click, button, action, mods, 0, vec2(x, y), vec2(0.0f), _clock::now() });
		}
	});

	glfwSetScrollCallback(glfwWindow, [](GLFWwindow * glfw, double xoff, double yoff)
	{
		//auto & win = Window::get();
		//if (win.hasFocus)
		{
			double x, y;
			glfwGetCursorPos(glfw, &x, &y);

			vec2 scroll(xoff, yoff);

			Input::get().addEvent({ InputEventType::Scroll, 0, 0, 0, 0, vec2(x, y), scroll, _clock::now() });
			Input::get().current.mouseScroll += scroll;
		}
	});
}

void Input::update()
{
	/*clearEvents();
	eventMutex.lock();*/

	// Retain last input state
	last = current;

	// Update current state
	auto win = Application::get().window;

	for (int i = GLFW_KEY_SPACE; i < GLFW_KEY_LAST; i++)
	{
		current.keyStates[i] = glfwGetKey(win, i);
	}

	// Update from GLFW first
	for (int i = 0; i < NUM_MICE; i++)
	{
		current.mouseStates[i] = glfwGetMouseButton(win, i);
	}

	double xpos, ypos;
	glfwGetCursorPos(win, &xpos, &ypos);
	current.mousePos = vec2(xpos, ypos);

	// And if VR is active, overwrite those values from these
	if (vrOverride)
	{
		current.mousePos = vrMousePos;
		current.mouseScroll = vrMouseScroll;
		for (int i = 0; i < NUM_MICE; i++)
		{
			current.mouseStates[i] |= vrMouseState[i];
		}

		// Add pending events
		if (!vrEvents.empty())
		{
			events.insert(events.end(), vrEvents.begin(), vrEvents.end());
			vrEvents.clear();
		}
	}

	//eventMutex.unlock();
}

void Input::renderUI() {
	ImGui::PushID((const void*)this);

	if (ImGui::CollapsingHeader("Input")) {
		static bool showWindow = false;

		if (!showWindow) {
			if (ImGui::Button("Show input events")) {
				showWindow = true;
			}
		}
		else {
			if (ImGui::Button("Hide input events")) {
				showWindow = false;
			}
		}

		if (showWindow) {
			ImGui::Begin("Input Events", &showWindow);
			for(auto & e : events) {
				ImGui::Text("%s", e.type._to_string());
				switch (e.type) {
					case InputEventType::Click:
					case InputEventType::Key:
						ImGui::Text("button %d, state %d", e.button, e.action);
						break;
					case InputEventType::Move:
						ImGui::Text("X %d, Y %d", (int)e.pos.x, (int)e.pos.y);
						break;
				}
			}
			ImGui::End();
		}
		
	}

	ImGui::PopID();
}