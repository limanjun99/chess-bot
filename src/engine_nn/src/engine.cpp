// #include "engine.h"

// #include "chess_game.h"
// #include "mcts.h"

// struct Engine::Impl {
//   Impl(std::convertible_to<Board> auto&& board, Color me)
//       : mcts{MCTS::MCTS<ChessGame::BoardState, ChessGame::MoveAction>{
//             ChessGame::BoardState{std::forward<decltype(board)>(board), me}}} {}

//   MCTS::MCTS<ChessGame::BoardState, ChessGame::MoveAction> mcts;
// };

// Engine::Engine(const Board& board, Color me) : impl{std::make_unique<Impl>(board, me)} {}
// Engine::Engine(Board&& board, Color me) : impl{std::make_unique<Impl>(std::move(board), me)} {}

// void Engine::apply_move(const Move& move) { impl->mcts.apply_action(ChessGame::MoveAction{move}); }

// Move Engine::choose_move() {
//   for (int iteration = 0; iteration < 10000; iteration++) {
//     impl->mcts.train();
//   }

//   return impl->mcts.get_best_action().get_move();
// }

// Engine::~Engine() = default;
