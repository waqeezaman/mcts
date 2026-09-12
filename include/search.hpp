#pragma once

#include "state.hpp"
#include "node.hpp"
#include <memory>
#include <optional>
#include <random>
#include <ranges>
#include <limits>
#include <cmath>
#include <algorithm>

/**
 * @brief Runs a full Monte Carlo Tree Search from the provided root node.
 *
 * The search follows the standard MCTS loop: selection, expansion, rollout, and
 * backpropagation. The function returns the best child of the root according to
 * the average reward that was accumulated for the player whose turn it is at the
 * root state.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 *
 * @param root_node Starting node for the search tree.
 * @param iterations Number of simulation iterations to run.
 * @param seed Random seed used to control rollout sampling.
 *
 * @return The most promising child of `root_node`, or `std::nullopt` if no child
 *         was explored.
 */
template<typename State, typename Move>
requires MCTSState<State, Move>
std::optional<std::shared_ptr<Node<State, Move>>> search(std::shared_ptr<Node<State, Move>> root_node, int iterations, int seed);

/**
 * @brief Computes the UCB score for a node relative to its parent.
 *
 * The score mixes the average reward seen at the child node with an exploration
 * term that encourages visiting less-explored branches.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 *
 * @param node Child node whose score is evaluated.
 * @param parent Parent node providing the visit count and player context.
 *
 * @return UCB value used to rank candidate children.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
float ucb(const Node<State, Move>& node, const Node<State, Move>& parent);

/**
 * @brief Selects the best child along the tree using the UCB criterion.
 *
 * The search descends greedily from the root until it reaches a leaf node. When
 * the node is not terminal, it is expanded and simulated before backpropagation.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 *
 * @param root_node Root of the subtree to traverse.
 *
 * @return A leaf or terminal node selected from the current subtree.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
std::shared_ptr<Node<State, Move>> selectNode(std::shared_ptr<Node<State, Move>> root_node);

/**
 * @brief Expands a leaf node by creating one child per legal move.
 *
 * For each legal move, a new child node is created with the state after applying
 * the move, the parent pointer set, and zeroed reward totals ready for rollout
 * statistics.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 *
 * @param node Leaf node to expand.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
void expand(std::shared_ptr<Node<State, Move>> node);

/**
 * @brief Runs a random rollout from a state until the game ends.
 *
 * The rollout samples legal moves uniformly at random from the current state and
 * returns the resulting terminal reward vector used during backpropagation.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 *
 * @param state State from which to simulate the rest of the game.
 * @param seed Random seed for deterministic sampling.
 *
 * @return Reward vector for each player at the terminal state.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
std::vector<float> rollout(State state, int seed);

/**
 * @brief Backpropagates a rollout result up the tree.
 *
 * The reward from a simulation is added to the running totals for each node from
 * the rollout leaf back to the root, and each node's visit count is incremented.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 *
 * @param node Leaf node reached by the rollout.
 * @param reward Reward vector produced by the terminal state.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
void backpropogate(std::shared_ptr<Node<State, Move>> node, std::vector<float> reward);

/**
 * @brief Compute the UCB exploration score for a child node.
 *
 * The score is calculated as the empirical average reward at the child plus a
 * constant times the exploration term based on the parent visit count.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move type used by the state.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
float ucb(const Node<State, Move>& node, const Node<State, Move>& parent){
    if (node.visits == 0) return std::numeric_limits<float>::infinity();

    auto average = node.total_score[parent.state.getCurrentPlayer()] / node.visits;
    auto optimism = 1.41f * std::sqrt(std::log(parent.visits) / node.visits);

    return average + optimism;
}

/**
 * @brief Select the most promising child using UCB while descending the tree.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
std::shared_ptr<Node<State, Move>> selectNode(std::shared_ptr<Node<State, Move>> root_node){
    auto current_node = root_node;

    while(!current_node->isLeaf()){
        auto best = std::ranges::max_element(current_node->children, std::less{},
            [parent = current_node](const auto& n) { return ucb(*n, *parent); });
        if (best != current_node->children.end()) {
            current_node = *best;
        } else {
            break;
        }
    }

    return current_node;
}

/**
 * @brief Expand a node by creating child nodes for each legal action.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
void expand(std::shared_ptr<Node<State, Move>> node){

    auto moves = node->state.getLegalMoves();
    int num_players = node->state.getNumPlayers();

    node->children.reserve(moves.size());

    for (const auto& move : moves) {
        auto child = std::make_shared<Node<State, Move>>();
        child->state = node->state;
        child->state.applyMove(move);
        child->move_from_parent = move;
        child->parent = node;  // weak_ptr - doesn't increase refcount
        child->total_score = std::vector<float>(num_players, 0.0f);

        node->children.push_back(child);
    }
}

/**
 * @brief Simulate a random playout to a terminal state.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
std::vector<float> rollout(State state, int seed){
    std::mt19937 gen;
    gen.seed(seed);

    while( !state.isTerminal() ){

        auto moves = state.getLegalMoves();

        if (moves.empty()) break;

        std::uniform_int_distribution<std::size_t> dist(0, moves.size()-1);
        auto chosenMove = moves[dist(gen)];

        state.applyMove(chosenMove);
    }

    return state.getReward();
}

/**
 * @brief Add the reward of a rollout to every node along the ancestry path.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
void backpropogate(std::shared_ptr<Node<State, Move>> node, std::vector<float> reward){

    std::ranges::transform(node->total_score, reward, node->total_score.begin(), std::plus<float>{});

    node->visits += 1;

    // Traverse up using weak_ptr safely
    while (auto parent = node->parent.lock()) {
        node = parent;
        std::ranges::transform(node->total_score, reward, node->total_score.begin(), std::plus<float>{});

        node->visits += 1;
    }
}

/**
 * @brief Execute the main MCTS loop starting from the root node.
 *
 * Each iteration selects a leaf, optionally expands it, performs a random rollout,
 * and backpropagates the resulting reward to all ancestors.
 */
template <typename State, typename Move>
requires MCTSState<State, Move>
std::optional<std::shared_ptr<Node<State, Move>>> search(std::shared_ptr<Node<State, Move>> root_node, int iterations, int seed) {

    if (root_node->total_score.empty()) {
        root_node->total_score = std::vector<float>(root_node->state.getNumPlayers(), 0.0f);
    }

    for (int i = 0; i < iterations; i++) {
        // find a leaf node
        auto selected_node = selectNode(root_node);

        // expand the leaf node if it's not terminal
        if (selected_node->isLeaf() && !selected_node->state.isTerminal()) {
            expand(selected_node);

            // If expansion succeeded, select a random child for rollout
            if (!selected_node->children.empty()) {
                std::mt19937 gen(seed + i);
                std::uniform_int_distribution<size_t> dist(0, selected_node->children.size() - 1);
                selected_node = selected_node->children[dist(gen)];
            }
        }

        // random rollouts
        auto reward = rollout<State, Move>(selected_node->state, seed + i);

        // back propogate
        backpropogate(selected_node, reward);
    }

    auto best = std::ranges::max_element(root_node->children, std::less{},
            [current_player = root_node->state.getCurrentPlayer()]
            (const auto& n) { return n->visits > 0 ? n->total_score[current_player]/(n->visits) : -std::numeric_limits<float>::infinity(); });

    if(best != root_node->children.end()) {
        return *best;
    }

    return std::nullopt;
}
