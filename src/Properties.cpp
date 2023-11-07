#include "Properties.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"

#include <unordered_set>

int decimalFilter(ImGuiInputTextCallbackData * data)
{
	static std::unordered_set<char> ok = { '+', '-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'e' };

	if (ok.find(data->EventChar) == ok.end())
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int vectorFilter(ImGuiInputTextCallbackData * data)
{
	static std::unordered_set<char> ok = { '+', '-', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'e',
		'[', ']', ',' };

	if (ok.find(data->EventChar) == ok.end())
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void renderLabel(const std::string & name = "")
{
	if (!name.empty())
	{
		//ImGui::PushID((int)&name);
		ImGui::PushID(name.c_str());
		ImGui::Text("%s", name.c_str());
		ImGui::SameLine();
		ImGui::PopID();
	}
}

template<typename T>
bool renderVector(T & val, const std::string & name = "", bool isFloat = true)
{
	static std::string components = "XYZW";
	
	renderLabel(name);

	ImGui::PushItemWidth(60.0f);
	bool changed = false;

	char label[2] = { '\0', '\0' };
	for (int i = 0; i < val.length(); i++)
	{
		label[0] = components[i];
		auto ptr = &val[i];
		//ImGui::PushID((int)ptr);
		ImGui::PushID((const void*)ptr);
		ImGui::Text("%s", label);
		ImGui::SameLine();
		if (isFloat)
		{
			changed |= ImGui::InputFloat("", (float*)ptr);
		}
		else
		{
			changed |= ImGui::InputInt("", (int*)ptr);
		}
		ImGui::SameLine();
		ImGui::PopID();
	}
	ImGui::PopItemWidth();
	return changed;
}

json IProperties::toJSON() const
{
	json js;
	for (const auto & p : properties)
	{
		js[name].push_back(p->toJSON());
	}
	return js;
}

template<>
void Property<bool>::renderSpecial()
{
	bool val = value;
	if (ImGui::Checkbox(name.c_str(), &val))
	{
		setVal(val);
	}
}

template<>
void Property<int>::renderSpecial()
{
	auto val = value;

	if (enforceClamp)
	{	
		if (ImGui::SliderInt(name.c_str(), &val, min, max))
		{
			setVal(val);
		}
	}
	else
	{
		if (ImGui::InputInt(name.c_str(), &val))
		{
			setVal(val);
		}
	}
}

template<>
void Property<unsigned int>::renderSpecial()
{
	auto val = (int)value;

	if (enforceClamp)
	{	
		if (ImGui::SliderInt(name.c_str(), &val, min, max))
		{
			setVal((unsigned int)val);
		}
	}
	else
	{
		if (ImGui::InputInt(name.c_str(), &val))
		{
			setVal((unsigned int)val);
		}
	}
}

template<>
void Property<float>::renderSpecial()
{
	auto val = value;

	if (enforceClamp)
	{	
		if (ImGui::SliderFloat(name.c_str(), &val, min, max))
		{
			setVal(val);
		}
	}
	else
	{
		if (ImGui::InputFloat(name.c_str(), &val))
		{
			setVal(val);
		}
	}
}

template<>
void Property<vec2>::renderSpecial()
{
	auto val = value;
	if (enforceClamp)
	{
		if (ImGui::SliderFloat2(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(val);
		}
	}
	else
	{
		if (renderVector<vec2>(val, name))
		{
			setVal(val);
		}
	}
}

template<>
void Property<ivec2>::renderSpecial()
{
	auto val = value;
	if (enforceClamp)
	{
		if (ImGui::SliderInt2(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(val);
		}
	}
	else
	{
		if (renderVector<ivec2>(val, name, false))
		{
			setVal(val);
		}
	}
}

template<>
void Property<uvec2>::renderSpecial()
{
	ivec2 val = value;
	if (enforceClamp)
	{
		if (ImGui::SliderInt2(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(uvec2(val.x, val.y));
		}
	}
	else
	{
		if (renderVector<ivec2>(val, name, false))
		{
			setVal(uvec2(val.x, val.y));
		}
	}
}

template<>
void Property<vec3>::renderSpecial()
{
	auto val = value;
	if (enforceClamp)
	{
		
		if (ImGui::SliderFloat3(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(val);
		}
	}
	else if (flags.color)
	{
		auto val = value;
		if (ImGui::ColorEdit3(name.c_str(), glm::value_ptr(val)))
		{
			setVal(val);
		}
	}
	else
	{
		if (renderVector<vec3>(val, name))
		{
			setVal(val);
		}
	}
}

template<>
void Property<ivec3>::renderSpecial()
{
	auto val = value;
	if (enforceClamp)
	{
		if (ImGui::SliderInt3(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(val);
		}
	}
	else
	{
		if (renderVector<ivec3>(val, name, false))
		{
			setVal(val);
		}
	}
}

template<>
void Property<vec4>::renderSpecial()
{
	auto val = value;
	if (enforceClamp)
	{
		if (ImGui::SliderFloat4(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(val);
		}
	}
	else if (flags.color)
	{
		auto val = value;
		if (ImGui::ColorEdit4(name.c_str(), glm::value_ptr(val)))
		{
			setVal(val);
		}
	}
	else
	{
		if (renderVector<vec4>(val, name))
		{
			setVal(val);
		}
	}
}

template<>
void Property<ivec4>::renderSpecial()
{
	auto val = value;
	if (enforceClamp)
	{
		
		if (ImGui::SliderInt4(name.c_str(), glm::value_ptr(val), min.x, max.x))
		{
			setVal(val);
		}
	}
	else
	{
		if (renderVector<ivec4>(val, name, false))
		{
			setVal(val);
		}
	}
}

/*else if (auto v2p = dynamic_cast<Settings::Property<vec2>*>(p))
{
if (v2p->enforceClamp)
{
auto newVal = v2p->value;
if (ImGui::SliderFloat2(v2p->name.c_str(), newVal.value, v2p->min.x, v2p->max.x))
{
v2p->setVal(newVal);
}
}
else
{
v2p->renderText();
}
}
else if (auto iv2p = dynamic_cast<Settings::Property<ivec2>*>(p))
{
if (iv2p->enforceClamp)
{
auto newVal = iv2p->value;
if (ImGui::SliderInt2(iv2p->name.c_str(), newVal.value, iv2p->min.x, iv2p->max.x))
{
iv2p->setVal(newVal);
}
}
else
{
iv2p->renderText();
}
}
else if (auto v3p = dynamic_cast<Settings::Property<vec3>*>(p))
{
if (v3p->enforceClamp)
{
auto newVal = v3p->value;
if (ImGui::SliderFloat3(v3p->name.c_str(), newVal.value, v3p->min.x, v3p->max.x))
{
v3p->setVal(newVal);
}
}
else
{
v3p->renderText();
}
}
else if (auto iv3p = dynamic_cast<Settings::Property<ivec3>*>(p))
{
if (iv3p->enforceClamp)
{
auto newVal = iv3p->value;
if (ImGui::SliderInt3(iv3p->name.c_str(), newVal.value, iv3p->min.x, iv3p->max.x))
{
iv3p->setVal(newVal);
}
}
else
{
iv3p->renderText();
}
}
else if (auto v4p = dynamic_cast<Settings::Property<vec4>*>(p))
{
if (v4p->enforceClamp)
{
auto newVal = v4p->value;
if (ImGui::SliderFloat4(v4p->name.c_str(), newVal.value, v4p->min.x, v4p->max.x))
{
v4p->setVal(newVal);
}
}
else
{
v4p->renderText();
}
}
else if (auto iv4p = dynamic_cast<Settings::Property<ivec4>*>(p))
{
if (iv4p->enforceClamp)
{
auto newVal = iv4p->value;
if (ImGui::SliderInt4(iv4p->name.c_str(), newVal.value, iv4p->min.x, iv4p->max.x))
{
iv4p->setVal(newVal);
}
}
else
{
iv4p->renderText();
}
}*/

//void Settings::loadFile(const std::string & settingsFile)
//{
//	settings = IJsonable::loadFromFile(settingsFile);
//}
//
//void Settings::saveToFile(const std::string & settingsFile)
//{
//	IJsonable::saveToFile(settings, settingsFile);
//}