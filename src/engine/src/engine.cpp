#include "engine.h"

#include <limits.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "chess/move.h"
#include "chess/stack_repetition_tracker.h"
#include "config.h"
#include "evaluation.h"

Engine::Engine() : current_board{chess::Board::initial()} {}

Engine::Engine(const chess::Board& board) : current_board{board} {}

void Engine::set_position(const chess::Board& board) {
  // Reset all cached data.
  killer_moves = KillerMoves{};
  transposition_table = TranspositionTable{};
  history_heuristic = HistoryHeuristic{};
  repetition_tracker = chess::StackRepetitionTracker{};
  current_board = board;
  repetition_tracker.push(current_board);
}

void Engine::apply_move(const chess::Move& move) {
  current_board = current_board.apply_move(move);
  repetition_tracker.push(current_board, move);
}

Engine::MoveInfo Engine::choose_move(std::chrono::milliseconds search_time) {
  const std::scoped_lock search_lock{search_mutex};
  const auto start_time{std::chrono::steady_clock::now()};
  const auto cutoff_time{start_time + search_time};
  reset_search();
  chess::Move chosen_move{chess::Move::null()};
  SearchContext context{DebugInfo{}, false, time_point::max(), 1, std::nullopt};

  int current_depth{1};
  for (; current_depth < config::max_depth; current_depth++) {
    const chess::Move best_move{iterative_deepening(current_depth, context)};
    if (best_move.is_null()) break;
    chosen_move = best_move;
    context.cutoff_time = cutoff_time;  // Ensure that at least depth 1 is searched before cutoff.
  }

  const auto time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
  return MoveInfo{chosen_move, time_spent, current_depth - 1, context.debug};
}

Engine::MoveInfo Engine::choose_move(int search_depth) {
  const std::scoped_lock search_lock{search_mutex};
  const auto start_time{std::chrono::steady_clock::now()};
  reset_search();
  chess::Move chosen_move{chess::Move::null()};
  SearchContext context{DebugInfo{}, false, time_point::max(), 1, std::nullopt};

  for (int current_depth{1}; current_depth <= search_depth; current_depth++) {
    const chess::Move best_move{iterative_deepening(current_depth, context)};
    chosen_move = best_move;
  }

  const auto time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
  return MoveInfo{chosen_move, time_spent, search_depth, context.debug};
}

