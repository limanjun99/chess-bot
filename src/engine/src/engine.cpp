#include "engine.h"

#include <limits.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "chess/stack_repetition_tracker.h"
#include "config.h"
#include "evaluation.h"

Engine::Engine() : current_board{chess::Board::initial()} {}

Engine::Engine(const chess::Board& board) : current_board{board} {}

void Engine::set_position(const chess::Board& board) {
  *this = Engine{};
  current_board = board;
  repetition_tracker.push(current_board);
}

void Engine::apply_move(const chess::Move& move) {
  current_board = current_board.apply_move(move);
  repetition_tracker.push(current_board, move);
}

Engine::MoveInfo Engine::choose_move(std::chrono::milliseconds search_time) {
  const auto start_time{std::chrono::steady_clock::now()};
  const auto cutoff_time{start_time + search_time};
  reset_search();
  chess::Move chosen_move{chess::Move::null()};
  DebugInfo debug{};

  int current_depth{1};
  for (; current_depth < config::max_depth; current_depth++) {
    const chess::Move best_move{iterative_deepening(current_depth, chosen_move, debug, cutoff_time)};
    if (best_move.is_null()) break;
    chosen_move = best_move;
  }

  const auto time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
  return MoveInfo{chosen_move, time_spent, current_depth - 1, debug};
}

Engine::MoveInfo Engine::choose_move(int search_depth) {
  auto start_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
  reset_search();
  chess::Move chosen_move{chess::Move::null()};
  DebugInfo debug{};

  for (int current_depth{1}; current_depth < search_depth; current_depth++) {
    const chess::Move best_move{iterative_deepening(current_depth, chosen_move, debug)};
    chosen_move = best_move;
  }

  const auto time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
  return MoveInfo{chosen_move, time_spent, search_depth, debug};
}

int Engine::evaluate_board(const chess::Board& board) { return pst::evaluate(board); }

