#pragma once

namespace chess {

enum class Color : bool { Black = 0, White = 1 };

namespace color {
constexpr Color flip(Color color);
}

// =============== IMPLEMENTATIONS ===============

inline constexpr Color color::flip(Color color) { return static_cast<Color>(!static_cast<bool>(color)); }

}  // namespace chess