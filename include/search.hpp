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

// Forward declarations
template<typename State, typename Move>
requires MCTSState<State, Move>
std::optional<std::shared_ptr<Node<State, Move>>> search(std::shared_ptr<Node<State, Move>> root_node, int iterations, int seed);

template <typename State, typename Move>
requires MCTSState<State, Move>
float ucb(const Node<State, Move>& node, const Node<State, Move>& parent);

template <typename State, typename Move>
requires MCTSState<State, Move>
std::shared_ptr<Node<State, Move>> selectNode(std::shared_ptr<Node<State, Move>> root_node);

template <typename State, typename Move>
requires MCTSState<State, Move>
void expand(std::shared_ptr<Node<State, Move>> node);

template <typename State, typename Move>
requires MCTSState<State, Move>
std::vector<float> rollout(State state, int seed);

template <typename State, typename Move>
requires MCTSState<State, Move>
void backpropogate(std::shared_ptr<Node<State, Move>> node, std::vector<float> reward);




template <typename State, typename Move>
requires MCTSState<State, Move>
float ucb(const Node<State, Move>& node, const Node<State, Move>& parent){
    if (node.visits == 0) return std::numeric_limits<float>::infinity();
    
    auto average = node.total_score[parent.state.getCurrentPlayer()] / node.visits;
    auto optimism = 1.41f * std::sqrt(std::log(parent.visits) / node.visits);
    
    return average + optimism;
}

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
