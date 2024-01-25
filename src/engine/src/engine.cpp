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
  auto start_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
  cutoff_time =
      start_time + std::chrono::duration_cast<std::chrono::milliseconds>(search_time * config::search_time_lowerbound);
  search_timeout = false;
  reset_debug();
  history_heuristic.clear();
  int search_depth = 0;
  MoveContainer moves = board.generate_moves();
  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  Move chosen_move{};

  // Iterative deepening.
  while (search_depth < config::max_depth) {
    soft_reset();
    int alpha = evaluation::min;
    Move best_move{};
    bool finished_evaluation = true;
    for (size_t i = 0; i < move_priorities.size(); i++) {
      move_priorities[i] = evaluate_move_priority(moves[i], search_depth, chosen_move, board.is_white_to_move());
    }
    for (size_t i = 0; i < moves.size(); i++) {
      int best_priority = move_priorities[i];
      size_t best_index = i;
      for (size_t j = i + 1; j < moves.size(); j++) {
        if (move_priorities[j] > best_priority) {
          best_priority = move_priorities[j];
          best_index = j;
        }
      }
      std::swap(moves[i], moves[best_index]);
      std::swap(move_priorities[i], move_priorities[best_index]);
      Board new_board = board.apply_move(moves[i]);
      int new_board_evaluation = search<true>(new_board, -evaluation::max, -alpha, search_depth);
      if (-new_board_evaluation > alpha) {
        alpha = -new_board_evaluation;
        best_move = moves[i];
      }
      if (search_timeout) {
        finished_evaluation = false;
        break;
      }
    }

    if (finished_evaluation) chosen_move = best_move;
    if (is_out_of_time()) break;
    search_depth++;
  }

  auto end_time = std::chrono::steady_clock::now();
  auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  return {chosen_move, time_spent, search_depth, debug};
}

Engine::MoveInfo Engine::choose_move(const Board& board, int depth) {
  auto start_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
  cutoff_time = start_time + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::years(1));
  search_timeout = false;
  reset_debug();
  history_heuristic.clear();
  int search_depth = 0;
  MoveContainer moves = board.generate_moves();
  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  Move chosen_move{};

  // Iterative deepening.
  while (search_depth <= depth) {
    soft_reset();
    int alpha = evaluation::min;
    Move best_move{};
    for (size_t i = 0; i < move_priorities.size(); i++) {
      move_priorities[i] = evaluate_move_priority(moves[i], search_depth, chosen_move, board.is_white_to_move());
    }
    for (size_t i = 0; i < moves.size(); i++) {
      int best_priority = move_priorities[i];
      size_t best_index = i;
      for (size_t j = i + 1; j < moves.size(); j++) {
        if (move_priorities[j] > best_priority) {
          best_priority = move_priorities[j];
          best_index = j;
        }
      }
      std::swap(moves[i], moves[best_index]);
      std::swap(move_priorities[i], move_priorities[best_index]);
      Board new_board = board.apply_move(moves[i]);
      int new_board_evaluation = search<false>(new_board, -evaluation::max, -alpha, search_depth);
      if (-new_board_evaluation > alpha) {
        alpha = -new_board_evaluation;
        best_move = moves[i];
      }
    }
    if (search_depth == depth) chosen_move = best_move;
    search_depth++;
  }

  auto end_time = std::chrono::steady_clock::now();
  auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  return {chosen_move, time_spent, search_depth, debug};
}

void Engine::add_position(const Board& board) { repetition_table.add(board_hash::hash(board)); }

int Engine::evaluate_board(const Board& board) {
  int score = 0;
  const Player& cur_player = board.cur_player();
  const Player& opp_player = board.opp_player();
  for (Piece piece : {Piece::Bishop, Piece::Knight, Piece::Pawn, Piece::Queen, Piece::Rook}) {
    score += (bitboard::count(cur_player[piece]) - bitboard::count(opp_player[piece])) *
             evaluation::piece[static_cast<int>(piece)];
  }
  u64 cur_pawn_bitboard = cur_player[Piece::Pawn];
  BITBOARD_ITERATE(cur_pawn_bitboard, bit) { score += pst::value_of(Piece::Pawn, bit, board.is_white_to_move()); }
  u64 opp_piece_bitboard = opp_player[Piece::Pawn];
  BITBOARD_ITERATE(opp_piece_bitboard, bit) { score -= pst::value_of(Piece::Pawn, bit, !board.is_white_to_move()); }

  return score;
}

