#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

// WinAPI types Re-definitions
using AppWindow = HWND;
using WindowStyleFlags = DWORD;

// STL Libraries
#include <iostream>
#include <memory>
#include <utility>
#include <typeindex>
#include <typeinfo>
#include <any>
#include <optional>
#include <functional>
#include <concepts>
#include <numbers>
#include <limits>
#include <algorithm>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <functional>
using namespace std::literals;
#include <format>

// Math Libraries
#include <cmath>

// STL containers
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <type_traits>

struct is_32bit_system : std::bool_constant<sizeof(void*) == 4ULL> {};
struct is_64bit_system : std::bool_constant<sizeof(void*) == 8ULL> {};
inline constexpr bool is_32bit_system_v{ is_32bit_system::value };
inline constexpr bool is_64bit_system_v{ is_64bit_system::value };

constexpr unsigned int MAX_CSTR_LENGTH = 256U;

// Byte size macros
consteval std::size_t KB(std::size_t kb) { return kb * 1024; }
consteval std::size_t MB(std::size_t mb) { return KB(mb * 1024); }
consteval std::size_t GB(std::size_t gb) { return MB(gb * 1024); }

using Flags = uint32_t;

// Fixed update updating rate
constexpr double FIXED_RATE = 1.0 / 60.0;

template<class Interface>
inline void SafeRelease(Interface** ppInterface) {
	if (*ppInterface != nullptr) {
		(*ppInterface)->Release();
		(*ppInterface) = nullptr;
	}
}

template<class Interface>
inline void SafeRelease(std::unique_ptr<Interface>& upInterface) {
	if (upInterface) {
		upInterface.get()->Release();
		upInterface.release();
	}
}

constexpr float FLOAT_MIN = (std::numeric_limits<float>::min)();
constexpr float FLOAT_MAX = (std::numeric_limits<float>::max)();
constexpr float PI_F = std::numbers::pi_v<float>;
constexpr float PI = std::numbers::pi_v<double>;
constexpr float SQRT2_F = std::numbers::sqrt2_v<float>;
constexpr double SQRT2 = std::numbers::sqrt2_v<double>;