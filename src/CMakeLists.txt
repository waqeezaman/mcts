add_library(mcts
    search.cpp
)

target_include_directories(mcts
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(mcts PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(mcts PRIVATE /W4 /WX)
else()
    target_compile_options(mcts PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
