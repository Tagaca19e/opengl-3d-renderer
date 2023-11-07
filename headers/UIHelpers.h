#pragma once

#include "globals.h"
#include "imgui.h"

struct ImGuiEditText
{
	std::string text;
	std::vector<char> buf;
	bool changed;

	ImGuiEditText()
	{
		buf.assign(1024U, '\0');
		changed = false;
	}

	ImGuiEditText(const std::string & txt)
		: ImGuiEditText()
	{
		text = txt;
		changed = true;
	}

	void update()
	{
		// Check if we have to update buffer
		if (text.length() > buf.size())
		{
			buf.resize(size_t(text.length() * 1.5));
		}
		if (changed)
		{
			buf.assign(buf.size(), '\0');
			text.copy((char*)buf.data(), std::min(text.length(), buf.size()));
			text.copy((char*)buf.data(), text.length() < buf.size() ? text.length() : buf.size());
		}
	}

	bool render(const std::string & name = "", bool multiline = false)
	{
		update();
		ImGui::PushID((void*)this);
		
		if (!name.empty())
		{
			ImGui::TextUnformatted(name.c_str());
			ImGui::SameLine();
		}
		bool txtChanged = false;
		if (multiline)
		{
			txtChanged = ImGui::InputTextMultiline("", &buf[0], buf.size());
		}
		else
		{
			txtChanged = ImGui::InputText("", &buf[0], buf.size());
		}
		if (txtChanged)
		{
			text = std::string(buf.data());
		}
		ImGui::PopID();
		return txtChanged;
	}
};

struct ImGuiTextFilePrompt
{
	std::string name;
	std::string ext;
	ImGuiEditText txt;
	bool isFolder = false;
	
	ImGuiTextFilePrompt() { }

	ImGuiTextFilePrompt(const std::string & _name, const std::string & _ext = "", const std::string & _txt = "", bool _isFolder = false)
		: name(_name), txt(_txt), ext(_ext), isFolder(_isFolder)
	{ }

	std::string & operator()()
	{
		return txt.text;
	}

	std::string operator()() const
	{
		return txt.text;
	}

	void render(const std::string browseStart = "");
};

class ImGuiConsole
{
public:
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset
	bool                ScrollToBottom;
	int					ScrollCount = 0;

	void    Clear() { Buf.clear(); LineOffsets.clear(); }

	void    AddLog(const std::string& entry)
	{
		int old_size = Buf.size();
		Buf.appendf("%s", entry.c_str());
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size);
		ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		ImGui::Begin(title, p_open);
		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy) ImGui::LogToClipboard();

		if (Filter.IsActive())
		{
			const char* buf_begin = Buf.begin();
			const char* line = buf_begin;
			for (int line_no = 0; line != NULL; line_no++)
			{
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
				if (Filter.PassFilter(line, line_end))
					ImGui::TextUnformatted(line, line_end);
				line = line_end && line_end[1] ? line_end + 1 : NULL;
			}
		}
		else
		{
			ImGui::TextUnformatted(Buf.begin());
		}

		if (ScrollToBottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
			ScrollCount++;
			if (ScrollCount == 2)
			{
				ScrollCount = 0;
				ScrollToBottom = false;
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}
};

// Iterates through enum choices and loops back around
template <typename T>
bool renderEnumButton(T & current)
{
	if (ImGui::Button(current._to_string()))
	{
		current = T::_values()[(current._to_integral() + 1) % T::_size_constant];
		return true;
	}
	return false;
}

// Iterates through enum choices and loops back around
template <typename T>
bool renderEnumDropDown(const std::string & label, T & current)
{
	static std::vector<const char *> names(T::_names().begin(), T::_names().end());
	
	//int selected = current._to_integral();
	int selected = 0;

	for(const auto & name : names) {
		//auto longestStr = std::max(strlen(name), strlen(current._to_string()));
		//if (strncmp(name, current._to_string(), longestStr)) {
		if (name == current._to_string()) {
			break;
		}
		selected++;
	}
	if (ImGui::Combo(label.c_str(), &selected, names.data(), T::_size_constant))
	{
		current = T::_from_string(names[selected]);
		return true;
	}

	return false;
}

inline bool renderDropDown(const std::string& label, const std::vector<std::string> & options, std::string & current)
{
	//int selected = current._to_integral();

	int selected = 0;
	std::vector<const char*> names;

	for (auto& o : options) {
		names.push_back(o.c_str());
		if (o != current) {
			selected++;
		}
	}

	if (ImGui::Combo(label.c_str(), &selected, names.data(), names.size()))
	{
		current = options[selected];
		return true;
	}

	return false;
}

// Same as the other, but uses the type as the label
template <typename T>
bool renderEnumDropDownNamed( T& current)
{
	static std::vector<const char*> names(T::_names().begin(), T::_names().end());

	int selected = current._to_integral();
	if (ImGui::Combo(typeid(T).name(), &selected, names.data(), T::_size_constant))
	{
		current = T::_from_integral(selected);
		return true;
	}

	return false;
}

