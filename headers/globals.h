#pragma once

#define PROJECT_NAME "3480 codebase"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define _USE_MATH_DEFINES

#include <glm/vec2.hpp>               // vec2, bvec2, dvec2, ivec2 and uvec2
#include <glm/vec3.hpp>               // vec3, bvec3, dvec3, ivec3 and uvec3
#include <glm/vec4.hpp>               // vec4, bvec4, dvec4, ivec4 and uvec4
#include <glm/mat2x2.hpp>             // mat2, dmat2
#include <glm/mat2x3.hpp>             // mat2x3, dmat2x3
#include <glm/mat2x4.hpp>             // mat2x4, dmat2x4
#include <glm/mat3x2.hpp>             // mat3x2, dmat3x2
#include <glm/mat3x3.hpp>             // mat3, dmat3
#include <glm/mat3x4.hpp>             // mat3x4, dmat2
#include <glm/mat4x2.hpp>             // mat4x2, dmat4x2
#include <glm/mat4x3.hpp>             // mat4x3, dmat4x3
#include <glm/mat4x4.hpp>             // mat4, dmat4

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/common.hpp>
#include <glm/gtc/epsilon.hpp>

// GLM primitives
using vec2 = glm::vec2;
using dvec2 = glm::dvec2;
using ivec2 = glm::ivec2;
using uvec2 = glm::uvec2;
using bvec2 = glm::bvec2;
using vec3 = glm::vec3;
using dvec3 = glm::dvec3;
using ivec3 = glm::ivec3;
using uvec3 = glm::uvec3;
using bvec3 = glm::bvec3;
using vec4 = glm::vec4;
using dvec4 = glm::dvec4;
using ivec4 = glm::ivec4;
using uvec4 = glm::uvec4;
using bvec4 = glm::bvec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using dmat2 = glm::dmat2;
using dmat3 = glm::dmat3;
using dmat4 = glm::dmat4;


#include <glm/gtx/hash.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/detail/qualifier.hpp>

using quaternion = glm::fquat;
using dualquaternion = glm::fdualquat;

struct glm_decomp
{
	vec3 position;
	vec3 scale;
	quaternion rotation;
	vec3 skew;
	vec4 persp;
};

namespace glm
{
	inline glm_decomp decompose(const mat4& m)
	{
		glm_decomp d;
		glm::decompose(m, d.scale, d.rotation, d.position, d.skew, d.persp);
		return d;
	}

	template<typename T>
	struct compare_less
	{
		bool operator()(const T& a, const T& b) const
		{
			bool lt = true;
			for (int i = 0; i < a.length() && i < b.length(); i++)
			{
				lt &= a[i] < b[i];
				if (!lt) break;
			}
			return lt;
		}
	};
}



#include <cfloat>
#include <climits>
#include <cstdint>
#include <ctime>

#include <chrono>
#include <algorithm>
#include <array>
#include <memory>
#include <vector>
#include <optional>
#include <regex>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <utility>
#include <iso646.h>

// Including util libraries here because I want them everywhere
#include <fmt/format.h>

// Helper functions for converting glm types to strings, jsons, etc.

#if defined(snprintf)
#undef snprintf
#endif

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "StringUtil.h"

namespace glm
{
	// Helper functions to convert glm types to/from json
	// to_json and from_json are both defined to support converting arbitrary types directly to json.
	// see: https://nlohmann.github.io/json/features/arbitrary_types/

	// JSON conversion functions for (u|i|b|d)*vec(1|2|3|4)
	template<int L, typename T, qualifier Q>
	inline void to_json(json& j, const vec<L, T, Q>& v)
	{
		if constexpr (L == 1) j = { {"x", v.x } };
		else if constexpr (L == 2) j = { {"x", v.x }, {"y", v.y } };
		else if constexpr (L == 3) j = { {"x", v.x }, {"y", v.y }, {"z", v.z } };
		else if constexpr (L == 4) j = { {"x", v.x }, {"y", v.y }, {"z", v.z }, {"w", v.w } };
	}

	template<int L, typename T, qualifier Q>
	inline void from_json(const json& j, vec<L, T, Q>& v)
	{
		if constexpr (L >= 1) j.at("x").get_to(v.x);
		if constexpr (L >= 2) j.at("y").get_to(v.y);
		if constexpr (L >= 3) j.at("z").get_to(v.z);
		if constexpr (L >= 4) j.at("w").get_to(v.w);
	}

