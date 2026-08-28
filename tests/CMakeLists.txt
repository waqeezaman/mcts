include(GoogleTest)

# Create test runner executable
add_executable(mcts_tests test.cpp)

# Link mcts and GoogleTest's main function
target_link_libraries(mcts_tests
    PRIVATE
        mcts
        gtest_main
)

# Automatically register test cases with ctest
gtest_discover_tests(mcts_tests)