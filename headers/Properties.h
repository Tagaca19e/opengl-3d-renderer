#pragma once

#include "globals.h"
#include "InputOutput.h"
//#include "IO/IJsonable.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "StringUtil.h"

#include "imgui.h"
#include "UIHelpers.h"

struct IProperties
{
	std::string name;
	json::value_t type;
	ptr_vector<IProperties> properties;

	struct TextFilter
	{
		ImGuiEditText filter;
		std::string lastFilter;
		string_vector filters;

		operator bool()
		{
			return !filters.empty();
		}

		bool refresh()
		{
			auto nextFilter = StringUtil::lower(filter.text);

			if (nextFilter != lastFilter)
			{
				filters = StringUtil::split(nextFilter, ',');
				lastFilter = nextFilter;
				return true;
			}
			return false;
		}

		// Returns true if value has changed
		bool update(const std::string & filterName = "filter")
		{
			// Update filters when text changes
			if (filter.render(filterName) && refresh())
			{
				return true;
			}

			return false;
		}
	};

	struct PropertiesFilter
	{
		TextFilter filter;
		std::vector<IProperties*> lastGroup;

		operator bool()
		{
			return filter;
		}

		void update(const ptr_vector<IProperties> properties, const std::string & filterName = "filter")
		{
			// When filter changes, update the properties that get filtered by it
			if (filter.update(filterName))
			{
				// Clear the last group of properties so we can repopulate
				lastGroup.clear();

				// Iterate over the given properties
				for (auto & p : properties)
				{
					// Get the property's name and compare with the filter entries
					auto lname = StringUtil::lower(p->name);

					for (auto & f : filter.filters)
					{
						if (lname.find(f, 0) != std::string::npos)
						{
							lastGroup.push_back(p);
							break;
						}
					}
				}
			}
		}
	};

	PropertiesFilter Filter;

	IProperties() { }
	IProperties(const std::string & n, json::value_t t = json::value_t::null) : name(n), type(t) { }

	virtual ~IProperties() { }

	virtual IProperties & AddTo(IProperties * bag)
	{
		bag->properties.push_back(this);
		return *this;
	}

	virtual json toJSON() const;

	virtual void renderUI()
	{
		ImGui::PushID((void*)this);
		Filter.update(properties);

		auto display = Filter ? &Filter.lastGroup : &properties;

		for(const auto & p : *display)
		{
			p->renderUI();
		}
		ImGui::PopID();
	}
};

template <typename T>
struct Property : public IProperties
{
	using ChangedEvent = std::function<void(IProperties*, const T &, const T &)>;

	T value;
	std::vector<ChangedEvent> changedEvents;

	bool enforceClamp = false;
	T min;
	T max;

	bool enforceRange = false;
	std::vector<Property> range;

	struct Flags
	{
		bool color = false;
	} flags;

	Property(const T & t) : value(t) { }
	Property(const std::string & _name = "", const T & t = T(), json::value_t _type = json::value_t::null)
		: IProperties(_name, _type), value(t)
	{
		if (type == json::value_t::null)
		{
			type = toJSON()[name].type();
		}
	}
	Property(const Property & rhs) : IProperties(rhs.name, rhs.type),
		value(rhs.value), changedEvents(rhs.changedEvents), enforceClamp(rhs.enforceClamp),
		min(rhs.min), max(rhs.max), enforceRange(rhs.enforceRange), range(rhs.range), flags(rhs.flags) { }

	Property(Property && rhs)
	{
		value = rhs.value;
		changedEvents = std::move(rhs.changedEvents);
		enforceClamp = rhs.enforceClamp;
		min = rhs.min;
		max = rhs.max;
		enforceRange = rhs.enforceRange;
		range = std::move(rhs.range);
		flags = rhs.flags;
	}

	virtual ~Property() { }

	Property & SetClamp(const T & _min = T(0), const T & _max = T(1))
	{
		enforceClamp = true;
		min = _min;
		max = _max;
		return *this;
	}

	Property & SetRange(const std::vector<Property> _range)
	{
		enforceRange = true;
		range = _range;
		return *this;
	}

	Property & SetFlags(bool _color = false)
	{
		flags.color = _color;
		return *this;
	}

	Property & AddChangeEvent(ChangedEvent ce)
	{
		changedEvents.push_back(ce);
		return *this;
	}

	Property & AddChangeEvents(std::vector<ChangedEvent> ces)
	{
		changedEvents.insert(changedEvents.end(), ces.begin(), ces.end());
		return *this;
	}

	Property & AddTo(IProperties * bag)
	{
		bag->properties.push_back((IProperties*)this);
		return *this;
	}

	Property & operator=(const Property & rhs)
	{
		if (this != &rhs)
		{
			*this = rhs;
		}

		return *this;
	}

	Property & operator=(const T & newVal)
	{
		setVal(newVal);
		return *this;
	}

	virtual void onChange(const T & oldVal, const T & newVal)
	{
		for (auto & ce : changedEvents)
		{
			ce(this, oldVal, newVal);
		}
	}

	operator T() const
	{
		return value;
	}

	operator T()
	{
		return value;
	}

	virtual void fromString(const std::string & str)
	{
		try
		{
			json rs = json::parse(fmt::format("{{ \"{0}\" : {1} }}", name, str));
			//value = glm::fromJSON<T>(rs);
		}
		catch (...)
		{

		}
	}

	virtual json toJSON() const
	{
		json rs;
		//rs[name] = value;
		return rs;
	}

	virtual void applyJSON(const json & rs)
	{
		//setVal(rs[name]);
	}

	virtual void setVal(const T & val)
	{
		T newVal = value;

		if (enforceClamp)
		{
			newVal = val;
			//newVal = glm::clamp<T>(val, min, max);
		}
		else
		{
			newVal = val;
		}

		/*if (enforceRange)
		{
			bool found = false;
			for (auto & r : range)
			{
				if (r.value == newVal) 
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				newVal = value;
			}
		}*/

		if (newVal != value)
		{
			onChange(value, newVal);
			value = newVal;
		}
	}

	virtual void renderUI()
	{
		ImGui::PushID((void*)this);
		if (enforceRange)
		{
			renderRange();
		}
		else
		{
			renderSpecial();
		}
		ImGui::PopID();
	}

	void renderRange()
	{
		auto numRange = range.size();
		std::vector<const char*> rangeStrs(numRange);
		int i = 0;
		int selected = -1;
		for (auto & r : range)
		{
			if (r.value == value)
			{
				selected = i;
			}
			rangeStrs[i++] = r.name.c_str();
		}

		if (ImGui::Combo(name.c_str(), &selected, rangeStrs.data(), (int)numRange))
		{
			setVal(range[selected]);
		}
	}

	void renderSpecial();

	// Function to use for setting value from string
	using StringToVal = std::function<void(const std::string&)>;

	void renderText(ImGuiInputTextCallback filter = nullptr, std::string result = "", StringToVal callback = nullptr)
	{
		auto newSize = 1024;
		if (newSize <= result.length())
		{
			newSize = result.length() * 2;
		}

		std::vector<char> text(result.length(), '\0');

		result.copy((char*)text.data(), result.length());

		if (ImGui::InputText(name.c_str(), (char*)text.data(), text.size(), (filter ?
			ImGuiInputTextFlags_CallbackCharFilter : 0), filter))
		{
			result = std::string(text.data());
			if (callback)
			{
				callback(result);
			}
		}
	}
};

using BoolProp = Property<bool>;
using FloatProp = Property<float>;
using IntProp = Property<int>;