	// String conversions for (u|i|b|d)*vec(2|3|4)
	template<int L, typename T, qualifier Q>
	inline vec<L, T, Q> from_string(const ::std::string & s) {
		vec<L, T, Q> result;

		const std::regex valuesRegex(R"TOOTHMAN(\(.*\))TOOTHMAN");
		std::smatch valueMatch;

		if (std::regex_search(s, valueMatch, valuesRegex)) {
			// Match found, so get the string contents
			auto resultMatch = valueMatch[0].str();

			resultMatch = StringUtil::replaceAll(resultMatch, "(", "");
			resultMatch = StringUtil::replaceAll(resultMatch, ")", "");
			resultMatch = StringUtil::replaceAll(resultMatch, ",", "");

			size_t i = 0;

			std::istringstream inputs(resultMatch);

			T v;

			while (inputs >> v && i < L) {
				result[i] = v;
#ifdef DEBUG				
				fmt::print("Parsed index {0}, value {1}\n", i, v);
#endif
				i++;
			}
		}

		return result;
	}

	// String conversions for mat(2|3|4)
	//template<int L, typename T, qualifier Q>
	template<int X, int Y, typename T, qualifier Q>	
	inline mat<X, Y, T, Q> from_string(const ::std::string & s) {
		mat<X, Y, T, Q> result;

		const std::regex valuesRegex(R"TOOTHMAN(\(.*\))TOOTHMAN");
		std::smatch valueMatch;

		if (std::regex_search(s, valueMatch, valuesRegex)) {
			// Match found, so get the string contents
			auto resultMatch = valueMatch[0].str();

			resultMatch = StringUtil::replaceAll(resultMatch, "(", "");
			resultMatch = StringUtil::replaceAll(resultMatch, ")", "");
			resultMatch = StringUtil::replaceAll(resultMatch, ",", "");

			size_t i = 0;

			std::istringstream inputs(resultMatch);

			T v;

			T * ptr = value_ptr(result);

			while (inputs >> v && i < (X * Y)) {
				ptr[i] = v;
#ifdef DEBUG				
				fmt::print("Parsed index {0}, value {1}\n", i, v);
#endif
				i++;
			}
		}

		return result;
	}

	// JSON conversion functions for quaternions
	inline void to_json(json& j, const quaternion& q)
	{
		j = { {"x", q.x }, {"y", q.y }, {"z", q.z }, {"w", q.w } };
	}

	inline void from_json(const json& j, quaternion& q)
	{
		j.at("x").get_to(q.x);
		j.at("y").get_to(q.y);
		j.at("z").get_to(q.z);
		j.at("w").get_to(q.w);
	}

	// JSON conversion functions for matrices
	template<int C, int R, typename T, qualifier Q>
	inline void to_json(json& j, const mat<C, R, T, Q>& m)
	{
		for(int c = 0; c < C; c++)
		for (int r = 0; r < R; r++) {
			j.push_back(m[c][r]);
		}
	}

	template<int C, int R, typename T, qualifier Q>
	inline void from_json(const json& j, mat<C, R, T, Q>& m)
	{
		int i = 0;
		for (int c = 0; c < C; c++)
		for (int r = 0; r < R; r++) {
			m[c][r] = j[i].get<T>();
			i++;
		}
	}

	template <typename V>
	inline V lerp(float progress, const V v0, const V v1) {
		return (1.f - progress) * v0 + progress * v1;
	}

	// Consistent world coordinate system
	// WARNING: Even though directions are defined here, code throughout assumes it's still right-handed
	//            e.g.: Right x Forward == Up

	constexpr vec3 Forward = vec3(1., 0., 0.);
	constexpr vec3 Right = vec3(0., 0., 1.);
	constexpr vec3 Up = vec3(0., 1., 0.);
	// constexpr vec3 Forward = vec3(0., 0., -1.);
	// constexpr vec3 Right = vec3(1., 0., 0.);
	// constexpr vec3 Up = vec3(0., 1., 0.);

	constexpr vec3 PitchUp = Right; // Pitch UP is a POSITIVE rotation around RIGHT
	constexpr vec3 PitchDown = -Right;
	constexpr vec3 RollRight = Forward; // Roll RIGHT is a POSITIVE rotation around FORWARD
	constexpr vec3 RollLeft = -Forward;
	constexpr vec3 YawLeft = Up; // Yaw LEFT is a POSITIVE rotation around UP
	constexpr vec3 YawRight = -Up;