// Iterates through enum choices and loops back around
template <typename T>
bool renderOptions(const std::string & label, T & current)
{
	static std::vector<const char *> names(T::_names().begin(), T::_names().end());

	int selected = current._to_integral();
	if (ImGui::Combo(label.c_str(), &selected, names.data(), T::_size_constant))
	{
		current = T::_from_integral(selected);
		return true;
	}

	return false;
}

inline bool renderIntAsCheckbox(const std::string & label, int & val)
{
	bool b = val == 1;
	if (ImGui::Checkbox(label.c_str(), &b))
	{
		val = b ? 1 : 0;
		return true;
	}
	return false;
}


inline void renderSampler(const std::string& label, sampler<>& points)
{

	ImGui::PushID((const void*)& points);
	ImGui::Text("%s", label.c_str());
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	//ImVector<ImVec2> canvasPoints;

	static bool adding_line = false;
	if (ImGui::Button("Clear")) points.fill(0.0f);

	// Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
	// But you can also draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
	// If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
	ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
	ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
	if (canvas_size.x < 100.0f) canvas_size.x = 100.0f;
	if (canvas_size.y < 100.0f) canvas_size.y = 100.0f;
	draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
	draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));

	bool adding_preview = false;
	ImGui::InvisibleButton("canvas", canvas_size);
	ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
	//if (adding_line)
	//{
	//	adding_preview = true;
	//	canvasPoints.push_back(mouse_pos_in_canvas);
	//	if (!ImGui::IsMouseDown(0))
	//		adding_line = adding_preview = false;
	//}
	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseDown(0))
		{
			// Convert mouse pos to value for this array
			vec2 pos_norm = vec2(mouse_pos_in_canvas.x / canvas_size.x, 1.0f - (mouse_pos_in_canvas.y / canvas_size.y));

			pos_norm.x = glm::clamp<float>(pos_norm.x, 0.0f, 1.0f);
			pos_norm.y = glm::clamp<float>(pos_norm.y, 0.0f, 1.0f);

			int index = glm::clamp<int>(glm::round(pos_norm.x * (points.size() - 1)),
				0, (int)points.size() - 1);
			points[index] = pos_norm.y;

		}
		/*if (!adding_line && ImGui::IsMouseClicked(0))
		{
			canvasPoints.push_back(mouse_pos_in_canvas);
			adding_line = true;
		}
		if (ImGui::IsMouseClicked(1) && !canvasPoints.empty())
		{
			adding_line = adding_preview = false;
			canvasPoints.pop_back();
			canvasPoints.pop_back();
		}*/
	}

	float width = canvas_size.x / (float)(points.size() - 1);

	draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
	for (int i = 0; i < points.size() - 1; i++)
	{
		ImVec2 v1(canvas_pos.x + (width * i), canvas_pos.y + canvas_size.y - (points[i] * (canvas_size.y)));
		ImVec2 v2(canvas_pos.x + (width * (i + 1)), canvas_pos.y + canvas_size.y - (points[i + 1] * (canvas_size.y)));
		draw_list->AddLine(v1, v2, IM_COL32(255, 255, 0, 255), 2.0f);
	}
	draw_list->PopClipRect();
	ImGui::PopID();
}

inline bool inputString(const std::string & label, std::string& str)
{
	static std::array<char, 8192> buffer;
	memset((void*)buffer.data(), 0, buffer.size());
	std::copy(str.begin(), str.end(), buffer.data());
	
	if (ImGui::InputText(label.c_str(), buffer.data(), buffer.size()))
	{
		str = std::string(buffer.begin(), buffer.end());
		return true;
	}
	return false;
}

template<int X, int Y, typename T, glm::qualifier Q>	
inline bool imguiInputMatrix(const std::string& label, glm::mat<X, Y, T, Q>& m) {
	ImGui::Text("%s", label.c_str());
	ImGui::NewLine();

	bool changed = false;

	for (int r = 0; r < X; r++) {
		ImGui::PushID(r);
		for (int c = 0; c < Y; c++) {
			ImGui::PushID(c);
			ImGui::SetNextItemWidth(50.f);
			changed |= ImGui::InputFloat("", &m[c][r]);
			ImGui::SameLine();
			ImGui::PopID();
		}
		ImGui::NewLine();
		ImGui::PopID();
	}

	return changed;
}

inline bool inputMat4(const std::string& label, mat4& m) {
	ImGui::Text("%s", label.c_str());
	ImGui::NewLine();

	bool changed = false;

	for (int r = 0; r < 4; r++) {
		ImGui::PushID(r);
		for (int c = 0; c < 4; c++) {
			ImGui::PushID(c);
			ImGui::SetNextItemWidth(50.f);
			changed |= ImGui::InputFloat("", &m[c][r]);
			ImGui::SameLine();
			ImGui::PopID();
		}
		ImGui::NewLine();
		ImGui::PopID();
	}

	return changed;
}