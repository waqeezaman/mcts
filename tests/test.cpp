#include <gtest/gtest.h>
#include "test.hpp"

TEST(DummyTests, dummyTest){

    ASSERT_EQ(true, true);
}


TEST(DummyTests, dummyTest2){
    ASSERT_EQ(getFive(), 5);
}