	inline void quat2FUR(quaternion quat, vec3 &fwd, vec3 &up, vec3 &right) {
		mat3 rotato = glm::mat3_cast(quat);
		fwd   = rotato * Forward;
		up    = rotato * Up;
		right = rotato * Right;
	}

	template<typename T, qualifier Q>
	GLM_FUNC_QUALIFIER qua<T, Q> quatLookie(vec<3, T, Q> const& direction, vec<3, T, Q> const& approx_up)
	{
		mat<3, 3, T, Q> Result;

		// I don't understand why only right was normalized in the previous implementation.
		vec<3, T, Q> fwd = direction * inversesqrt(max(static_cast<T>(0.00001), dot(direction, direction)));
		vec<3, T, Q> right = cross(fwd, approx_up);
		right *= inversesqrt(max(static_cast<T>(0.00001), dot(right, right)));
		vec<3, T, Q> up = cross(right, fwd);

		Result[0] = fwd * Forward.x + up * Up.x + right * Right.x;
		Result[1] = fwd * Forward.y + up * Up.y + right * Right.y;
		Result[2] = fwd * Forward.z + up * Up.z + right * Right.z;

		return quat_cast(Result);
	}
}

using float_vector = std::vector<float>;
using int_vector = std::vector<int>;
using string_vector = std::vector<std::string>;

template <typename T = float>
using float_map = std::map<float, T>;

template <typename T = int>
using int_map = std::map<int, T>;

// Time types and functions
using _clock = std::chrono::steady_clock;
using _elapsed = std::chrono::duration<double>;
using _time = std::chrono::time_point<_clock, _elapsed>;

inline std::time_t current_time_t()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

// Returns the seconds elapsed since the program was initialized (wrapper for glfwGetTime())
double getTime();

using Command = std::function<void()>;
using TimedCommand = std::function<bool()>;

// usings for keeping track of GLSL uniforms
using glsl_var = std::pair<const char*, const char*>;
using glsl_list = std::vector<glsl_var>;
using glsl_text = std::vector<std::string>;

// Shorthand for pointers (and containers of)
template<typename t>
using ptr = t*;

template<typename t>
using ptr_vector = std::vector<t*>;

template<typename t>
using ptr_stack = std::stack<t*>;

template<typename t>
using ptr_queue = std::queue<t*>;

template<typename t>
using s_ptr = std::shared_ptr<t>;

template<typename t>
using s_ptr_vector = std::vector<s_ptr<t>>;

template<typename t>
using s_ptr_stack = std::stack<s_ptr<t>>;

template<typename t>
using s_ptr_queue = std::queue<s_ptr<t>>;

template<typename t>
using w_ptr = std::weak_ptr<t>;

template<typename t>
using w_ptr_vector = std::vector<w_ptr<t>>;

template<typename t>
using w_ptr_stack = std::stack<w_ptr<t>>;

template<typename t>
using w_ptr_queue = std::queue<w_ptr<t>>;

template<typename t>
using u_ptr = std::unique_ptr<t>;

template<typename t>
using u_ptr_vector = std::vector<u_ptr<t>>;

template<typename t>
using u_ptr_stack = std::stack<u_ptr<t>>;

template<typename t>
using u_ptr_queue = std::queue<u_ptr<t>>;

// Aliases for GLFW input callbacks
using keyCallback = void(*)(int key, int scancode, int action, int mods);
using mouseButtonCallback = void(*)(int button, int action, int mods);
using mouseScrollCallback = void(*)(double xoff, double yoff);
using mousePosCallback = void(*)(double x, double y);

// 2021-07-24 update: wow, I was up to the same shit with typemaps almost 3 years ago!
// And I think I was pretty wrong??
// This doesn't seem to work yet, at least not as of 2018-09-10.
// Type map
#include <typeindex>
#include <typeinfo>
template <typename Type>
using typemap = std::unordered_map<std::type_index, Type>;

// Named object template
template<typename T>
struct NamedObject
{
	std::string name;
	T obj;

	T& operator()()
	{
		return obj;
	}

	NamedObject() { }

