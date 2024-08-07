#pragma once

#include <cstddef>

namespace chess {

struct Color {
public:
  bool color;
  static const Color Black;
  static const Color White;

  constexpr bool operator==(const Color& other) const;
  constexpr bool operator!=(const Color& other) const;

  constexpr Color flip() const;

  constexpr size_t to_index() const;

private:
  constexpr explicit Color(bool color);
};

// ========== IMPLEMENTATIONS ==========

constexpr Color::Color(bool color) : color{color} {}

constexpr Color Color::Black{false};
constexpr Color Color::White{true};

constexpr bool Color::operator==(const Color& other) const { return color == other.color; }
constexpr bool Color::operator!=(const Color& other) const { return color != other.color; }

constexpr Color Color::flip() const { return Color{!color}; }

constexpr size_t Color::to_index() const { return color; }

}  // namespace chess