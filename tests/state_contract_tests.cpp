#include "test_helpers.hpp"

TEST(StateContractTests, CoinFlipState_GetLegalMoves_WhenNonTerminal) {
    CoinFlipState state;

    const auto moves = state.getLegalMoves();

    ASSERT_EQ(moves.size(), 2u);
    EXPECT_TRUE(std::find(moves.begin(), moves.end(), 0) != moves.end());
    EXPECT_TRUE(std::find(moves.begin(), moves.end(), 1) != moves.end());
}

TEST(StateContractTests, CoinFlipState_GetLegalMoves_WhenTerminal) {
    CoinFlipState state;
    state.flips_remaining = 0;

    const auto moves = state.getLegalMoves();

    EXPECT_TRUE(moves.empty());
}

TEST(StateContractTests, CoinFlipState_RewardVectorMatchesPlayerCount) {
    CoinFlipState state;

    const auto reward = state.getReward();

    ASSERT_EQ(reward.size(), 1u);
    EXPECT_GE(reward[0], 0.0f);
}

TEST(StateContractTests, TerminalState_IsTerminalAndHasNoMoves) {
    TerminalState state;

    EXPECT_TRUE(state.isTerminal());
    EXPECT_TRUE(state.getLegalMoves().empty());
    EXPECT_EQ(state.getNumPlayers(), 1);
    EXPECT_EQ(state.getCurrentPlayer(), 0);
}

TEST(StateContractTests, TwoPlayerState_TracksPlayersAndRewards) {
    TwoPlayerState state;

    ASSERT_EQ(state.getNumPlayers(), 2);
    ASSERT_EQ(state.getCurrentPlayer(), 0);
    EXPECT_TRUE(state.getLegalMoves().size() == 2u);

    state.applyMove(1);
    EXPECT_EQ(state.getCurrentPlayer(), 1);
    EXPECT_EQ(state.getReward()[0], 1.0f);
    EXPECT_EQ(state.getReward()[1], 0.0f);
}