int Engine::evaluate_move_priority(const Move& move, int depth_left, const Move& hash_move, bool is_white) {
  if (move == hash_move) return move_priority::hash_move;

  int priority = 0;

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
    for (size_t i = 0; i < config::killer_move_count; i++) {
      // Killer move priority.
      if (killer_moves.get(depth_left, i) == move) {
        priority += move_priority::killer - move_priority::killer_index * i;
        is_killer = true;
        break;
      }
    }

    if (!is_killer) {
      priority += history_heuristic.get_score(is_white, move.get_from(), move.get_to());
    }
  }

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

template <bool IsTimed>
int Engine::search(const Board& board, int alpha, int beta, int depth_left) {
  if (depth_left <= 0) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, 0);
  }

  debug.normal_node_count++;
  if constexpr (IsTimed) {
    if (debug.normal_node_count % config::timeout_check_interval == 0 && is_out_of_time()) {
      search_timeout = true;
    }
    if (search_timeout) return 0;
  }

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

  NodeType node_type{NodeType::All};  // Assume all-node unless a good enough move is found.
  Move best_move{};

  // Check transposition table.
  Move hash_move{};
  const PositionInfo& info = transposition_table.get(board_hash);
  if (info.hash == board_hash && info.depth_left >= depth_left) {
    debug.transposition_table_total++;
    // We have seen this position before and analyzed it to at least the same depth.
    if (info.node_type == NodeType::PV) {
      debug.transposition_table_success++;
      return std::max(alpha, std::min(beta, info.score));
    } else if (info.node_type == NodeType::Cut) {
      if (info.score >= beta) {
        debug.transposition_table_success++;
        return beta;
      }
      if (info.score >= alpha) {
        alpha = info.score;
        node_type = NodeType::PV;
      }
    } else {
      if (info.score <= alpha) {
        debug.transposition_table_success++;
        return alpha;
      }
    }
    hash_move = info.best_move.to_move();
  } else if (info.hash == board_hash) {
    // We have seen this position before, but this time we must analyze it to a greater depth.
    hash_move = info.best_move.to_move();
  }

  // Null move heuristic (https://www.chessprogramming.org/Null_Move_Pruning).
  if (!is_in_check && depth_left >= config::null_move_heuristic_R + 1 && beta < evaluation::winning) {
    debug.null_move_total++;
    Board new_board = board.skip_turn();
    int null_move_evaluation =
        -search<IsTimed>(new_board, -beta, -beta + 1, depth_left - 1 - config::null_move_heuristic_R);
    if (null_move_evaluation >= beta) {
      debug.null_move_success++;
      return beta;
    }
  }

  MoveContainer moves = board.generate_moves();
  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (size_t i = 0; i < moves.size(); i++) {
    move_priorities.push_back(evaluate_move_priority(moves[i], depth_left, hash_move, board.is_white_to_move()));
  }

  // Only used for futility pruning.
  int cur_board_evaluation = 0;
  if (depth_left == 1) cur_board_evaluation = evaluate_board(board);

  int late_moves_count = 0;
  for (size_t i = 0; i < moves.size(); i++) {
    int best_priority = move_priorities[i];
    size_t best_index = i;
    for (size_t j = i + 1; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);

    // Futility pruning. If the expected value of this move does not raise the evaluation above alpha, then it is likely
    // not worth it to try it out.
    if (depth_left == 1 && !is_in_check && beta <= evaluation::winning / 2 && alpha >= evaluation::losing / 2) {
      int move_value_estimate = 0;
      if (moves[i].get_captured_piece() != Piece::None) {
        move_value_estimate += evaluation::piece[static_cast<int>(moves[i].get_captured_piece())];
      }
      if (moves[i].get_promotion_piece() != Piece::None) {
        move_value_estimate += evaluation::piece[static_cast<int>(moves[i].get_promotion_piece())];
      }
      if (cur_board_evaluation + move_value_estimate + config::futility_margin <= alpha) {
        continue;
      }
    }

    Board new_board = board.apply_move(moves[i]);
    bool is_late_move =
        depth_left >= 2 && !moves[i].is_capture() && !moves[i].is_promotion() && !new_board.is_in_check();
    late_moves_count += is_late_move;
    int late_move_reduction = is_late_move && late_moves_count > 3 ? depth_left / 3 : 0;
    int new_board_evaluation;
    if (late_move_reduction) {
      new_board_evaluation = -search<IsTimed>(new_board, -alpha - 1, -alpha, depth_left - 1 - late_move_reduction);
      if (new_board_evaluation > alpha) {
        // If a late move was found to be good, we should recompute it with full depth.
        new_board_evaluation = -search<IsTimed>(new_board, -beta, -alpha, depth_left - 1);
      }
    } else {
      new_board_evaluation = -search<IsTimed>(new_board, -beta, -alpha, depth_left - 1);
    }

    if (new_board_evaluation >= beta) {
      alpha = beta;
      best_move = moves[i];
      node_type = NodeType::Cut;
      history_heuristic.add_move_success(board.is_white_to_move(), moves[i].get_from(), moves[i].get_to());
      break;
    }
    history_heuristic.add_move_failure(board.is_white_to_move(), moves[i].get_from(), moves[i].get_to());
    if (new_board_evaluation > alpha) {
      alpha = new_board_evaluation;
      best_move = moves[i];
      node_type = NodeType::PV;
    }
  }

  if (node_type == NodeType::Cut && !best_move.is_capture()) {
    // Add new killer move if beta-cutoff caused by non-capture.
    killer_moves.add(best_move, depth_left);
  }

  if (info.hash != board_hash || depth_left >= info.depth_left) {
    // Update transposition table if this entry is new or has >= depth than current entry.
    transposition_table.update(board_hash, depth_left, best_move, node_type, alpha);
  }

  return alpha;
}

