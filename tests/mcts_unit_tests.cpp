#include "test_helpers.hpp"

TEST(MCTSUnitTests, Ucb_ZeroVisitChildReturnsInfinity) {
    auto parent = std::make_shared<Node<CoinFlipState, Move>>();
    parent->state = CoinFlipState();
    parent->visits = 1;
    parent->total_score = {1.0f};

    auto child = std::make_shared<Node<CoinFlipState, Move>>();
    child->state = CoinFlipState();
    child->visits = 0;
    child->total_score = {0.0f};

    const float value = ucb(*child, *parent);

    EXPECT_TRUE(std::isinf(value));
}

TEST(MCTSUnitTests, Ucb_IncludesAverageRewardAndExploration) {
    auto parent = std::make_shared<Node<CoinFlipState, Move>>();
    parent->state = CoinFlipState();
    parent->visits = 10;
    parent->total_score = {50.0f};

    auto child = std::make_shared<Node<CoinFlipState, Move>>();
    child->state = CoinFlipState();
    child->visits = 5;
    child->total_score = {20.0f};

    const float value = ucb(*child, *parent);

    EXPECT_GT(value, 4.0f);
    EXPECT_LT(value, 10.0f);
}

TEST(MCTSUnitTests, SelectNode_ChoosesHighestUcbChild) {
    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();
    root->visits = 10;
    root->total_score = {50.0f};

    auto strong_child = std::make_shared<Node<CoinFlipState, Move>>();
    strong_child->state = CoinFlipState();
    strong_child->visits = 10;
    strong_child->total_score = {50.0f};
    strong_child->parent = root;
    strong_child->move_from_parent = 0;

    auto weak_child = std::make_shared<Node<CoinFlipState, Move>>();
    weak_child->state = CoinFlipState();
    weak_child->visits = 10;
    weak_child->total_score = {0.0f};
    weak_child->parent = root;
    weak_child->move_from_parent = 1;

    root->children = {weak_child, strong_child};

    const auto selected = selectNode(root);

    EXPECT_EQ(selected, strong_child);
}

TEST(MCTSUnitTests, Expand_CreatesOneChildPerLegalMove) {
    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();

    expand(root);

    ASSERT_EQ(root->children.size(), 2u);
    EXPECT_TRUE(root->children[0]->move_from_parent == 0 || root->children[0]->move_from_parent == 1);
    EXPECT_TRUE(root->children[1]->move_from_parent == 0 || root->children[1]->move_from_parent == 1);
    EXPECT_EQ(root->children[0]->total_score.size(), 1u);
    EXPECT_EQ(root->children[1]->total_score.size(), 1u);
    EXPECT_EQ(root->children[0]->parent.lock(), root);
    EXPECT_EQ(root->children[1]->parent.lock(), root);
}

TEST(MCTSUnitTests, Backpropogate_UpdatesVisitsAndScoresOnAncestors) {
    auto root = std::make_shared<Node<CoinFlipState, Move>>();
    root->state = CoinFlipState();
    root->visits = 2;
    root->total_score = {10.0f};

    auto child = std::make_shared<Node<CoinFlipState, Move>>();
    child->state = CoinFlipState();
    child->visits = 0;
    child->total_score = {3.0f};
    child->parent = root;

    backpropogate(child, {4.0f});

    EXPECT_EQ(child->visits, 1);
    EXPECT_EQ(root->visits, 3);
    EXPECT_FLOAT_EQ(child->total_score[0], 7.0f);
    EXPECT_FLOAT_EQ(root->total_score[0], 14.0f);
}

TEST(MCTSUnitTests, Rollout_ReturnsRewardVectorWithExpectedRange) {
    CoinFlipState state;

    const auto reward = rollout<CoinFlipState, Move>(state, 2);

    ASSERT_EQ(reward.size(), 1u);
    EXPECT_GE(reward[0], 0.0f);
    EXPECT_LE(reward[0], 5.0f);
}
