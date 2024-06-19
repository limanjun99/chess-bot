#include "model.h"

#include "chess_game.h"

namespace model {

NetImpl::NetImpl() {
  conv1 = register_module("conv1", torch::nn::Conv2d(torch::nn::Conv2dOptions(input_channels, 6, 3).padding(1)));
  batch_norm1 = register_module("batch_norm1", torch::nn::BatchNorm2d(6));
  conv2 = register_module("conv2", torch::nn::Conv2d(torch::nn::Conv2dOptions(6, 4, 3).padding(1)));

  // policy_conv1 = register_module("policy_conv1", torch::nn::ConvTranspose2d(6, 16, 3));
  policy_conv1 = register_module("policy_conv1",
                                 torch::nn::ConvTranspose2d(torch::nn::ConvTranspose2dOptions(4, 16, 3).padding(1)));
  policy_batch_norm1 = register_module("policy_batch_norm1", torch::nn::BatchNorm2d(16));
  policy_conv2 = register_module("policy_conv2",
                                 torch::nn::ConvTranspose2d(torch::nn::ConvTranspose2dOptions(16, 68, 3).padding(1)));

  value_conv1 = register_module("value_conv1", torch::nn::Conv2d(4, 1, 3));
  value_flatten1 = register_module("value_flatten1", torch::nn::Flatten());
  value_linear1 = register_module("value_linear1", torch::nn::Linear(36, 1));
}

std::pair<torch::Tensor, torch::Tensor> NetImpl::forward(torch::Tensor x) {
  auto result = forward_batch(x.unsqueeze(0));
  return {result.first.squeeze(0), result.second.squeeze(0)};
}

std::pair<torch::Tensor, torch::Tensor> NetImpl::forward_batch(torch::Tensor x) {
  x = torch::relu(batch_norm1(conv1(x)));
  x = torch::relu(conv2(x));

  torch::Tensor policy_x = torch::relu(policy_batch_norm1(policy_conv1(x)));
  policy_x = torch::sigmoid(policy_conv2(policy_x));

  torch::Tensor value_x = torch::relu(value_conv1(x));
  value_x = value_linear1(value_flatten1(value_x));
  value_x = torch::sigmoid(value_x);

  return {policy_x, value_x};
}

std::pair<torch::Tensor, torch::Tensor> NetImpl::forward_state(const ChessGame::BoardState& board) {
  return forward(to_tensor(board));
}

torch::Tensor NetImpl::to_tensor(const ChessGame::BoardState& board) { return board.to_tensor(); }

}  // namespace model