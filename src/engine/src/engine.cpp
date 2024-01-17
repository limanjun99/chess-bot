#include "engine.h"

#include <limits.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "board_hash.h"
#include "config.h"
#include "evaluation.h"

Engine::Engine() {}

Engine::MoveInfo Engine::choose_move(const Board& board, std::chrono::milliseconds search_time) {
  auto start_time = std::chrono::high_resolution_clock::now();
  normal_node_count = 0;
  quiescence_node_count = 0;
  int search_depth = 0;
  MoveContainer moves = board.generate_moves();
  Move chosen_move{};
  auto is_out_of_time = [&]() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto current_time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return current_time_spent >= search_time * config::search_time_lowerbound;
  };

  // Iterative deepening.
  while (search_depth < config::max_depth) {
    soft_reset();
    int alpha = evaluation::min;
    Move best_move{};
    bool finished_evaluation = true;
    for (size_t i = 0; i < moves.size(); i++) {
      Board new_board = board.apply_move(moves[i]);
      int new_board_evaluation = search(new_board, -evaluation::max, -alpha, search_depth);
      if (-new_board_evaluation > alpha) {
        alpha = -new_board_evaluation;
        best_move = moves[i];
      }
      if (is_out_of_time()) {
        finished_evaluation = false;
        break;
      }
    }

    if (finished_evaluation) chosen_move = best_move;
    if (is_out_of_time()) break;
    search_depth++;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  return {chosen_move, time_spent, search_depth, normal_node_count, quiescence_node_count};
}

