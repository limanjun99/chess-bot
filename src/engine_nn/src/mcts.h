#pragma once

#include <torch/optim.h>
#include <torch/serialize.h>

#include <cmath>
#include <concepts>
#include <cstdint>
#include <memory>
#include <vector>

namespace MCTS {

template <typename State, typename Action, typename Net>
concept IsMCTS = requires(State state, Action action, Net net) {
                   { state.is_terminal() } -> std::convertible_to<bool>;

                   { state.get_transitions() } -> std::same_as<std::vector<std::pair<State, Action>>>;

                   { state.apply_action(action) } -> std::same_as<State>;

                   { state.get_player_score() } -> std::same_as<std::optional<float>>;

                   { action == action } -> std::same_as<bool>;

                   { action.get_density(std::declval<torch::Tensor>()) } -> std::convertible_to<float>;

                   { net->forward_state(state) } -> std::same_as<std::pair<torch::Tensor, torch::Tensor>>;
                 };

struct Config {
  float C{std::sqrtf(2)};
};

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
class MCTS {
public:
  MCTS(std::convertible_to<State> auto &&state, Net net, Config config = Config{});
  void rollout();
  Action get_best_action() const;
  std::vector<std::pair<Action, int32_t>> get_action_visits() const;
  const State &get_state() const;
  int32_t get_visit_count() const;
  void apply_action(const Action &action);

private:
  class Node {
  public:
    Node(std::convertible_to<State> auto &&state, Config &config, float prior = 0, Node *parent = nullptr);
    float uct(int32_t parent_visit_count) const;
    Node *select();
    Node *expand(torch::Tensor policy);
    void backprop(float result);
    Action get_best_action() const;
    Node *apply_action(const Action &action);
    bool is_expanded() const;
    const State &get_state() const;
    int32_t get_visit_count() const;
    std::vector<std::pair<Action, int32_t>> get_action_visits() const;

  private:
    State state;
    Config &config;
    Node *parent;
    int32_t visit_count;
    float score;
    float prior;
    std::vector<std::pair<std::unique_ptr<Node>, Action>> children;
  };

  Config config;
  std::unique_ptr<Node> root_node;
  Net net;

  MCTS(std::unique_ptr<Node> node);
};

}  // namespace MCTS

#include "mcts.tpp"