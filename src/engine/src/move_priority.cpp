#include "move_priority.h"

#include <array>
#include <cstdint>

#include "chess/color.h"
#include "chess/move.h"
#include "heuristics.h"
#include "killer_moves.h"

// Scores for move priority.
namespace move_priority {
constexpr int32_t hash_move{1'000'000};

constexpr int32_t capture{400'000};

// Ordered by most valuable victim, then least valuable attacker. Indexed by [victim][attacker].
constexpr std::array<std::array<int32_t, 6>, 7> mvv_lva{[]() {
  std::array<std::array<int32_t, 6>, 7> mvv_lva{};

  using chess::PieceType;
  constexpr std::array pieces_by_value{PieceType::King,   PieceType::Queen,  PieceType::Rook,
                                       PieceType::Bishop, PieceType::Knight, PieceType::Pawn};

  // King victim (just filler as king can't be captured)
  mvv_lva[static_cast<size_t>(PieceType::King)].fill(0);
  // None victim (i.e. not a capture)
  mvv_lva[static_cast<size_t>(PieceType::None)].fill(0);

  int32_t base{50};
  for (const auto victim : pieces_by_value) {
    if (victim == PieceType::King) continue;
    int32_t increment{0};
    for (const auto attacker : pieces_by_value) {
      mvv_lva[static_cast<size_t>(victim)][static_cast<size_t>(attacker)] = base + increment;
      increment++;
    }
    base -= 10;
  }

  return mvv_lva;
}()};

constexpr int32_t promotion{300'000};
constexpr std::array<int, 6> promotion_piece = []() {
  std::array<int, 6> promotion_piece{};
  promotion_piece[static_cast<size_t>(chess::PieceType::Knight)] = 1;
  promotion_piece[static_cast<size_t>(chess::PieceType::Bishop)] = 2;
  promotion_piece[static_cast<size_t>(chess::PieceType::Rook)] = 3;
  promotion_piece[static_cast<size_t>(chess::PieceType::Queen)] = 4;
  return promotion_piece;
}();

constexpr int32_t killer{200'000};
constexpr int killer_index{1};  // To prioritise more recent killer moves.

constexpr int32_t history_heuristic_scale = {100'000};
}  // namespace move_priority

MovePriority MovePriority::evaluate(const chess::Move& move, int32_t depth_left, const chess::Move& hash_move,
                                    chess::Color player_color, Heuristics& heuristics) {
  if (move == hash_move) return MovePriority{move_priority::hash_move};

  int32_t priority = 0;

  if (move.is_capture()) {
    // MVV LVA priority.
    priority +=
        move_priority::capture +
        move_priority::mvv_lva[static_cast<size_t>(move.get_captured_piece())][static_cast<size_t>(move.get_piece())];
  }

  if (move.is_promotion()) {
    priority += move_priority::promotion + move_priority::promotion_piece[static_cast<int>(move.get_promotion_piece())];
  }

  if (!move.is_capture()) {
    bool is_killer = false;
    for (size_t i = 0; i < KillerMoves::count; i++) {
      // Killer move priority.
      if (heuristics.killer_moves.get(depth_left, i) == move) {
        priority += move_priority::killer - move_priority::killer_index * i;
        is_killer = true;
        break;
      }
    }

    if (!is_killer) {
      priority += heuristics.history_heuristic.get_ratio(player_color, move.get_from(), move.get_to()) *
                  move_priority::history_heuristic_scale;
    }
  }

  return MovePriority{priority};
}

MovePriority MovePriority::evaluate_quiescence(const chess::Move& move) {
  int32_t priority = 0;

  if (move.is_capture()) {
    // MVV LVA priority.
    priority +=
        move_priority::capture +
        move_priority::mvv_lva[static_cast<size_t>(move.get_captured_piece())][static_cast<size_t>(move.get_piece())];
  }

  return MovePriority{priority};
}