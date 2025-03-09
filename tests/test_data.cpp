#include <gtest/gtest.h>

#include "Data.hpp"

class DataTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Setup code here
    }

    void TearDown() override {
        // Cleanup code here
    }

    Data data;
};

TEST_F(DataTest, test) {
    // TODO
}
