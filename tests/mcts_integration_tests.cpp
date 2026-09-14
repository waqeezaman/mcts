#include "test_helpers.hpp"

TEST(MCTSIntegrationTests, Search_TerminalRootReturnsNullopt) {
    auto root = std::make_shared<Node<TerminalState, Move>>(TerminalState{});

    const auto result = search(root, 10, 0);

    EXPECT_FALSE(result.has_value());
}

TEST(MCTSIntegrationTests, Search_ProducesLegalChildForCoinFlipState) {
    auto root = makeRoot(CoinFlipState{});

    const auto result = search(root, 100, 42);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->get()->move_from_parent == 0 || result->get()->move_from_parent == 1);
}

TEST(MCTSIntegrationTests, Search_IsDeterministicWithFixedSeed) {
    const auto root_a = makeRoot(CoinFlipState{});
    const auto root_b = makeRoot(CoinFlipState{});

    const auto result_a = search(root_a, 200, 7);
    const auto result_b = search(root_b, 200, 7);

    ASSERT_TRUE(result_a.has_value());
    ASSERT_TRUE(result_b.has_value());
    EXPECT_EQ(result_a->get()->move_from_parent, result_b->get()->move_from_parent);
}

TEST(MCTSIntegrationTests, Search_CreatesTreeAndPreservesRootState) {
    auto root = makeRoot(CoinFlipState{});

    const auto result = search(root, 100, 0);

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(root->children.empty());
    EXPECT_FALSE(root->state.isTerminal());
    EXPECT_EQ(root->total_score.size(), 1u);
}

TEST(MCTSIntegrationTests, Search_RepeatedRunsStayValid) {
    auto root = makeRoot(CoinFlipState{});

    const auto first = search(root, 100, 1);
    ASSERT_TRUE(first.has_value());

    const auto second = search(root, 100, 2);
    ASSERT_TRUE(second.has_value());

    EXPECT_TRUE(second->get()->move_from_parent == 0 || second->get()->move_from_parent == 1);
    EXPECT_FALSE(root->children.empty());
}
