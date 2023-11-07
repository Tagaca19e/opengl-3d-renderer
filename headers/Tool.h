#pragma once

#include "Input.h"
#include <stdlib.h>

class Tool
{
	public:
		// Static member to indicate which tool is currently active
		static Tool* activeTool;
		// Function object is an event handler for a tool
		// Returns true if the event was handled, false otherwise
		typedef std::function<bool(Tool*, const InputEvent & ie)> InputAction;

		// Struct is a collection of InputAction objects for an event
		struct InputActions
		{
			InputEvent inputEvent;
			std::vector<InputAction> actions;

			InputActions() { }
			InputActions(const InputEvent & ie, const InputAction & ia) : inputEvent(ie) 
			{ 
				actions.push_back(ia); 
			}
			InputActions(const InputEvent & ie, std::vector<InputAction> & a) : inputEvent(ie), actions(a) 
			{ 
			}

			InputActions & operator=(const InputActions & ia)
			{
				if (this != &ia)
				{
					inputEvent = ia.inputEvent;
					actions = ia.actions;
				}

				return *this;
			}

			InputActions & operator=(const InputAction & ia)
			{
				actions.clear();
				actions.push_back(ia);

				return *this;
			}

			InputActions & operator+=(const InputActions & ias)
			{
				if (this != &ias)
				{
					if (inputEvent == ias.inputEvent)
					{
						actions.insert(actions.end(), ias.actions.begin(), ias.actions.end());
					}
				}
				
				return *this;
			}

			InputActions & operator+=(const InputAction & ia)
			{
				actions.push_back(ia);

				return *this;
			}

			bool operator()(Tool * tool, const InputEvent & ie)
			{
				bool handled = false;
				if (inputEvent == ie)
				{
					for (auto & a : actions)
					{
						if (a(tool, ie))
						{
							handled = true;
							break;
						}
					}
				}
				return handled;
			}
		};

		using EventActions = std::vector<InputActions>;

		// Bread and butter of the Tool class
		EventActions eventActions;

		class ToolAction
		{
			public:
				Tool * tool = nullptr;

				ToolAction(Tool * t = nullptr) : tool(t) { }
				virtual ~ToolAction() { }

				virtual void apply() { }
				virtual void undo() { }
		};

		std::vector<ToolAction*> toolActions;
		std::vector<ToolAction*> undoActions;

		std::string name = "AestusTool";

		bool active = false;

		// The unique identifier of a deform tool
		int ID;

		// Static index of all DeformTools
		static int ToolID;

		bool showUI = false;
		bool uiWindow = false;
		bool clearUIOnReset = true;
		bool initialized = false;

		int_vector uiContainers;

		Tool(const std::string & n = "AestusTool", const EventActions & ea = {});
		virtual ~Tool() { }

		virtual EventActions defaultEvents();

		virtual void reset();

		virtual bool init();
		virtual void update(std::vector<InputEvent> & events);

		virtual void activate();
		virtual void deactivate();

		virtual void render() { }

		virtual void renderUI() { }

		virtual void createAction(s_ptr<ToolAction>, bool apply = false);
		virtual void undo();
		virtual void redo();

		// Overload for focusing camera
		virtual void cameraFocus() { }
};