	NamedObject(const std::string& n, const T& o)
		: name(n), obj(o) { }

	NamedObject(const std::string& n, T&& o)
		: name(n), obj(o) { }
};

template <typename T>
using named_vector = std::vector<NamedObject<T>>;

template <typename T>
using s_named_vector = std::vector<NamedObject<s_ptr<T>>>;

// Selection template
template<typename T>
inline s_ptr_vector<T> getSelected(const s_ptr_vector<T>& objects)
{
	s_ptr_vector<T> results;
	for (auto& o : objects)
	{
		if (o->selected)
		{
			results.push_back(o);
		}
	}
	return results;
}

// Timed-value template
template <typename T>
struct Timestamp
{
	T value;
	_time at;

	bool operator==(const Timestamp<T>& rhs) const
	{
		return (&rhs == this || (value == rhs.value && at == rhs.at));
	}

	bool operator !=(const Timestamp<T>& rhs) const
	{
		return !(*this == rhs);
	}
};

// Timed-value template
template <typename T>
struct Timed
{
	T value;
	double at;

	bool operator==(const Timed<T>& rhs) const
	{
		return (&rhs == this || (value == rhs.value && at == rhs.at));
	}

	bool operator !=(const Timed<T>& rhs) const
	{
		return !(*this == rhs);
	}
};

// Sampler type
template<typename T = float, size_t N = 64>
using sampler = std::array<T, N>;

// Transform struct (XF)
struct XF// : public ISerializable
{
	vec3 scale = vec3(1.0f);
	vec3 position = vec3(0.0f);
	quaternion rotation = glm::identity<quaternion>();

	XF() { }

	XF(const json& j)
	{
		const auto& js = j["scale"];
		scale = vec3(js[0], js[1], js[2]);
		const auto& jp = j["position"];
		position = vec3(jp[0], jp[1], jp[2]);
		const auto& jr = j["rotation"];
		rotation = quaternion(jr[0], jr[1], jr[2], jr[3]);
	}

	operator json()
	{
		json j;
		j["scale"] = { scale.x, scale.y, scale.z };
		j["position"] = { position.x, position.y, position.z };
		j["rotation"] = { rotation.w, rotation.x, rotation.y, rotation.z };

		return j;
	}

	XF(const vec3& pos, const quaternion& r, const vec3& s)
		: scale(s), position(pos), rotation(r)
	{

	}

	XF(const mat4& m)
	{
		vec3 skew;
		vec4 persp;
		glm::decompose(m, scale, rotation, position, skew, persp);
	}

	mat4 toMatrix() const
	{
		return glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale);
	}

	XF inverse() const
	{
		return XF(glm::inverse(toMatrix()));
	}

	bool operator==(const XF& rhs) const
	{
		return scale == rhs.scale && position == rhs.position && rotation == rhs.rotation;
	}

	bool operator!=(const XF& rhs) const
	{
		return !this->operator==(rhs);
	}

	// Multiplying transforms should result in transform that produces both.
	// Assume that this object is the parent and the rhs is the child

	//virtual bool saveToFile(std::ofstream & fout)
	//{
	//	if (!fout.is_open()) return false;
	//	if (!IO::writeContents(fout, FileContents::XF)) return false;

	//	if (!scale.saveToFile(fout)) return false;
	//	if (!position.saveToFile(fout)) return false;
	//	//if (!rotation.saveToFile(fout)) return false;

	//	return true;
	//}

	//virtual bool loadFromFile(std::ifstream & fin)
	//{
	//	if (!fin.is_open()) return false;
	//	if (!IO::readContents(fin, FileContents::XF)) return false;

	//	if (!scale.loadFromFile(fin)) return false;
	//	if (!position.loadFromFile(fin)) return false;
	//	//if (!rotation.loadFromFile(fin)) return false;

	//	return true;
	//}

	static XF linterp(float t, const XF& a, const XF& b)
	{
		if (t < 0) return a;
		if (t > 1) return b;
		if (a == b) return a;

		XF result;

		result.scale = glm::mix(a.scale, b.scale, t);
		result.rotation = glm::slerp(a.rotation, b.rotation, t);
		result.position = glm::mix(a.position, b.position, t);

		return result;
	}
};

