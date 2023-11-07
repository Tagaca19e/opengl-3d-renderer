#include "Tool.h"
#include "Application.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"


int Tool::ToolID = 0;
Tool* Tool::activeTool = nullptr;

Tool::Tool(const std::string & n, const EventActions & ea)
	: name(n), eventActions(ea), ID(ToolID++)
{
}

Tool::EventActions Tool::defaultEvents() 
{ 
	EventActions ea;

	ea.push_back(Tool::InputActions(InputEvent::KeyEvent(GLFW_KEY_F, GLFW_PRESS),
		[](Tool * tool, const InputEvent & ie)
	{
		if (tool->active)
		{
			tool->cameraFocus();
			return true;
		}

		return false;
	}));

	return ea;
}

void Tool::activate()
{
	if (!active)
	{
		if (activeTool) {
			if (this != activeTool) {
				log("Deactivating {0}, activating {1}\n", activeTool->name, name);
			}

			activeTool->deactivate();
		}
		active = true;
		activeTool = this;
		//Toolset::get().activate(this);
	}
}

void Tool::deactivate()
{
	active = false;
}

void Tool::reset()
{
	for (auto & ta : toolActions)
	{
		delete ta;
	}
	for (auto & ua : undoActions)
	{
		delete ua;
	}

	toolActions.clear();
	undoActions.clear();

	if (clearUIOnReset)
	{
		
	}
	
}

//Tool::EventActions Tool::MergeEventActions(const EventActions & first, const EventActions & second)
//{
//	EventActions ea = first;
//
//	for (auto & m : second)
//	{
//		ea[m.first] += m.second;
//	}
//
//	return ea;
//}


bool Tool::init()
{
	initialized = true;
	return true;
}

void Tool::update(std::vector<InputEvent> & events)
{	
	if (!initialized)
	{
		init();
	}

	for (auto it = events.begin(); it != events.end();)
	{
		auto e = *it;

		// See if event is handled by a widget first
		bool eventHandled = false;

		// We can't check for ALL events based on whether the tool is active because some events
		// activate the tool. But we can settle on not allowing widgets to be used unless the 
		// tool is active. I keep catching invisible widgets when I try to work!
		if (active)
		{
			if (showUI)
			{
				//for (auto & w : widgets)
				//{
				//	if (w->handleEvent(e))
				//	{
				//		eventHandled = true;
				//		break;
				//	}
				//}
			}
		}
		
		bool imguiWantsFocus = ImGui::GetIO().WantCaptureKeyboard
			|| ImGui::GetIO().WantCaptureMouse;

		if (!eventHandled && !imguiWantsFocus)
		{
			for (auto & ea : eventActions)
			{
				if (ea(this, e))
				{
					eventHandled = true;
					break;
				}
			}
		}

		// If the event was handled, we want to flag it for removal
		if (eventHandled)
		{
			it = events.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Tool::createAction(s_ptr<ToolAction> ta, bool apply)
{
	//Toolset::get().createAction(ta, apply);
}

void Tool::undo()
{
	if (toolActions.empty()) return;

	auto ta = toolActions.back();

	toolActions.pop_back();

	ta->undo();

	undoActions.push_back(ta);
}

void Tool::redo()
{
	if (undoActions.empty()) return;

	auto ta = undoActions.back();

	undoActions.pop_back();

	ta->apply();

	toolActions.push_back(ta);
}