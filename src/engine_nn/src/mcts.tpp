#pragma once

#include "mcts.h"

namespace MCTS {

template <typename State, typename Action>
  requires StateAction<State, Action>
inline MCTS<State, Action>::Node::Node(std::convertible_to<State> auto&& state, Config& config, Node* parent)
    : state{std::forward<decltype(state)>(state)}, config{config}, parent{parent}, visit_count{0}, score{0} {}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline float MCTS<State, Action>::Node::uct(int32_t parent_visit_count) const {
  if (visit_count == 0) return std::numeric_limits<float>::max();
  return static_cast<float>(score) / visit_count + config.C * sqrtf(std::log(parent_visit_count) / visit_count);
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline MCTS<State, Action>::Node* MCTS<State, Action>::Node::select() {
  Node* selected_child{nullptr};
  float max_uct{std::numeric_limits<float>::lowest()};
  for (const auto& [child, action] : children) {
    float child_uct{child->uct(visit_count)};
    if (child_uct > max_uct) {
      selected_child = child.get();
      max_uct = child_uct;
    }
  }
  if (selected_child == nullptr) return this;
  return selected_child->select();
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline MCTS<State, Action>::Node* MCTS<State, Action>::Node::expand() {
  if (state.is_terminal()) return this;
  std::vector<std::pair<State, Action>> transitions = state.get_transitions();
  for (const auto& [child_state, action] : transitions) {
    auto child_node = std::make_unique<Node>(child_state, config, this);
    children.emplace_back(std::move(child_node), action);
  }
  return select();
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline float MCTS<State, Action>::Node::simulate() const {
  State current_state{state};
  std::mt19937 rng{std::random_device{}()};
  int32_t move_counter{0};
  constexpr int32_t move_threshold{400};
  while (move_counter++ < move_threshold) {
    std::optional<Action> action{current_state.get_random_action(rng)};
    if (!action) break;
    current_state = current_state.apply_action(*action);
  }
  if (move_counter >= move_threshold) return 0.5;
  return current_state.get_score();
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline void MCTS<State, Action>::Node::backprop(float result) {
  visit_count++;
  score += result;
  if (parent != nullptr) {
    parent->backprop(result);
  }
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline Action MCTS<State, Action>::Node::get_best_action() const {
  std::optional<Action> best_action{std::nullopt};
  float max_uct{std::numeric_limits<float>::lowest()};
  for (const auto& [child, action] : children) {
    float child_uct{child->uct(visit_count)};
    if (child_uct > max_uct) {
      best_action = std::optional{action};
      max_uct = child_uct;
    }
  }
  return *best_action;
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline MCTS<State, Action>::Node* MCTS<State, Action>::Node::apply_action(const Action& action) {
  if (children.empty()) {
    expand();
  }
  for (auto& [child, child_action] : children) {
    if (child_action == action) {
      child->parent = nullptr;
      return child.release();
    }
  }
  return nullptr;
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline MCTS<State, Action>::MCTS(std::convertible_to<State> auto&& state, Config config)
    : root_node{std::make_unique<Node>(state, config)}, config{config} {}

template <typename State, typename Action>
  requires StateAction<State, Action>
void MCTS<State, Action>::train() {
  // 1. Selection.
  Node* selected_node{root_node->select()};

  // 2. Expansion.
  Node* simulated_node{selected_node->expand()};

  // 3. Simulation.
  float score{simulated_node->simulate()};

  // 4. Backpropagation.
  simulated_node->backprop(score);
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline void MCTS<State, Action>::apply_action(const Action& action) {
  Node* new_root_node{root_node->apply_action(action)};
  root_node = std::unique_ptr<Node>{new_root_node};
}

template <typename State, typename Action>
  requires StateAction<State, Action>
inline Action MCTS<State, Action>::get_best_action() const {
  return root_node->get_best_action();
}

}  // namespace MCTS