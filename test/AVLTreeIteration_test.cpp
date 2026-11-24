#include <gtest/gtest.h>
#include "AVL.hpp"

#include <stdexcept>
#include <vector>

std::vector<int> collectByEnumerator(
    const AvlTree<int>& tree,
    const TraversalOrder::Policy& order
) {
    std::vector<int> result;
    auto enumerator = tree.getEnumerator(order);

    while (enumerator->moveNext())
        result.push_back(enumerator->current());

    return result;
}

TEST(AVLIteratorTest, EmptyTree) {
    AvlTree<int> tree;
    auto enumerator = tree.getEnumerator();

    EXPECT_FALSE(enumerator->moveNext());
    EXPECT_THROW(enumerator->current(), std::out_of_range);

    std::vector<int> values;
    for (int value : tree)
        values.push_back(value);

    EXPECT_TRUE(values.empty());
}

TEST(AVLIteratorTest, DefaultRangeForUsesLKP) {
    AvlTree<int> tree;
    for (int value : {4, 2, 6, 1, 3, 5, 7})
        tree.insert(value);

    std::vector<int> values;
    for (int value : tree)
        values.push_back(value);

    EXPECT_EQ(values, (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
}

TEST(AVLIteratorTest, SupportsAllSixOrders) {
    AvlTree<int> tree;
    for (int value : {4, 2, 6, 1, 3, 5, 7})
        tree.insert(value);

    EXPECT_EQ(collectByEnumerator(tree, TraversalOrder::KLP),
              (std::vector<int>{4, 2, 1, 3, 6, 5, 7}));

    EXPECT_EQ(collectByEnumerator(tree, TraversalOrder::KPL),
              (std::vector<int>{4, 6, 7, 5, 2, 3, 1}));

    EXPECT_EQ(collectByEnumerator(tree, TraversalOrder::LKP),
              (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));

    EXPECT_EQ(collectByEnumerator(tree, TraversalOrder::LPK),
              (std::vector<int>{1, 3, 2, 5, 7, 6, 4}));

    EXPECT_EQ(collectByEnumerator(tree, TraversalOrder::PLK),
              (std::vector<int>{7, 5, 6, 3, 1, 2, 4}));

    EXPECT_EQ(collectByEnumerator(tree, TraversalOrder::PKL),
              (std::vector<int>{7, 6, 5, 4, 3, 2, 1}));
}

TEST(AVLIteratorTest, ResetStartsTraversalAgain) {
    AvlTree<int> tree;
    for (int value : {2, 1, 3})
        tree.insert(value);

    auto enumerator = tree.getEnumerator();

    EXPECT_THROW(enumerator->current(), std::out_of_range);

    ASSERT_TRUE(enumerator->moveNext());
    EXPECT_EQ(enumerator->current(), 1);

    ASSERT_TRUE(enumerator->moveNext());
    EXPECT_EQ(enumerator->current(), 2);

    enumerator->reset();

    ASSERT_TRUE(enumerator->moveNext());
    EXPECT_EQ(enumerator->current(), 1);

    ASSERT_TRUE(enumerator->moveNext());
    EXPECT_EQ(enumerator->current(), 2);

    ASSERT_TRUE(enumerator->moveNext());
    EXPECT_EQ(enumerator->current(), 3);

    EXPECT_FALSE(enumerator->moveNext());
    EXPECT_THROW(enumerator->current(), std::out_of_range);
}

TEST(AVLIteratorTest, RangeForIteratorReachesEnd) {
    AvlTree<int> tree;
    for (int value : {2, 1, 3})
        tree.insert(value);

    auto iterator = tree.begin();

    ASSERT_TRUE(iterator != tree.end());
    EXPECT_EQ(*iterator, 1);

    ++iterator;
    EXPECT_EQ(*iterator, 2);

    ++iterator;
    EXPECT_EQ(*iterator, 3);

    ++iterator;
    EXPECT_TRUE(iterator == tree.end());
    EXPECT_THROW(*iterator, std::out_of_range);
}

TEST(AVLIteratorTest, ModificationInvalidatesEnumerator) {
    AvlTree<int> tree;
    for (int value : {2, 1, 3})
        tree.insert(value);

    auto enumerator = tree.getEnumerator();
    ASSERT_TRUE(enumerator->moveNext());

    tree.insert(4);

    EXPECT_THROW(enumerator->current(), std::logic_error);
    EXPECT_THROW(enumerator->moveNext(), std::logic_error);
    EXPECT_THROW(enumerator->reset(), std::logic_error);
}