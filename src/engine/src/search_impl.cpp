#include "search_impl.h"

#include <chrono>
#include <mutex>

#include "config.h"
#include "evaluation.h"
#include "move_priority.h"
#include "time_management.h"
#include "uci.h"

engine::Search::Impl::Impl(chess::Board position_, chess::StackRepetitionTracker repetition_tracker_,
                           std::shared_ptr<Heuristics> heuristics_, engine::uci::SearchConfig config_)
    : starting_position{std::move(position_)},
      repetition_tracker{std::move(repetition_tracker_)},
      heuristics{std::move(heuristics_)},
      config{std::move(config_)},
      stop_signal{false},
      stopped{false},
      done{false},
      search_thread{},
      best_move{chess::Move::null()},
      debug_info{},
      root_depth{1},
      time_management{config, starting_position.get_color()} {
  //! TODO: support time controls (wtime, btime, winc, binc).
  //! TODO: support nodes <x>
  //! TODO: support searchmoves

  // Acquire lock on heuristics before starting thread.
  std::unique_lock search_lock{this->heuristics->mutex};

  //! TODO: What if acquiring the lock takes a long time due to another ongoing search?
  //! Ideally there should be some API to try_search() that returns immediately on failing to lock,
  //! or some mechanism in the Engine to prevent concurrent searches.

  // Set the best move to any move in case we timeout before searching.
  best_move = [&]() {
    if (auto moves{starting_position.generate_moves()}; !moves.empty()) return moves[0];
    return chess::Move::null();
  }();

  const auto go_wrapper{[this](std::unique_lock<std::mutex> search_lock) { go(std::move(search_lock)); }};
  search_thread = std::thread{go_wrapper, std::move(search_lock)};
}

engine::Search::Impl::~Impl() {
  if (!done.load(std::memory_order_acquire)) {
    stop();
    wait_for_done();
  }
  if (search_thread.joinable()) search_thread.join();
}

void engine::Search::Impl::stop() { stop_signal.store(true, std::memory_order::release); }

void engine::Search::Impl::wait_for_done() const { done.wait(false, std::memory_order_acquire); }

chess::Move engine::Search::Impl::get_move() const {
  wait_for_done();
  return best_move;
}

engine::Search::DebugInfo engine::Search::Impl::get_debug_info() const {
  wait_for_done();
  return debug_info;
}

std::shared_ptr<engine::Search> engine::Search::Impl::to_search(std::unique_ptr<Impl> impl) {
  return std::make_shared<engine::Search>(std::move(impl));
}

void engine::Search::Impl::go(std::unique_lock<std::mutex>) {
  const int32_t max_search_depth{config.depth.value_or(config::max_depth)};
  while (root_depth <= max_search_depth) {
    chess::Move found_move{iterative_deepening()};
    if (found_move.is_null()) {
      debug_info.timed_out = true;
      break;
    }
    debug_info.search_depth = root_depth;
    best_move = std::move(found_move);
    root_depth++;
    if (!time_management.can_continue_iteration()) {
      if (root_depth <= max_search_depth) debug_info.timed_out = true;
      break;
    }
  }
  debug_info.time_spent = time_management.time_spent();
  done.store(true, std::memory_order::release);
  done.notify_all();
}

chess::Move engine::Search::Impl::iterative_deepening() {
  reset_iteration();
  const auto [evaluation, best_move] = search(starting_position, Evaluation::min, Evaluation::max, root_depth);
  if (should_stop()) return chess::Move::null();
  debug_info.evaluation = evaluation.to_centipawns();
  return best_move;
}

