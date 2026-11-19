#include <gtest/gtest.h>
#include "AVL.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

template <class Tree, class Order>
std::vector<int> collect(const Tree& tree, Order order) {
    std::vector<int> result;

    tree.traverse(order, [&](const int& value) {
        result.push_back(value);
    });

    return result;
}

TEST(AVLTreeTest, EmptyTree) {
    AvlTree<int> tree;

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.height(), 0);
    EXPECT_FALSE(tree.contains(10));
    EXPECT_FALSE(tree.erase(10));
    EXPECT_EQ(tree.serialize(), "");
}

TEST(AVLTreeTest, InsertAndDuplicate) {
    AvlTree<int> tree;

    EXPECT_TRUE(tree.insert(10));
    EXPECT_TRUE(tree.insert(5));
    EXPECT_TRUE(tree.insert(20));
    EXPECT_FALSE(tree.insert(10));

    EXPECT_EQ(tree.size(), 3u);
    EXPECT_TRUE(tree.contains(5));
    EXPECT_TRUE(tree.contains(10));
    EXPECT_TRUE(tree.contains(20));
    EXPECT_FALSE(tree.contains(15));
}

TEST(AVLTreeTest, AllFourRotationCases) {
    const int cases[4][3] = {
        {30, 20, 10},
        {10, 20, 30},
        {30, 10, 20},
        {10, 30, 20}
    };

    for (const auto& values : cases) {
        AvlTree<int> tree;

        for (int value : values)
            tree.insert(value);

        EXPECT_EQ(tree.serialize(), "{20}({10})[{30}]");
        EXPECT_EQ(tree.height(), 2);
    }
}

TEST(AVLTreeTest, EraseLeafOneChildAndTwoChildren) {
    AvlTree<int> tree;
    for (int value : {40, 20, 60, 10, 30, 50, 70})
        tree.insert(value);

    EXPECT_TRUE(tree.erase(10));
    EXPECT_TRUE(tree.erase(20));
    EXPECT_TRUE(tree.erase(40));

    EXPECT_FALSE(tree.erase(999));
    EXPECT_EQ(tree.size(), 4u);
    EXPECT_EQ(
        collect(tree, TraversalOrder::LKP),
        (std::vector<int>{30, 50, 60, 70})
    );
}

TEST(AVLTreeTest, SixTraversalOrders) {
    AvlTree<int> tree;
    for (int value : {4, 2, 6, 1, 3, 5, 7})
        tree.insert(value);

    EXPECT_EQ(collect(tree, TraversalOrder::KLP),
              (std::vector<int>{4, 2, 1, 3, 6, 5, 7}));

    EXPECT_EQ(collect(tree, TraversalOrder::KPL),
              (std::vector<int>{4, 6, 7, 5, 2, 3, 1}));

    EXPECT_EQ(collect(tree, TraversalOrder::LKP),
              (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));

    EXPECT_EQ(collect(tree, TraversalOrder::LPK),
              (std::vector<int>{1, 3, 2, 5, 7, 6, 4}));

    EXPECT_EQ(collect(tree, TraversalOrder::PLK),
              (std::vector<int>{7, 5, 6, 3, 1, 2, 4}));

    EXPECT_EQ(collect(tree, TraversalOrder::PKL),
              (std::vector<int>{7, 6, 5, 4, 3, 2, 1}));
}

TEST(AVLTreeTest, TraverseCanReturnSequence) {
    AvlTree<int> tree;
    for (int value : {2, 1, 3})
        tree.insert(value);

    auto sequence = tree.traverse(TraversalOrder::LKP);

    ASSERT_NE(sequence, nullptr);
    ASSERT_EQ(sequence->GetLength(), 3);
    EXPECT_EQ(sequence->Get(0), 1);
    EXPECT_EQ(sequence->Get(1), 2);
    EXPECT_EQ(sequence->Get(2), 3);
}

TEST(AVLTreeTest, StaysBalancedWithSortedInput) {
    AvlTree<int> tree;

    for (int value = 1; value <= 1000; ++value)
        tree.insert(value);

    EXPECT_EQ(tree.size(), 1000);
    EXPECT_LE(tree.height(), 15);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_TRUE(tree.contains(500));
    EXPECT_TRUE(tree.contains(1000));
}

TEST(AVLTreeTest, CopyAndMove) {
    AvlTree<int> original;
    for (int value : {1, 2, 3})
        original.insert(value);

    AvlTree<int> copy(original);
    copy.erase(1);
    EXPECT_TRUE(original.contains(1));
    EXPECT_FALSE(copy.contains(1));

    AvlTree<int> assigned;
    assigned = original;
    assigned.erase(2);
    EXPECT_TRUE(original.contains(2));

    AvlTree<int> moved(std::move(copy));
    EXPECT_TRUE(moved.contains(3));
    EXPECT_TRUE(copy.empty());

    AvlTree<int> moveAssigned;
    moveAssigned = std::move(assigned);
    EXPECT_TRUE(moveAssigned.contains(1));
    EXPECT_TRUE(assigned.empty());
}