// XF composition behaves like Unity Transforms, as far as I can tell...
// Scale and rotation: straight multiplied from parent to child. 
// Position: The child's scale doesn't factor into the position result, but the parent's does.
// It also is subject to the parent's rotation, so apply that before adding the parent's position.
inline XF operator*(const XF& parent, const XF& child)
{
	XF result;
	result.scale = parent.scale * child.scale;
	//result.rotation = child.rotation * parent.rotation; // this is probably the correct line
	result.rotation = parent.rotation * child.rotation;
	result.position = parent.position + (parent.rotation * (child.position * parent.scale));
	return result;
}

// Default constructor for enums
#define BETTER_ENUMS_DEFAULT_CONSTRUCTOR(Enum) public: Enum() = default;

#include <enum.h>

#include <cassert>

void logString(const std::string& s);

template<typename ... Args>
void log(const char* arg0, const Args & ... args)
{
	std::string s = fmt::format(arg0, args...);
	fmt::print(s);
	logString(s);
}

#define MAKE_ENUM(EnumName, UnderlyingType, ...)								\
	BETTER_ENUM(EnumName, UnderlyingType, __VA_ARGS__);							\
	inline void to_json(json& j, const EnumName & val) {						\
		j[EnumName::_name()] = val._to_string();							\
	}																			\
	inline void from_json(const json& j, EnumName & val) {						\
		val = EnumName::_from_string_nocase(j[EnumName::_name()].get<std::string>().c_str());				\
	}																			\

MAKE_ENUM(MoverType, int, None, Player, Goodie, Baddie);
MAKE_ENUM(SimpleShapeType, int, sphere, box, joint, quad, tri, model);
MAKE_ENUM(ColliderType, int, None, Static, Dynamic, Kinematic);
MAKE_ENUM(GBufferMode, int, Rendered, Color, Position, Normal, PrimitiveData, Depth, Stencil, Shadow);
MAKE_ENUM(ProjectionType, int, Perspective, Orthographic);
// First four values are for mouse and keyboard input. Last one is for generic device
MAKE_ENUM(InputEventType, int, Key, Click, Scroll, Move, Device);
MAKE_ENUM(PhysicsIntegrationMethod, int, ForwardEuler, SemiImplicitEuler);
MAKE_ENUM(Axis, int, X, Y, Z);

// Forward declarations
class Framebuffer;
class FullscreenFilters;
typedef struct GLFWwindow GLFWwindow;

using EID = unsigned long;

#define IMDENT ImGui::Indent(16.f)
#define IMDONT ImGui::Unindent(16.f)



struct Physics {
	int shapeIndex = -1;
	float mass = 1.0f;
	float inverseMass = 1.0f;
	vec3 force = vec3(0.f);
	vec3 forceDerivative = vec3(0.f);
	float maxForceMagnitude = 10.0f;
	vec3 position = vec3(0.f);
	vec3 linearMomentum = vec3(0.f);
	vec3 linearVelocity = vec3(0.f);
	vec3 linearVelocityDerivative = vec3(0.f);
	vec3 linearAcceleration = vec3(0.f);
	vec3 rotation = vec3(0.f);
	vec3 angularVelocity = vec3(0.f);
	vec3 angularAcceleration = vec3(0.f);
	bool gravity = true;
	bool isStatic = false;
	PhysicsIntegrationMethod method = PhysicsIntegrationMethod::SemiImplicitEuler;

	void* target = nullptr;

	void recalculate() {
		linearVelocity = linearMomentum * inverseMass;
	}
};

inline void to_json(json& j, const Physics& p) {
	j = json{
		{"shapeIndex", p.shapeIndex},
		{"mass", p.mass},
		{"inverseMass", p.inverseMass},
		{"position", p.position},
		{"rotation", p.rotation},
		{"gravity", p.gravity},
		{"isStatic", p.isStatic},
		{"method", p.method}
	};
}

inline void from_json(const json& j, Physics & p) {
	p.shapeIndex = j.at("shapeIndex").get_to(p.shapeIndex);
	p.mass = j.at("mass").get_to(p.mass);
	p.inverseMass= j.at("inverseMass").get_to(p.inverseMass);
	p.position = j.at("position").get_to(p.position);
	p.rotation = j.at("rotation").get_to(p.rotation);
	p.gravity = j.at("gravity").get_to(p.gravity);
	p.isStatic = j.at("isStatic").get_to(p.isStatic);
	p.method = j.at("method").get_to(p.method);
}