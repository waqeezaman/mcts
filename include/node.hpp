#pragma once

#include <memory>
#include <vector>

#include "state.hpp"

/**
 * @brief Represents a single node in the MCTS search tree.
 *
 * Each node is associated with a concrete game state and stores the accumulated
 * statistics required by the selection and backpropagation phases of Monte Carlo
 * Tree Search.
 *
 * @tparam State The concrete state type used by the search.
 * @tparam Move  The move type used to transition between states.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
struct Node {
    /// State associated with this node.
    State state;

    /// Number of times this node has been visited during the search.
    int visits{0};

    /// Sum of rewards accumulated for each player at this node.
    std::vector<float> total_score;

    /// Pointer to the parent node, used during backpropagation.
    std::weak_ptr<Node<State, Move>> parent;

    /// Move applied to reach this node from the parent.
    Move move_from_parent;

    /// Child nodes generated from legal moves in the current state.
    std::vector<std::shared_ptr<Node<State, Move>>> children;

    /**
     * @brief Returns whether this node has no children.
     * @return `true` if the node is a leaf; otherwise `false`.
     */
    bool isLeaf() const { return children.empty(); }
};