TEST(AVLTreeTest, MapWhereAndReduce) {
    AvlTree<int> tree;
    for (int value : {1, 2, 3, 4, 5})
        tree.insert(value);

    auto doubled = tree.map([](int value) {
        return value * 2;
    });

    auto even = tree.where([](int value) {
        return value % 2 == 0;
    });

    int sum = tree.reduce(
        [](int current, int value) {
            return current + value;
        },
        0
    );

    EXPECT_EQ(collect(doubled, TraversalOrder::LKP),(std::vector<int>{2, 4, 6, 8, 10}));
    EXPECT_EQ(collect(even, TraversalOrder::LKP),(std::vector<int>{2, 4}));
    EXPECT_EQ(sum, 15);
    EXPECT_EQ(tree.size(), 5);
}

TEST(AVLTreeTest, CustomComparator) {
    AvlTree<int, std::greater<int>> tree(std::greater<int>{});

    for (int value : {2, 1, 3})
        tree.insert(value);

    EXPECT_EQ(collect(tree, TraversalOrder::LKP),(std::vector<int>{3, 2, 1}));
}

TEST(AVLTreeTest, SerializationEscapesSpecialCharacters) {
    AvlTree<std::string> tree;
    tree.insert("a}b\\c");

    EXPECT_EQ(tree.serialize(), "{a\\}b\\\\c}");
}

TEST(AVLTreeTest, Clear) {
    AvlTree<int> tree;
    for (int value : {1, 2, 3})
        tree.insert(value);

    tree.clear();

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.height(), 0);
    EXPECT_EQ(tree.serialize(), "");
}

TEST(AVLTreeSerializationTest, SingleNodeAndMissingChildren) {
    AvlTree<int> single;
    single.insert(10);
    EXPECT_EQ(single.serialize(), "{10}");

    AvlTree<int> onlyLeft;
    onlyLeft.insert(10);
    onlyLeft.insert(5);
    EXPECT_EQ(onlyLeft.serialize(), "{10}({5})[]");

    AvlTree<int> onlyRight;
    onlyRight.insert(10);
    onlyRight.insert(20);
    EXPECT_EQ(onlyRight.serialize(), "{10}()[{20}]");

    AvlTree<int> both;
    for (int value : {10, 5, 20})
        both.insert(value);
    EXPECT_EQ(both.serialize(), "{10}({5})[{20}]");
}

TEST(AVLTreeSerializationTest, PreservesNestedTreeStructure) {
    AvlTree<int> tree;
    for (int value : {10, 5, 20, 2, 7, 15, 25})
        tree.insert(value);

    EXPECT_EQ(
        tree.serialize(),
        "{10}({5}({2})[{7}])[{20}({15})[{25}]]"
    );

    EXPECT_EQ(tree.size(), 7);
    EXPECT_EQ(tree.height(), 3);
    EXPECT_TRUE(tree.contains(15));
}

TEST(AVLTreeSerializationTest, EscapesCharactersInsideValues) {
    AvlTree<std::string> tree;
    tree.insert("a}b\\c");

    EXPECT_EQ(tree.serialize(), "{a\\}b\\\\c}");
}

TEST(AVLTreeSerializationTest, PreservesDoublePrecision) {
    const double value = 0.12345678901234567;

    AvlTree<double> tree;
    tree.insert(value);

    const std::string serialized = tree.serialize();
    ASSERT_GE(serialized.size(), 3);
    ASSERT_EQ(serialized.front(), '{');
    ASSERT_EQ(serialized.back(), '}');

    const std::string number = serialized.substr(
        1, serialized.size() - 2
    );
    EXPECT_EQ(std::stod(number), value);
}

TEST(AVLTreeDeserializationTest, RestoresTreeStructure) {
    const std::string text =
        "{10}({5}({2})[{7}])[{20}({15})[{25}]]";

    AvlTree<int> tree;
    tree.deserialize(text);

    EXPECT_EQ(tree.serialize(), text);
    EXPECT_EQ(tree.size(), 7);
    EXPECT_EQ(tree.height(), 3);
    EXPECT_TRUE(tree.contains(15));
    EXPECT_FALSE(tree.contains(99));
}

TEST(AVLTreeDeserializationTest, RejectsInvalidTreeAndKeepsOldOne) {
    AvlTree<int> tree;
    tree.insert(42);

    EXPECT_THROW(
        tree.deserialize("{10}({20})[{30}]"),
        std::invalid_argument
    );

    EXPECT_EQ(tree.serialize(), "{42}");
    EXPECT_EQ(tree.size(), 1);
}