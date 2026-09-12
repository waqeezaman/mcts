#pragma once

#include <concepts>
#include <vector>

/**
 * @brief Concept describing the minimum state interface required by the MCTS engine.
 *
 * Any concrete game state used with the generic search must expose the set of legal
 * moves for the current position, whether the position is terminal, how a move
 * changes the state, and the reward vector reported for all players.
 *
 * @tparam State The concrete game state type.
 * @tparam Move  The move representation used by the state.
 *
 * @note This contract is intentionally small so the same MCTS implementation can
 *       be reused by different turn-based games and simulations.
 */
template <typename State, typename Move>
concept MCTSState = requires(State s, Move m) {
    { s.getLegalMoves() } -> std::same_as<std::vector<Move>>;

    { s.isTerminal() } -> std::same_as<bool>;

    { s.applyMove(m) };

    { s.getReward() } -> std::same_as<std::vector<float>>;

    { s.getNumPlayers() } -> std::same_as<int>;
    { s.getCurrentPlayer() } -> std::same_as<int>;
};