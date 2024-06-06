#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

namespace MCTS {

template <typename State, typename Action>
concept StateAction = requires(State state, Action action) {
                        { state.is_terminal() } -> std::same_as<bool>;
                        { state.get_transitions() } -> std::same_as<std::vector<std::pair<State, Action>>>;
                        {
                          state.get_random_action(std::declval<std::mt19937 &>())
                        } -> std::same_as<std::optional<Action>>;
                        { state.apply_action(std::declval<const Action &>()) } -> std::same_as<State>;
                        { state.get_score() } -> std::same_as<float>;
                        { std::declval<Action>() == std::declval<Action>() } -> std::same_as<bool>;
                      };

struct Config {
  float C{std::sqrtf(2)};
};

template <typename State, typename Action>
  requires StateAction<State, Action>
class MCTS {
public:
  MCTS(std::convertible_to<State> auto &&state, Config config = Config{});
  void train();
  Action get_best_action() const;
  void apply_action(const Action &action);

private:
  class Node {
  public:
    Node(std::convertible_to<State> auto &&state, Config &config, Node *parent = nullptr);
    float uct(int32_t parent_visit_count) const;
    Node *select();
    Node *expand();
    float simulate() const;
    void backprop(float result);
    Action get_best_action() const;
    Node *apply_action(const Action &action);

  private:
    State state;
    Config &config;
    Node *parent;
    int32_t visit_count;
    float score;
    std::vector<std::pair<std::unique_ptr<Node>, Action>> children;
  };

  std::unique_ptr<Node> root_node;
  Config config;

  MCTS(std::unique_ptr<Node> node);
};

}  // namespace MCTS

#include "mcts.tpp"