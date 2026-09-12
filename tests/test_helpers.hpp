#pragma once

#include <array>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "node.hpp"
#include "search.hpp"
#include "state.hpp"

using Move = int;

struct CoinFlipState {
    std::array<int, 5> flips = {0, 1, 0, 1, 0};
    int flips_remaining = 5;
    float correct_guesses = 0.0f;

    std::vector<Move> getLegalMoves() const {
        if (isTerminal()) {
            return {};
        }
        return {0, 1};
    }

    bool isTerminal() const {
        return flips_remaining <= 0;
    }

    void applyMove(Move move) {
        const int flip_result = flips[flips_remaining % flips.size()];
        if (move == flip_result) {
            correct_guesses += 1.0f;
        }
        flips_remaining--;
    }

    std::vector<float> getReward() const {
        return {correct_guesses};
    }

    int getNumPlayers() const {
        return 1;
    }

    int getCurrentPlayer() const {
        return 0;
    }
};

struct TerminalState {
    std::vector<Move> getLegalMoves() const {
        return {};
    }

    bool isTerminal() const {
        return true;
    }

    void applyMove(Move) {
    }

    std::vector<float> getReward() const {
        return {0.0f};
    }

    int getNumPlayers() const {
        return 1;
    }

    int getCurrentPlayer() const {
        return 0;
    }
};

struct TwoPlayerState {
    int remaining_moves = 2;
    int current_player = 0;
    int score_player_0 = 0;
    int score_player_1 = 0;

    std::vector<Move> getLegalMoves() const {
        if (isTerminal()) {
            return {};
        }
        return {0, 1};
    }

    bool isTerminal() const {
        return remaining_moves <= 0;
    }

    void applyMove(Move move) {
        if (current_player == 0) {
            score_player_0 += move;
        } else {
            score_player_1 += move;
        }

        current_player = (current_player + 1) % 2;
        remaining_moves--;
    }

    std::vector<float> getReward() const {
        return {static_cast<float>(score_player_0), static_cast<float>(score_player_1)};
    }

    int getNumPlayers() const {
        return 2;
    }

    int getCurrentPlayer() const {
        return current_player;
    }
};

template <typename State>
std::shared_ptr<Node<State, Move>> makeRoot(const State& state) {
    auto root = std::make_shared<Node<State, Move>>();
    root->state = state;
    root->total_score = std::vector<float>(state.getNumPlayers(), 0.0f);
    return root;
}
