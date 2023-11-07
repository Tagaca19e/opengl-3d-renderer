#pragma once

#include "globals.h"

#include <mutex>

#define NUM_MICE 8

class Input
{
	public:

		static Input & get()
		{
			static Input instance;
			return instance;
		}

		// General state of GLFW input. Keys
		struct State
		{
			int keyStates[1024];
			int mouseStates[NUM_MICE];

			vec2 mousePos;
			vec2 mouseScroll;

			State & operator=(const State & rhs)
			{
				if (this != &rhs)
				{
					memcpy(keyStates, rhs.keyStates, sizeof(keyStates));
					memcpy(mouseStates, rhs.mouseStates, sizeof(mouseStates));

					mousePos = rhs.mousePos;
					mouseScroll = rhs.mouseScroll;
				}
				return *this;
			}
		};

		// Event that modifies state
		struct Event
		{

			InputEventType type;
			// Can be a key or a mouse
			int button;
			// press or release
			int action;
			// Keyboard mods
			int mods;
			// Only relevant if it's a key
			int scancode;
			// Mouse position
			vec2 pos;
			// Mouse scroll
			vec2 scroll;

			// Time of event
			_time when;

			// User data pointer
			void * device = nullptr;

			Event() { }
			Event(const Event & e) { *this = e; }
			Event(InputEventType _type, int _button, int _action, int _mods, int _scancode, vec2 _pos, vec2 _scroll, _time w)
				: type(_type), button(_button), action(_action), mods(_mods), scancode(_scancode), pos(_pos), scroll(_scroll), when(w)
			{

			}

			Event& operator=(const Event &rhs)
			{
				if (this != &rhs)
				{
					type = rhs.type;
					button = rhs.button;
					action = rhs.action;
					mods = rhs.mods;
					scancode = rhs.scancode;
					pos = rhs.pos;
					scroll = rhs.scroll;
					when = rhs.when;
					device = rhs.device;
				}
				return *this;
			}

			bool operator<(const Event &e) const;

			bool operator==(const Event &e) const;

			bool Equals(const Event & e) const;

			// Static constructors for events. A mods value 
			// that's negative indicates that it should be considered
			// equal to another Event's mods value during comparison
			static Event KeyEvent(int button = -1, int action = -1, int mods = -1)
			{
				return Event(InputEventType::Key, button, action, mods, -1, vec2(0.0f), vec2(0.0f), _clock::now());
			}

			static Event ClickEvent(int button = -1, int action = -1, int mods = -1)
			{
				return Event(InputEventType::Click, button, action, mods, -1, vec2(0.0f), vec2(0.0f), _clock::now());
			}

			static Event ScrollEvent()
			{
				return Event(InputEventType::Scroll, -1, -1, -1, -1, vec2(0.0f), vec2(0.0f), _clock::now());
			}

			static Event MoveEvent()
			{
				return Event(InputEventType::Move, -1, -1, -1, -1, vec2(0.0f), vec2(0.0f), _clock::now());
			}

			static Event DeviceEvent(void * device = nullptr)
			{
				Event event(InputEventType::Device, -1, -1, -1, -1, vec2(0.0f), vec2(0.0f), _clock::now());
				event.device = device;
				return event;
			}

			static void Equalize(Event & a, Event & b);
		};

		std::vector<Event> events;

		std::vector<Event> vrEvents;

		vec2 vrMousePos;
		vec2 vrMouseScroll;
		int vrMouseState[8];
		bool vrOverride = false;

		std::mutex eventMutex;

		State current, last;

		Input() { }

		void addEvent(const Event & e);
		void addVREvent(const Event & e);
		void clearEvents();
		void clear();

		std::vector<Event> getEvents();
		void setEvents(const std::vector<Event> & _events);

		void init();

		void update();

		void renderUI();
};

using InputState = Input::State;
using InputEvent = Input::Event;
//using InputEventType = Input::Event::Type;

namespace std
{
	template <>
	struct hash<InputEvent>
	{
		InputEvent ie;
		size_t value;

		hash() { }

		hash(const InputEvent & _ie) : ie(_ie)
		{
			value = 0;

			value = std::hash<int>()((int)ie.type);

			switch (ie.type)
			{
			case InputEventType::Key:
				value = std::hash<int>()(ie.button) ^ (value << 1);
				// FUCK scancodes!
				//value = std::hash<int>()(ie.scancode) ^ (value << 1);
				value = std::hash<int>()(ie.action) ^ (value << 1);
				value = std::hash<int>()(ie.mods) ^ (value << 1);
				break;
			case InputEventType::Click:
				value = std::hash<int>()(ie.button) ^ (value << 1);
				value = std::hash<int>()(ie.action) ^ (value << 1);
				value = std::hash<int>()(ie.mods) ^ (value << 1);
				break;
			case InputEventType::Scroll:
				//value = std::hash<vec2>(ie.scroll) ^ (value << 1);
				break;
			case InputEventType::Move:
				//value = std::hash<vec2>(ie.pos) ^ (value << 1);
				break;
			default:
				break;
			}
		}

		bool operator==(const hash & rhs) const
		{
			return value == rhs.value;
		}

		bool operator<(const hash & rhs) const
		{
			return value < rhs.value;
		}

		operator size_t() const
		{
			return value;
		}

		// For use as map keys, etc.
		size_t operator()(const InputEvent & ie) const
		{
			return hash(ie).value;
		}
	};
}