int Engine::quiescence_search(const Board& board, int alpha, int beta, int depth_left) {
  debug.quiescence_node_count++;
  if (!board.has_moves()) {
    if (board.is_in_check())
      return evaluation::losing - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
    else
      return evaluation::draw;  // Stalemate.
  }

  bool is_in_check = board.is_in_check();
  if (!is_in_check && depth_left <= -config::quiescence_search_depth) return evaluate_board(board);

  int board_evaluation = evaluate_board(board);
  if (!is_in_check) {
    if (board_evaluation >= beta) return beta;

    // Delta pruning. If the evaluation remains below alpha after capturing a queen, then the position's true evaluation
    // is likely below alpha.
    debug.q_delta_pruning_total++;
    if (board_evaluation + evaluation::piece[static_cast<int>(Piece::Queen)] +
            config::quiescence_search_delta_pruning_safety <
        alpha) {
      debug.q_delta_pruning_success++;
      return alpha;
    }

    alpha = std::max(alpha, board_evaluation);
  }

  auto generate_moves = [&]() {
    if (is_in_check) return board.generate_moves();
    return board.generate_quiescence_moves();
  };
  MoveContainer moves = generate_moves();

  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (size_t i = 0; i < moves.size(); i++) {
    move_priorities.push_back(evaluate_quiescence_move_priority(moves[i]));
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

    // Delta pruning. If capturing a piece (+ some safety value) does not raise evaluation above alpha, then there is
    // likely no point in checking this move at all.
    if (!is_in_check && moves[i].is_capture()) {
      debug.q_delta_pruning_total++;
      if (board_evaluation + evaluation::piece[static_cast<int>(moves[i].get_captured_piece())] +
              config::quiescence_search_delta_pruning_safety <
          alpha) {
        debug.q_delta_pruning_success++;
        continue;
      }
    }

    Board new_board = board.apply_move(moves[i]);
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, depth_left - 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}

void Engine::soft_reset() { killer_moves.clear(); }

void Engine::reset_debug() { debug = {0, 0, 0, 0, 0, 0, 0, 0}; }

bool Engine::is_out_of_time() {
  auto current_time = std::chrono::steady_clock::now();
  return current_time >= cutoff_time;
}