int Engine::evaluate_move_priority(const chess::Move& move, int depth_left, const chess::Move& hash_move,
                                   bool is_white) {
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

int Engine::evaluate_quiescence_move_priority(const chess::Move& move) {
  int priority = 0;
  if (move.is_capture()) {
    // MVV LVA priority.
    priority += move_priority::capture;
    priority +=
        move_priority::mvv_lva[static_cast<size_t>(move.get_captured_piece())][static_cast<size_t>(move.get_piece())];
  }
  return priority;
}

int Engine::search(const chess::Board& board, int alpha, int beta, int depth_left, DebugInfo& debug, bool& timeout_flag,
                   time_point cutoff_time) {
  if (depth_left <= 0) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, 0, debug);
  }

  debug.normal_node_count++;
  if (debug.normal_node_count % config::timeout_check_interval == 0 && std::chrono::steady_clock::now() > cutoff_time) {
    timeout_flag = true;
    return 0;
  }

  if (const auto score{board.get_score(repetition_tracker)}) {
    if (*score == 0) return evaluation::draw;
    else return evaluation::losing - depth_left;
  }

  const chess::Board::Hash board_hash = board.get_hash();
  NodeType node_type{NodeType::All};  // Assume all-node unless a good enough move is found.
  chess::Move best_move{};

  // Check transposition table.
  chess::Move hash_move{};
  const PositionInfo& info = transposition_table.get(board_hash);
  if (info.hash == board_hash && info.depth_left >= depth_left) {
    debug.transposition_table_total++;
    // We have seen this position before and analyzed it to at least the same depth.
    if (info.node_type == NodeType::PV) {
      debug.transposition_table_success++;
      return std::max(alpha, std::min(beta, static_cast<int>(info.score)));
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
    hash_move = info.best_move;
  } else if (info.hash == board_hash) {
    // We have seen this position before, but this time we must analyze it to a greater depth.
    hash_move = info.best_move;
  }

  // Null move heuristic (https://www.chessprogramming.org/Null_Move_Pruning).
  // We check whether a null move causes beta cutoff when the following condtions are met:
  // 1. Current player is not in check.
  // 2. There is at least R depth left.
  // 3. Beta is not completely winning.
  // 4. Static evalution of current position is >= beta.
  const bool is_in_check{board.is_in_check()};
  if (!is_in_check && depth_left >= config::null_move_heuristic_R + 1 && beta < evaluation::winning &&
      evaluate_board(board) >= beta) {
    debug.null_move_total++;
    chess::Board new_board = board.skip_turn();
    int null_move_evaluation = -search(new_board, -beta, -beta + 1, depth_left - 1 - config::null_move_heuristic_R,
                                       debug, timeout_flag, cutoff_time);
    if (null_move_evaluation >= beta) {
      debug.null_move_success++;
      return beta;
    }
  }

  chess::MoveContainer moves = board.generate_moves();
  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (const auto& move : moves) {
    move_priorities.push_back(evaluate_move_priority(move, depth_left, hash_move, board.is_white_to_move()));
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
      if (moves[i].get_captured_piece() != chess::PieceType::None) {
        move_value_estimate += evaluation::piece[static_cast<int>(moves[i].get_captured_piece())];
      }
      if (moves[i].get_promotion_piece() != chess::PieceType::None) {
        move_value_estimate += evaluation::piece[static_cast<int>(moves[i].get_promotion_piece())];
      }
      if (cur_board_evaluation + move_value_estimate + config::futility_margin <= alpha) {
        continue;
      }
    }

    chess::Board new_board = board.apply_move(moves[i]);
    repetition_tracker.push(new_board, moves[i]);
    bool is_late_move =
        depth_left >= 2 && !moves[i].is_capture() && !moves[i].is_promotion() && !new_board.is_in_check();
    late_moves_count += is_late_move;
    int late_move_reduction = is_late_move && late_moves_count > 3 ? depth_left / 3 : 0;
    int new_board_evaluation;
    if (late_move_reduction) {
      new_board_evaluation = -search(new_board, -alpha - 1, -alpha, depth_left - 1 - late_move_reduction, debug,
                                     timeout_flag, cutoff_time);
      if (new_board_evaluation > alpha) {
        // If a late move was found to be good, we should recompute it with full depth.
        new_board_evaluation = -search(new_board, -beta, -alpha, depth_left - 1, debug, timeout_flag, cutoff_time);
      }
    } else {
      new_board_evaluation = -search(new_board, -beta, -alpha, depth_left - 1, debug, timeout_flag, cutoff_time);
    }
    repetition_tracker.pop();

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

int Engine::quiescence_search(const chess::Board& board, int alpha, int beta, int depth_left, DebugInfo& debug) {
  debug.quiescence_node_count++;

  if (const auto score{board.get_score(repetition_tracker)}) {
    if (*score == 0) return evaluation::draw;
    return evaluation::losing - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
  }

  bool is_in_check = board.is_in_check();
  if (!is_in_check && depth_left <= -config::quiescence_search_depth) return evaluate_board(board);

  int board_evaluation = evaluate_board(board);
  if (!is_in_check) {
    if (board_evaluation >= beta) return beta;

    // Delta pruning. If the evaluation remains below alpha after capturing a queen, then the position's true evaluation
    // is likely below alpha.
    debug.q_delta_pruning_total++;
    if (board_evaluation + evaluation::piece[static_cast<int>(chess::PieceType::Queen)] +
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
  chess::MoveContainer moves = generate_moves();

  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (const auto& move : moves) {
    move_priorities.push_back(evaluate_quiescence_move_priority(move));
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

    chess::Board new_board = board.apply_move(moves[i]);
    repetition_tracker.push(new_board, moves[i]);
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, depth_left - 1, debug);
    repetition_tracker.pop();
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}

void Engine::reset_iteration() {
  // Reset killer moves between each iteration of iterative deepening.
  killer_moves.clear();
}

void Engine::reset_search() {
  // Reset history heuristic between different boards.
  history_heuristic.clear();
}

chess::Move Engine::iterative_deepening(int search_depth, chess::Move candidate_best_move, DebugInfo& debug,
                                        time_point cutoff_time) {
  reset_iteration();

  chess::MoveContainer moves{current_board.generate_moves()};
  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (const auto& move : moves) {
    move_priorities.push_back(
        evaluate_move_priority(move, search_depth, candidate_best_move, current_board.is_white_to_move()));
  }
  int alpha{evaluation::min};
  chess::Move best_move{chess::Move::null()};
  bool timed_out{false};

  //! TODO: This should really just be a sort for clarity, since this function is not called often.
  for (size_t i{0}; i < moves.size(); i++) {
    // Swap the move with highest priority to index i.
    int best_priority{move_priorities[i]};
    size_t best_index{i};
    for (size_t j{i + 1}; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);

    const chess::Board new_board{current_board.apply_move(moves[i])};
    repetition_tracker.push(new_board, moves[i]);
    const int new_board_evaluation{
        search(new_board, -evaluation::max, -evaluation::min, search_depth - 1, debug, timed_out, cutoff_time)};
    repetition_tracker.pop();
    if (-new_board_evaluation > alpha) {
      alpha = -new_board_evaluation;
      best_move = moves[i];
    }

    if (timed_out) {
      return chess::Move::null();
    }
  }

  debug.evaluation = alpha;
  return best_move;
}