std::pair<Evaluation, chess::Move> engine::Search::Impl::search(const chess::Board& board, Evaluation alpha,
                                                                Evaluation beta, int32_t depth_left) {
  if (depth_left <= 0) {
    // Switch to quiescence search
    return {quiescence_search(board, alpha, beta, 0), chess::Move::null()};
  }

  debug_info.normal_node_count++;
  if (should_stop()) {
    return {Evaluation::draw, chess::Move::null()};
  }

  if (const auto score{board.get_score(repetition_tracker)}) {
    if (*score == 0) return {Evaluation::draw, chess::Move::null()};
    else return {Evaluation::losing(depth_left), chess::Move::null()};
  }

  const chess::Board::Hash board_hash = board.get_hash();
  NodeType node_type{NodeType::All};  // Assume all-node unless a good enough move is found.
  chess::Move best_move{};

  // Check transposition table.
  chess::Move hash_move{};
  if (const PositionInfo * info{heuristics->transposition_table.get(board_hash)}; info && depth_left < root_depth) {
    if (info->depth_left >= depth_left) {
      // We have seen this position before and analyzed it to at least the same depth.
      debug_info.transposition_table_total++;

      const auto [score_lowerbound, score_upperbound] = info->get_score_bounds();

      // Check if the previous evaluation causes a cutoff here.
      const bool is_cutoff{score_lowerbound >= beta || score_upperbound <= alpha ||
                           score_lowerbound == score_upperbound};
      if (is_cutoff) {
        debug_info.transposition_table_success++;
        if (score_lowerbound >= beta) return {beta, chess::Move::null()};
        if (score_upperbound <= alpha) return {alpha, chess::Move::null()};
        const Evaluation score{std::max(alpha, std::min(beta, score_lowerbound))};  // Bound score to [alpha, beta].
        return {score, chess::Move::null()};
      }

      if (score_lowerbound >= alpha) {
        alpha = score_lowerbound;
        node_type = NodeType::PV;
      }
    }

    hash_move = info->best_move;
  }

  // Null move heuristic (https://www.chessprogramming.org/Null_Move_Pruning).
  // We check whether a null move causes beta cutoff when the following condtions are met:
  // 1. Current player is not in check.
  // 2. There is at least R depth left.
  // 3. Beta is not completely winning.
  // 4. Static evalution of current position is >= beta.
  const bool is_in_check{board.is_in_check()};
  if (depth_left < root_depth && !is_in_check && depth_left >= config::null_move_heuristic_R + 1 &&
      !beta.is_winning() && Evaluation::evaluate(board) >= beta) {
    debug_info.null_move_total++;
    chess::Board new_board{board.skip_turn()};
    Evaluation null_move_evaluation =
        -search(new_board, -beta, (-beta).succ(), depth_left - 1 - config::null_move_heuristic_R).first;
    if (null_move_evaluation >= beta) {
      debug_info.null_move_success++;
      return {beta, chess::Move::null()};
    }
  }

  chess::MoveContainer moves = board.generate_moves();
  std::vector<MovePriority> move_priorities;
  move_priorities.reserve(moves.size());
  for (const auto& move : moves) {
    move_priorities.push_back(MovePriority::evaluate(move, depth_left, hash_move, board.get_color(), *heuristics));
  }

  // Only used for futility pruning.
  Evaluation cur_board_evaluation{};
  if (depth_left == 1) cur_board_evaluation = Evaluation::evaluate(board);

  for (size_t i = 0; i < moves.size(); i++) {
    MovePriority best_priority = move_priorities[i];
    size_t best_index = i;
    for (size_t j = i + 1; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);

    // Futility pruning. If the expected value of this move does not raise the evaluation above alpha, then it is
    // likely not worth it to try it out.
    if (depth_left == 1 && !is_in_check && !beta.is_winning() && !alpha.is_losing()) {
      Evaluation move_value_estimate{};
      if (moves[i].get_captured_piece() != chess::PieceType::None) {
        move_value_estimate += Evaluation::piece[static_cast<size_t>(moves[i].get_captured_piece())];
      }
      if (moves[i].get_promotion_piece() != chess::PieceType::None) {
        move_value_estimate += Evaluation::piece[static_cast<size_t>(moves[i].get_promotion_piece())];
      }
      if (cur_board_evaluation + move_value_estimate + config::futility_margin <= alpha) {
        continue;
      }
    }

    const chess::Board new_board{board.apply_move(moves[i])};
    repetition_tracker.push(new_board, moves[i]);
    //! TODO: Late Move Reduction was removed because it was pruning good lines and causing testcases to fail.
    //! Figure out how to implement it correctly.
    const Evaluation new_board_evaluation{-search(new_board, -beta, -alpha, depth_left - 1).first};
    repetition_tracker.pop();

    if (new_board_evaluation >= beta) {
      alpha = beta;
      best_move = moves[i];
      node_type = NodeType::Cut;
      heuristics->history_heuristic.add_move_success(board.is_white_to_move(), moves[i].get_from(), moves[i].get_to());
      break;
    }
    heuristics->history_heuristic.add_move_failure(board.is_white_to_move(), moves[i].get_from(), moves[i].get_to());
    if (new_board_evaluation > alpha) {
      alpha = new_board_evaluation;
      best_move = moves[i];
      node_type = NodeType::PV;
    }
  }

  if (node_type == NodeType::Cut && !best_move.is_capture()) {
    // Add new killer move if beta-cutoff caused by non-capture.
    heuristics->killer_moves.add(best_move, depth_left);
  }

  heuristics->transposition_table.try_update(board_hash, depth_left, best_move, node_type, alpha);

  return {alpha, best_move};
}

