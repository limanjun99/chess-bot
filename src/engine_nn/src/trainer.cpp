#include "trainer.h"

#include <torch/torch.h>

#include <random>
#include <ranges>

#include "chess_game.h"
#include "mcts.h"
#include "model.h"

namespace trainer {

void Trainer::train() {
  model::Net net{};
  std::mt19937 rng{0};
  torch::manual_seed(0);
  auto optimizer{torch::optim::Adam(net->parameters(), torch::optim::AdamOptions(1e-4))};

  constexpr int epochs{100};
  constexpr int self_play_iterations{8};
  constexpr int rollout_iterations{200};
  constexpr int max_batch_size{16};

  for (int epoch{1}; epoch <= epochs; epoch++) {
    std::cout << "Epoch " << epoch << '\n' << std::endl;
    // State, policy, value.
    std::vector<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>> train_dataset;

    for (int self_play_i{1}; self_play_i <= self_play_iterations; self_play_i++) {
      auto current_state{ChessGame::BoardState::initial()};
      std::vector<std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>> game_data;
      std::string game_pgn;

      while (!current_state.is_terminal()) {
        MCTS::MCTS<ChessGame::BoardState, ChessGame::MoveAction, model::Net> mcts{current_state, net};

        for (int rollout_i{1}; rollout_i <= rollout_iterations; rollout_i++) mcts.rollout();

        // Add to training data.
        const std::vector<std::pair<ChessGame::MoveAction, int32_t>> action_visits{mcts.get_action_visits()};
        const auto weights{action_visits |
                           std::views::transform([](const auto& action_visit) { return action_visit.second; })};
        const int32_t total_visits{std::accumulate(weights.begin(), weights.end(), 0)};
        torch::Tensor policy{torch::zeros({68, 8, 8})};
        for (const auto& [action, visit] : action_visits) {
          action.set_density(policy, static_cast<float>(visit) / total_visits);
        }
        game_data.emplace_back(current_state.to_tensor(), policy, torch::zeros({1}));

        // Choose a random action.
        std::discrete_distribution<size_t> dist{weights.begin(), weights.end()};
        const size_t index{dist(rng)};
        const auto action{action_visits[index].first};
        game_pgn += " " + action.get_move().to_algebraic(current_state.get_board());
        current_state = current_state.apply_action(action);
      }

      auto score{*current_state.get_player_score()};
      bool is_white_turn{true};
      for (auto& [state, policy, value] : game_data) {
        value[0] = is_white_turn ? score : 1 - score;
        is_white_turn = !is_white_turn;
      }
      for (const auto& data : game_data) train_dataset.push_back(data);

      if (score != 0.5) std::cout << "Not a draw: ";
      std::cout << game_pgn << '\n' << std::endl;
    }

    // Train.
    for (size_t begin{0}; begin < train_dataset.size(); begin += max_batch_size) {
      size_t end = std::min(begin + max_batch_size, train_dataset.size());
      int batch_size{static_cast<int>(end - begin)};

      torch::Tensor states{torch::zeros({batch_size, model::input_channels, model::input_height, model::input_width})};
      torch::Tensor policies_target{torch::zeros({batch_size, 68, model::input_height, model::input_width})};
      torch::Tensor values_target{torch::zeros({batch_size, 1})};
      for (size_t i{begin}; i < end; i++) {
        states[i - begin] = std::get<0>(train_dataset[i]);
        policies_target[i - begin] = std::get<1>(train_dataset[i]);
        values_target[i - begin] = std::get<2>(train_dataset[i]);
      }

      const auto [policies_pred, values_pred] = net->forward_batch(states);
      const auto policy_loss{torch::cross_entropy_loss(policies_pred, policies_target)};
      const auto value_loss{torch::mse_loss(values_pred, values_target)};
      const auto loss = policy_loss + value_loss;
      optimizer.zero_grad();
      loss.backward();
      optimizer.step();
    }
  }
}

}  // namespace trainer