std::shared_ptr<Engine::SearchControl> Engine::cancellable_search(engine::uci::SearchConfig config) {
  //! TODO: support time controls (wtime, btime, winc, binc).
  //! TODO: support nodes <x>
  //! TODO: support infinite
  //! TODO: support searchmoves
  const auto control{std::make_shared<Engine::SearchControl>()};
  const auto search{[this](std::shared_ptr<Engine::SearchControl> control, engine::uci::SearchConfig config) {
    const std::scoped_lock search_lock{search_mutex};
    const auto start_time{std::chrono::steady_clock::now()};

    //! TODO: What is a good way to get an accurate safety_margin that minimizes lost time?
    const auto safety_margin{[](const auto& movetime) { return movetime - std::chrono::milliseconds{100}; }};
    const auto add_to_start_time{[&start_time](const auto& movetime) { return start_time + movetime; }};
    const auto cutoff_time{config.movetime.transform(safety_margin)
                               .transform(add_to_start_time)
                               .value_or(std::chrono::steady_clock::time_point::max())};

    SearchContext context{DebugInfo{}, false, cutoff_time, 1, control.get()};
    chess::Move chosen_move{[this]() {
      auto moves{current_board.generate_moves()};
      if (moves.empty()) return chess::Move::null();
      return moves[0];
    }()};
    const int32_t search_depth{config.depth.value_or(config::max_depth)};
    reset_search();

    int32_t current_depth{1};
    for (; current_depth <= search_depth; current_depth++) {
      const chess::Move best_move{iterative_deepening(current_depth, context)};
      if (best_move.is_null()) break;
      chosen_move = best_move;
    }

    control->set_move(chosen_move);
  }};

  control->run(std::thread{search, control, std::move(config)});
  return control;
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

std::pair<int, chess::Move> Engine::search(const chess::Board& board, int alpha, int beta, int depth_left,
                                           SearchContext& context) {
  if (depth_left <= 0) {
    // Switch to quiescence search
    return {quiescence_search(board, alpha, beta, 0, context), chess::Move::null()};
  }

  context.debug.normal_node_count++;
  if (context.should_stop()) {
    return {0, chess::Move::null()};
  }

  if (const auto score{board.get_score(repetition_tracker)}) {
    if (*score == 0) return {evaluation::draw, chess::Move::null()};
    else return {evaluation::losing - depth_left, chess::Move::null()};
  }

  const chess::Board::Hash board_hash = board.get_hash();
  NodeType node_type{NodeType::All};  // Assume all-node unless a good enough move is found.
  chess::Move best_move{};

  // Check transposition table.
  chess::Move hash_move{};
  if (const PositionInfo * info{transposition_table.get(board_hash)}; info && depth_left < context.root_depth) {
    if (info->depth_left >= depth_left) {
      // We have seen this position before and analyzed it to at least the same depth.
      context.debug.transposition_table_total++;

      const auto [score_lowerbound, score_upperbound] = info->get_score_bounds();

      // Check if the previous evaluation causes a cutoff here.
      const bool is_cutoff{score_lowerbound >= beta || score_upperbound <= alpha ||
                           score_lowerbound == score_upperbound};
      if (is_cutoff) {
        context.debug.transposition_table_success++;
        if (score_lowerbound >= beta) return {beta, chess::Move::null()};
        if (score_upperbound <= alpha) return {alpha, chess::Move::null()};
        const int score{std::max(alpha, std::min(beta, score_lowerbound))};  // Bound score to [alpha, beta].
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
  if (depth_left < context.root_depth && !is_in_check && depth_left >= config::null_move_heuristic_R + 1 &&
      beta < evaluation::winning && evaluate_board(board) >= beta) {
    context.debug.null_move_total++;
    chess::Board new_board{board.skip_turn()};
    int null_move_evaluation =
        -search(new_board, -beta, -beta + 1, depth_left - 1 - config::null_move_heuristic_R, context).first;
    if (null_move_evaluation >= beta) {
      context.debug.null_move_success++;
      return {beta, chess::Move::null()};
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

    // Futility pruning. If the expected value of this move does not raise the evaluation above alpha, then it is
    // likely not worth it to try it out.
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

    const chess::Board new_board{board.apply_move(moves[i])};
    repetition_tracker.push(new_board, moves[i]);
    //! TODO: Late Move Reduction was removed because it was pruning good lines and causing testcases to fail.
    //! Figure out how to implement it correctly.
    const int new_board_evaluation{-search(new_board, -beta, -alpha, depth_left - 1, context).first};
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

  transposition_table.try_update(board_hash, depth_left, best_move, node_type, alpha);

  return {alpha, best_move};
}

int Engine::quiescence_search(const chess::Board& board, int alpha, int beta, int depth_left, SearchContext& context) {
  context.debug.quiescence_node_count++;
  if (context.should_stop()) return 0;

  if (const auto score{board.get_score(repetition_tracker)}) {
    if (*score == 0) return evaluation::draw;
    return evaluation::losing - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
  }

  bool is_in_check{board.is_in_check()};
  if (!is_in_check && depth_left <= -config::quiescence_search_depth) return evaluate_board(board);

  int board_evaluation{evaluate_board(board)};
  if (!is_in_check) {
    if (board_evaluation >= beta) return beta;

    // Delta pruning. If the evaluation remains below alpha after capturing a queen, then the position's true
    // evaluation is likely below alpha.
    context.debug.q_delta_pruning_total++;
    if (board_evaluation + evaluation::piece[static_cast<size_t>(chess::PieceType::Queen)] +
            config::quiescence_search_delta_pruning_safety <
        alpha) {
      context.debug.q_delta_pruning_success++;
      return alpha;
    }

    alpha = std::max(alpha, board_evaluation);
  }

  chess::MoveContainer moves{[&board, is_in_check]() {
    if (is_in_check) return board.generate_moves();
    return board.generate_quiescence_moves();
  }()};

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
      context.debug.q_delta_pruning_total++;
      if (board_evaluation + evaluation::piece[static_cast<size_t>(moves[i].get_captured_piece())] +
              config::quiescence_search_delta_pruning_safety <
          alpha) {
        context.debug.q_delta_pruning_success++;
        continue;
      }
    }

    chess::Board new_board = board.apply_move(moves[i]);
    repetition_tracker.push(new_board, moves[i]);
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, depth_left - 1, context);
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

chess::Move Engine::iterative_deepening(int search_depth, SearchContext& context) {
  reset_iteration();
  context.root_depth = search_depth;
  const auto [evaluation, best_move] = search(current_board, evaluation::min, evaluation::max, search_depth, context);
  if (context.should_stop()) return chess::Move::null();
  context.debug.evaluation = evaluation;
  return best_move;
}

bool Engine::SearchContext::should_stop() {
  if (stopped) return true;

  const int visited_nodes_count{debug.normal_node_count + debug.quiescence_node_count};

  // Only check timeout and control every config::timeout_check_interval nodes visited,
  // to reduce overhead of getting current time and reading an atomic.
  if (visited_nodes_count % config::timeout_check_interval) return false;

  const bool timed_out{std::chrono::steady_clock::now() >= cutoff_time};
  const bool control_stopped{control && (*control)->is_stopped()};
  if (timed_out || control_stopped) stopped = true;

  return stopped;
}

Engine::SearchControl::SearchControl() : stopped{false}, done{false}, best_move{chess::Move::null()} {}

bool Engine::SearchControl::is_stopped() const { return stopped.load(std::memory_order::acquire); }

void Engine::SearchControl::stop() { stopped.store(true, std::memory_order::release); }

void Engine::SearchControl::wait_for_done() const { done.wait(false, std::memory_order_acquire); }

void Engine::SearchControl::set_move(chess::Move move) {
  best_move = move;
  done.store(true, std::memory_order::release);
  done.notify_all();
}

void Engine::SearchControl::run(std::thread thread) { search_thread = std::move(thread); }

std::optional<chess::Move> Engine::SearchControl::try_get_move() const {
  if (done.load(std::memory_order::acquire)) {
    return std::make_optional(best_move);
  }
  return std::nullopt;
}

chess::Move Engine::SearchControl::wait_and_get_move() const {
  wait_for_done();
  return best_move;
}

Engine::SearchControl::~SearchControl() {
  if (search_thread.joinable()) search_thread.join();
}