Evaluation engine::Search::Impl::quiescence_search(const chess::Board& board, Evaluation alpha, Evaluation beta,
                                                   int32_t depth_left) {
  debug_info.quiescence_node_count++;
  if (should_stop()) return Evaluation::draw;

  if (const auto score{board.get_score(repetition_tracker)}) {
    if (*score == 0) return Evaluation::draw;
    return Evaluation::losing(depth_left);  // Checkmate, minus depth_left so that shorter mates are preferred.
  }

  bool is_in_check{board.is_in_check()};
  if (!is_in_check && depth_left <= -config::quiescence_search_depth) return Evaluation::evaluate(board);

  const Evaluation board_evaluation{Evaluation::evaluate(board)};
  if (!is_in_check) {
    if (board_evaluation >= beta) return beta;

    // Delta pruning. If the evaluation remains below alpha after capturing a queen, then the position's true
    // evaluation is likely below alpha.
    debug_info.q_delta_pruning_total++;
    if (board_evaluation + Evaluation::piece[static_cast<size_t>(chess::PieceType::Queen)] +
            config::quiescence_search_delta_pruning_safety <
        alpha) {
      debug_info.q_delta_pruning_success++;
      return alpha;
    }

    alpha = std::max(alpha, board_evaluation);
  }

  chess::MoveContainer moves{[&board, is_in_check]() {
    if (is_in_check) return board.generate_moves();
    return board.generate_quiescence_moves();
  }()};

  std::vector<MovePriority> move_priorities;
  move_priorities.reserve(moves.size());
  for (const auto& move : moves) {
    move_priorities.push_back(MovePriority::evaluate_quiescence(move));
  }

  for (size_t i = 0; i < moves.size(); i++) {
    MovePriority best_priority = move_priorities[i];
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
      debug_info.q_delta_pruning_total++;
      const auto best_improvement =
          Evaluation::piece[static_cast<size_t>(moves[i].get_captured_piece())] +
          (moves[i].is_promotion() ? Evaluation::piece[static_cast<size_t>(moves[i].get_promotion_piece())]
                                   : Evaluation{0}) +
          config::quiescence_search_delta_pruning_safety;
      if (board_evaluation + best_improvement < alpha) {
        debug_info.q_delta_pruning_success++;
        continue;
      }
    }

    chess::Board new_board = board.apply_move(moves[i]);
    repetition_tracker.push(new_board, moves[i]);
    Evaluation new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, depth_left - 1);
    repetition_tracker.pop();
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}

void engine::Search::Impl::reset_iteration() {
  // Reset killer moves between each iteration of iterative deepening.
  heuristics->killer_moves.clear();
}

bool engine::Search::Impl::should_stop() {
  if (stopped) return true;

  const int64_t visited_nodes_count{debug_info.normal_node_count + debug_info.quiescence_node_count};
  if (visited_nodes_count & (time_management.check_interval() - 1)) return false;

  const bool timed_out{time_management.has_timed_out()};
  const bool stop_signalled{stop_signal.load(std::memory_order::acquire)};
  if (timed_out || stop_signalled) stopped = true;

  return stopped;
}