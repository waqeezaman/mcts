#pragma once

#include <concepts>
#include <vector>

template <typename State, typename Move>
concept MCTSState = requires(State s, Move m){
    
    { s.getLegalMoves() } -> std::same_as<std::vector<Move>>;

    { s.isTerminal() } -> std::same_as<bool>;

    { s.applyMove(m) };
    
    { s.getReward() } -> std::same_as<std::vector<float>>;

    { s.getNumPlayers() } -> std::same_as<int>;
    { s.getCurrentPlayer() } -> std::same_as<int>;
};