Engine::MoveInfo Engine::choose_move(const Board& board, int depth) {
  auto start_time = std::chrono::high_resolution_clock::now();
  normal_node_count = 0;
  quiescence_node_count = 0;
  int search_depth = 0;
  MoveContainer moves = board.generate_moves();
  Move chosen_move{};

  // Iterative deepening.
  while (search_depth <= depth) {
    soft_reset();
    int alpha = evaluation::min;
    Move best_move{};
    for (size_t i = 0; i < moves.size(); i++) {
      Board new_board = board.apply_move(moves[i]);
      int new_board_evaluation = search(new_board, -evaluation::max, -alpha, search_depth);
      if (-new_board_evaluation > alpha) {
        alpha = -new_board_evaluation;
        best_move = moves[i];
      }
    }
    if (search_depth == depth) chosen_move = best_move;
    search_depth++;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  return {chosen_move, time_spent, search_depth, normal_node_count, quiescence_node_count};
}

void Engine::add_position(const Board& board) { repetition_table.add(board_hash::hash(board)); }

int Engine::evaluate_board(const Board& board) {
  int score = 0;
  const Player& cur_player = board.cur_player();
  const Player& opp_player = board.opp_player();
  for (Piece piece : {Piece::Bishop, Piece::Knight, Piece::Pawn, Piece::Queen, Piece::Rook}) {
    score += (bitboard::count(cur_player[piece]) - bitboard::count(opp_player[piece])) *
             evaluation::piece[static_cast<int>(piece)];
    u64 cur_piece_bitboard = cur_player[piece];
    BITBOARD_ITERATE(cur_piece_bitboard, bit) { score += pst::value_of(piece, bit, board.is_white_to_move()); }
    u64 opp_piece_bitboard = opp_player[piece];
    BITBOARD_ITERATE(opp_piece_bitboard, bit) { score -= pst::value_of(piece, bit, !board.is_white_to_move()); }
  }

  return score;
}

int Engine::evaluate_move_priority(const Move& move, int depth_left, const Move& refutation) {
  int priority = 0;
  if (move.is_capture()) {
    // MVV LVA priority.
    priority += move_priority::capture;
    priority +=
        move_priority::mvv_lva[static_cast<size_t>(move.get_captured_piece())][static_cast<size_t>(move.get_piece())];
  } else {
    for (size_t i = 0; i < config::killer_move_count; i++) {
      // Killer move priority.
      if (killer_moves.get(depth_left, i) == move) {
        priority += move_priority::killer - move_priority::killer_index * i;
        break;
      }
    }
  }
  if (move == refutation) priority += move_priority::refutation;
  return priority;
}

int Engine::evaluate_quiescence_move_priority(const Move& move) {
  int priority = 0;
  if (move.is_capture()) {
    // MVV LVA priority.
    priority += move_priority::capture;
    priority +=
        move_priority::mvv_lva[static_cast<size_t>(move.get_captured_piece())][static_cast<size_t>(move.get_piece())];
  }
  return priority;
}

int Engine::search(const Board& board, int alpha, int beta, int depth_left) {
  if (depth_left <= 0) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, 0);
  }
  normal_node_count++;

  bool is_in_check = board.is_in_check();
  if (!board.has_moves()) {
    if (is_in_check)
      return evaluation::losing - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
    else
      return evaluation::draw;  // Stalemate.
  }

  // Don't threefold.
  const u64 board_hash = board_hash::hash(board);
  if (repetition_table.is_draw_if_add(board_hash)) return evaluation::draw;

  // Check transposition table.
  Move refutation{};
  const PositionInfo& info = transposition_table.get(board_hash);
  if (info.hash == board_hash && info.depth_left >= depth_left) {
    // We have seen this position before and analyzed it to at least the same depth.
    if ((info.node_type == NodeType::PV || info.node_type == NodeType::Cut) && info.score >= beta) {
      return beta;
    }
    if ((info.node_type == NodeType::PV || info.node_type == NodeType::All) && info.score <= alpha) {
      return alpha;
    }
    refutation = info.best_move;
  } else if (info.hash == board_hash) {
    // We have seen this position before, but this time we must analyze it to a greater depth.
    refutation = info.best_move;
  }

  // Null move heuristic (https://www.chessprogramming.org/Null_Move_Pruning).
  if (!is_in_check && depth_left >= config::null_move_heuristic_R + 1 && beta < evaluation::winning) {
    Board new_board = board.skip_turn();
    int null_move_evaluation = -search(new_board, -beta, -alpha, depth_left - 1 - config::null_move_heuristic_R);
    if (null_move_evaluation >= beta) return beta;
  }

  MoveContainer moves = board.generate_moves();
  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (size_t i = 0; i < moves.size(); i++) {
    move_priorities[i] = evaluate_move_priority(moves[i], depth_left, refutation);
  }

  int evaluation = evaluation::min;
  Move best_move{};
  bool beta_cutoff = false;

  for (size_t i = 0; i < moves.size(); i++) {
    int best_priority = INT_MIN;
    size_t best_index = i;
    for (size_t j = i; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);
    Board new_board = board.apply_move(moves[i]);
    int new_board_evaluation = -search(new_board, -beta, -std::max(evaluation, alpha), depth_left - 1);
    if (new_board_evaluation >= beta) {
      beta_cutoff = true;
      best_move = moves[i];
      evaluation = beta;
      break;
    }
    if (new_board_evaluation > evaluation) {
      evaluation = new_board_evaluation;
      best_move = moves[i];
    }
  }

  NodeType node_type;
  if (beta_cutoff) {
    node_type = NodeType::Cut;
    if (!best_move.is_capture()) {
      // Add new killer move.
      killer_moves.add(best_move, depth_left);
    }
  } else if (evaluation < alpha) {
    node_type = NodeType::All;
  } else {
    node_type = NodeType::PV;
  }
  transposition_table.update(board_hash, depth_left, best_move, node_type, evaluation);
  return std::max(alpha, evaluation);
}

int Engine::quiescence_search(const Board& board, int alpha, int beta, int depth_left) {
  quiescence_node_count++;
  if (!board.has_moves()) {
    if (board.is_in_check())
      return evaluation::losing - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
    else
      return evaluation::draw;  // Stalemate.
  }

  bool is_in_check = board.is_in_check();
  if (!is_in_check && depth_left <= -config::quiescence_search_depth) return evaluate_board(board);

  if (!is_in_check) {
    int board_evaluation = evaluate_board(board);
    if (board_evaluation >= beta) return beta;
    alpha = std::max(alpha, board_evaluation);
  }

  auto generate_moves = [&]() {
    if (is_in_check) return board.generate_moves();
    if (depth_left > -config::quiescence_search_check_depth) return board.generate_quiescence_moves_and_checks();
    return board.generate_quiescence_moves();
  };
  MoveContainer moves = generate_moves();

  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (size_t i = 0; i < moves.size(); i++) {
    move_priorities[i] = evaluate_quiescence_move_priority(moves[i]);
  }

  for (size_t i = 0; i < moves.size(); i++) {
    int best_priority = INT_MIN;
    size_t best_index = i;
    for (size_t j = i; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);
    Board new_board = board.apply_move(moves[i]);
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, depth_left - 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}

void Engine::soft_reset() { killer_moves.clear(); }
