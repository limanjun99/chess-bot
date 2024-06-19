#pragma once

#include "chess_game.h"

namespace model {

constexpr int input_height{8};
constexpr int input_width{8};
constexpr int input_channels{15};

struct NetImpl : torch::nn::Module {
  NetImpl();

  std::pair<torch::Tensor, torch::Tensor> forward(torch::Tensor x);

  std::pair<torch::Tensor, torch::Tensor> forward_batch(torch::Tensor x);

  std::pair<torch::Tensor, torch::Tensor> forward_state(const ChessGame::BoardState& board);

  torch::Tensor to_tensor(const ChessGame::BoardState& board);

  torch::nn::Conv2d conv1{nullptr}, conv2{nullptr}, value_conv1{nullptr};
  torch::nn::ConvTranspose2d policy_conv1{nullptr}, policy_conv2{nullptr};
  torch::nn::BatchNorm2d batch_norm1{nullptr}, policy_batch_norm1{nullptr};
  torch::nn::Linear value_linear1{nullptr};
  torch::nn::Flatten value_flatten1{nullptr};
};

TORCH_MODULE(Net);

}  // namespace model