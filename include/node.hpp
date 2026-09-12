#pragma once

#include <memory>
#include <vector>

#include "state.hpp"

template <typename State, typename Move>
requires MCTSState<State, Move>
struct Node{
    State state;

    int visits {0};
    std::vector<float> total_score;

    std::weak_ptr<Node<State, Move>> parent;
    Move move_from_parent;

    std::vector<std::shared_ptr<Node<State, Move>>> children;

    bool isLeaf() const { return children.empty(); }
};
