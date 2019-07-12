#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
#include "gtest/gtest.h"
#include "red_black_tree.hpp"

using namespace simplelib;

class RedBlackTreeTest : public testing::Test {
public:
    void SetUp() {
        _rbt = new RedBlackTree<int, int>();
    }

    void TearDown() {
        delete _rbt;
    }
protected:
    RedBlackTree<int, int> *_rbt;
};

TEST_F(RedBlackTreeTest, Test_ALL) {
    std::vector<int> v;
    for (int i = 0; i < 100; i++) {
      v.push_back(i);
    }

    for (int i = 0; i < 10000; i++) {
        //Check insertion and balance
        unsigned seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
        std::shuffle(v.begin(), v.end(), std::default_random_engine (seed));
        for (size_t i = 0; i < v.size(); i++) {
            ASSERT_TRUE(_rbt->tree_insert(v[i], v[i]));
            int temp = -1;
            ASSERT_TRUE(_rbt->tree_find(v[i], &temp));
            ASSERT_EQ(v[i], temp);
            if ((i + 1) % 10 == 0) {
                ASSERT_TRUE(_rbt->check_balanced());
                ASSERT_EQ(_rbt->tree_size(), i + 1);
                for (int j = i + 1; j < v.size(); j++) {
                    ASSERT_FALSE(_rbt->tree_find(v[j], &temp));
                }
            }
        }

        //Check deletion and balance
        seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
        std::shuffle(v.begin(), v.end(), std::default_random_engine (seed));
        for (int i = 0; i < 90; i++) {
            ASSERT_TRUE(_rbt->tree_delete(v[i]));
            int temp = -1;
            ASSERT_FALSE(_rbt->tree_find(v[i], &temp));
            if ((i + 1) % 10 == 0) {
                ASSERT_TRUE(_rbt->check_balanced());
                ASSERT_EQ(_rbt->tree_size(), 99 - i);
                for (int j = i + 1; j < v.size(); j++) {
                    ASSERT_TRUE(_rbt->tree_find(v[j], &temp));
                    ASSERT_EQ(v[j], temp);
                }
            }
        }

        //Check clear
        _rbt->tree_clear();
        ASSERT_TRUE(_rbt->check_balanced());
        ASSERT_EQ(_rbt->tree_size(), 0);
    }
}

int main(int argc, char** argv) {  
    testing::InitGoogleTest(&argc, argv);

    // Runs all tests using Google Test.  
    return RUN_ALL_TESTS();
}
