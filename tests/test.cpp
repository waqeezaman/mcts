#include <gtest/gtest.h>
#include "search.hpp"
#include "node.hpp"
#include "state.hpp"

using Move = int;

// A simple coin flip game
struct CoinFlipState {
    int flips[5] = {0, 1, 0, 1, 0};

    int flips_remaining = 5;
    float correct_guesses = 0;
    
    std::vector<Move> getLegalMoves() const {
        if (isTerminal()) return {};
        return {0, 1}; // 0 = heads, 1 = tails
    }
    
    bool isTerminal() const {
        return flips_remaining <= 0;
    }
    
    void applyMove(Move move) {
        int flip_result = flips[flips_remaining % std::size(flips)];
        if (move == flip_result) {
            correct_guesses++;
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


TEST(MCTSTests, testSearchInstantiation){
    // Test that search() works with shared_ptr
    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();
    
    auto best_child_optional = search(root, 10, 42);
    
    ASSERT_TRUE(best_child_optional.has_value());

    auto best_move = best_child_optional.value()->move_from_parent;

    ASSERT_TRUE(best_move == 0 || best_move == 1);
}

TEST(MCTSTests, testUCBCalculation){
    // Test UCB calculation
    auto parent_node = std::make_shared<Node<CoinFlipState, Move>>();
    parent_node->state = CoinFlipState();
    parent_node->visits = 10;
    parent_node->total_score = {50};
    
    
    auto node = std::make_shared<Node<CoinFlipState, Move>>();
    node->state = CoinFlipState();
    node->visits = 10;
    node->total_score = {50};


    
    float ucb_value = ucb(*node, *parent_node);
    
    // Average should be 5.0, plus optimism term
    ASSERT_GE(ucb_value, 5.0f);
}

TEST(MCTSTests, testRolloutReward){
    // Test rollout returns a float reward
    CoinFlipState state;
    auto reward = rollout<CoinFlipState, Move>(state, 2);
    
    // Reward should be between 0 and 5 (max flips)
    ASSERT_GE(reward[0], 0.0f);
    ASSERT_LE(reward[0], 5.0f);
}

TEST(MCTSTests, testSearch){

    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();

    auto best_child_optional = search(root, 100, 0);

    ASSERT_TRUE(best_child_optional.has_value());

    auto best_child = best_child_optional.value();
    ASSERT_TRUE(best_child->move_from_parent == 0 || best_child->move_from_parent == 1);
}


TEST(MCTSTests, testMultipleSearch){

    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();

    auto best_child_optional = search(root, 100, 0);

    ASSERT_TRUE(best_child_optional.has_value());

    auto best_child = best_child_optional.value();
    ASSERT_TRUE(best_child->move_from_parent == 0 || best_child->move_from_parent == 1);

    // Now perform another search from the best child
    auto next_best_child_optional = search(best_child, 100, 1);
    auto next_best_child = next_best_child_optional.value();
    ASSERT_TRUE(next_best_child->move_from_parent == 0 || next_best_child->move_from_parent == 1);
}

TEST(MCTSTests, testSearchUntilTerminal){

    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();


    while(!root->state.isTerminal()){
        auto best_child_optional = search(root, 100, 0);

        ASSERT_TRUE(best_child_optional.has_value());

        auto best_child = best_child_optional.value();
        ASSERT_TRUE(best_child->move_from_parent == 0 || best_child->move_from_parent == 1);

        root = best_child;
    }

    ASSERT_TRUE(root->state.isTerminal());
    ASSERT_TRUE(root->children.empty());
}