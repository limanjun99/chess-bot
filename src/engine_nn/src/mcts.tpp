#pragma once

#include "mcts.h"

namespace MCTS {

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline MCTS<State, Action, Net>::Node::Node(std::convertible_to<State> auto &&state, Config &config, float prior,
                                            Node *parent)
    : state{std::forward<decltype(state)>(state)},
      config{config},
      parent{parent},
      visit_count{0},
      score{0},
      prior{prior} {}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline float MCTS<State, Action, Net>::Node::uct(int32_t parent_visit_count) const {
  const float exploitation{visit_count == 0 ? 0 : static_cast<float>(score) / visit_count};
  const float exploration{config.C * sqrtf(parent_visit_count) / (visit_count + 1) * prior};
  return exploitation + exploration;
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline MCTS<State, Action, Net>::Node *MCTS<State, Action, Net>::Node::select() {
  Node *selected_child{nullptr};
  float max_uct{std::numeric_limits<float>::lowest()};
  for (const auto &[child, action] : children) {
    float child_uct{child->uct(visit_count)};
    if (child_uct > max_uct) {
      selected_child = child.get();
      max_uct = child_uct;
    }
  }
  if (selected_child == nullptr) return this;
  return selected_child->select();
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline MCTS<State, Action, Net>::Node *MCTS<State, Action, Net>::Node::expand(torch::Tensor policy) {
  if (state.is_terminal()) return this;

  const std::vector<std::pair<State, Action>> transitions = state.get_transitions();
  float total_density{0};
  for (const auto &[_, action] : transitions) total_density += action.get_density(policy);

  for (const auto &[child_state, action] : transitions) {
    const float prior{action.get_density(policy) / total_density};
    auto child_node = std::make_unique<Node>(child_state, config, prior, this);
    children.emplace_back(std::move(child_node), action);
  }
  return select();
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline void MCTS<State, Action, Net>::Node::backprop(float result) {
  visit_count++;
  score += result;
  if (parent != nullptr) {
    parent->backprop(1 - result);
  }
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline Action MCTS<State, Action, Net>::Node::get_best_action() const {
  std::optional<Action> best_action{std::nullopt};
  float max_uct{std::numeric_limits<float>::lowest()};
  for (const auto &[child, action] : children) {
    float child_uct{child->uct(visit_count)};
    if (child_uct > max_uct) {
      best_action = std::optional{action};
      max_uct = child_uct;
    }
  }
  return *best_action;
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline MCTS<State, Action, Net>::Node *MCTS<State, Action, Net>::Node::apply_action(const Action &action) {
  for (auto &[child, child_action] : children) {
    if (child_action == action) {
      child->parent = nullptr;
      return child.release();
    }
  }
  return nullptr;
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline bool MCTS<State, Action, Net>::Node::is_expanded() const {
  if (state.is_terminal()) return true;
  return !children.empty();
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline const State &MCTS<State, Action, Net>::Node::get_state() const {
  return state;
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline std::vector<std::pair<Action, int32_t>> MCTS<State, Action, Net>::Node::get_action_visits() const {
  std::vector<std::pair<Action, int32_t>> action_visits;
  action_visits.reserve(children.size());
  for (const auto &[child, action] : children) {
    action_visits.emplace_back(action, child->visit_count);
  }
  return action_visits;
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline MCTS<State, Action, Net>::MCTS(std::convertible_to<State> auto &&state, Net net, Config config)
    : root_node{std::make_unique<Node>(state, config)}, net{net}, config{config} {}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
void MCTS<State, Action, Net>::rollout() {
  // 1. Selection.
  Node *selected_node{root_node->select()};

  // 2. Expansion.
  auto [policy, value] = net->forward_state(selected_node->get_state());
  if (const auto score{selected_node->get_state().get_player_score()}) {
    value = torch::full({1}, *score);
  } else {
    selected_node->expand(policy);
  }

  // 3. Backpropagate the result.
  selected_node->backprop(1 - value.template item<float>());
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline void MCTS<State, Action, Net>::apply_action(const Action &action) {
  if (!root_node->is_expanded()) {
    const auto [policy, _] = net->forward_state(root_node->get_state());
    root_node->expand(policy);
  }
  Node *new_root_node{root_node->apply_action(action)};
  root_node = std::unique_ptr<Node>{new_root_node};
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline Action MCTS<State, Action, Net>::get_best_action() const {
  return root_node->get_best_action();
}

template <typename State, typename Action, typename Net>
  requires IsMCTS<State, Action, Net>
inline std::vector<std::pair<Action, int32_t>> MCTS<State, Action, Net>::get_action_visits() const {
  return root_node->get_action_visits();
}

}  // namespace MCTS