#pragma once

#include "ICollection.hpp"
#include "MutableSequenceArray.hpp"
#include <functional> 
#include <memory>  

namespace TraversalOrder {
    auto LKP = [](auto root, auto left, auto right) {
        left(); root(); right();
    };

    auto KLP = [](auto root, auto left, auto right) {
    root(); left(); right();
    };

    auto KPL = [](auto root, auto left, auto right) {
        root(); right(); left();
    };

    auto LPK = [](auto root, auto left, auto right) {
        left(); right(); root();
    };

    auto PLK = [](auto root, auto left, auto right) {
        right(); left(); root();
    };

    auto PKL = [](auto root, auto left, auto right) {
        right(); root(); left();
    };
}

template <typename T, typename Compare = std::less<T>>
class AvlTree : public ICollection<T>{
    struct  Node{
        T value;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        int height = 1;
        
        Node(const T& value) : value(value) {};
    };

    using NodePtr = std::unique_ptr<Node>;

    private:    
    

    NodePtr root_;
    Compare comp_;

    int getHeight(const Node* node){
        return node ? node->height : 0;
    }

    int getBalance(Node* node){
        if(!node) return 0;
        return getHeight(node->left.get()) - getHeight(node->right.get());
    }

    void updateHeight(Node* node) {
        const int leftHeight = getHeight(node->left.get());
        const int rightHeight = getHeight(node->right.get());

        node->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
    }

    NodePtr rotateRight(NodePtr node) {
        NodePtr newRoot = std::move(node->left);

        node->left = std::move(newRoot->right);
        newRoot->right = std::move(node);

        updateHeight(newRoot->right.get());
        updateHeight(newRoot.get());
        return newRoot;
    }

    NodePtr rotateLeft(NodePtr node) {
        NodePtr newRoot = std::move(node->right);

        node->right = std::move(newRoot->left);
        newRoot->left = std::move(node);

        updateHeight(newRoot->left.get());
        updateHeight(newRoot.get());
        return newRoot;
    }

    NodePtr rebalance(NodePtr node) {
        updateHeight(node.get());

        if (getBalance(node.get()) > 1) {
            if (getBalance(node->left.get()) < 0)
                node->left = rotateLeft(std::move(node->left));

            return rotateRight(std::move(node));
        }

        if (getBalance(node.get()) < -1) {
            if (getBalance(node->right.get()) > 0)
                node->right = rotateRight(std::move(node->right));

            return rotateLeft(std::move(node));
        }

        return node;
    }

    bool insertNode(NodePtr& node, const T& value) {
        if (!node) {
            node = std::make_shared<Node>(value);
            return true;
        }

        bool inserted;

        if (comp_(value, node->value))
            inserted = insertNode(node->left, value);
        else if (comp_(node->value, value))
            inserted = insertNode(node->right, value);
        else
            return false;

        if (inserted)
            node = rebalance(std::move(node));

        return inserted;
    }
    
    bool eraseNode(NodePtr& node, const T& value) {
        if (!node){
            return false;
        }

        bool erased;

        if (comp_(value, node->value)) {
            erased = eraseNode(node->left, value);
        } else if (comp_(node->value, value)) {
            erased = eraseNode(node->right, value);
        } else {

            if (!node->left) {
                NodePtr replacement = std::move(node->right);
                node = std::move(replacement);
                return true;
            }

            if (!node->right) {
                NodePtr replacement = std::move(node->left);
                node = std::move(replacement);
                return true;
            }

            Node* successor = node->right.get();

            while (successor->left)
                successor = successor->left.get();

            T successorValue = successor->value;
            node->value = successorValue;

            eraseNode(node->right, successorValue);
            erased = true;
        }

        if (erased)
            node = rebalance(std::move(node));

        return erased;
    }

    std::size_t countNodes(const Node* node) {
        if (!node){
            return 0;
        }

        return 1 + countNodes(node->left.get()) + countNodes(node->right.get());
    }

    template <typename Order, typename Visitor>
    void walk(const Node* node, Order& order, Visitor& visit) {
        if (!node)
            return;

        auto root = [&] {
            visit(node->value);
        };

        auto left = [&] {
            walk(node->left.get(), order, visit);
        };

        auto right = [&] {
            walk(node->right.get(), order, visit);
        };

        order(root, left, right);
    }

    NodePtr cloneNode(const Node* node){
        if(!node){
            return nullptr;
        }

        NodePtr copy = std::make_unique<Node>(node->value);
    }

    static NodePtr cloneNode(const Node* node) {
        if (!node) return nullptr;

        NodePtr copy = std::make_unique<Node>(node->value);
        copy->height = node->height;
        copy->left = cloneNode(node->left.get());
        copy->right = cloneNode(node->right.get());
        return copy;
    }

    void swap(AvlTree& other) {
        using std::swap;
        swap(comp_, other.comp_);
        root_.swap(other.root_);
    }

    public:

    AvlTree() = default;

    AvlTree(const AvlTree& other) : root_(cloneNode(other.root_.get())),comp_(other.comp_) {}

    AvlTree& operator=(const AvlTree& other) {
        if (this != &other) {
            AvlTree copy(other);
            swap(copy);
        }
        return *this;
    }

    AvlTree(AvlTree&& other) = default;
    
    AvlTree& operator=(AvlTree&& other) = default;

    ~AvlTree() override = default;

    vlTree(const Compare& comp) : comp_(comp) {}

    bool insert(const T& value) override {
        return insertNode(root_, value);
    }

    bool erase(const T& value) override {
        return eraseNode(root_, value);
    }

    bool contains(const T& value) const override {
        const Node* current = root_.get();

        while (current) {
            if (comp_(value, current->value)) {
                current = current->left.get();
            } else if (comp_(current->value, value)) {
                current = current->right.get();
            } else {
                return true;
            }
        }

        return false;
    }
    
    std::size_t size() const override {
        return countNodes(root_.get());
    }

    bool empty() const override { 
        return root_ == nullptr;
    }
    
    void clear() override {
        root_.reset();
    }
    
    int height() const {
        return root_ ? root_->height : 0;
    }

    template <typename Order>
    static std::unique_ptr<Sequence<T>> traverse(Order order) {
        const std::size_t finalSize = size();
        auto result = std::make_unique<MutableSequenceArray<T>>(finalSize);
        std::size_t index = 0;

        auto visit = [&](const T& value) {
            if (index >= finalSize)
                throw std::invalid_argument("Обход посетил слишком много узлов");

            result->Set(index++, value);
        };

        walk(root_.get(), order, visit);

        if (index != finalSize)
            throw std::invalid_argument("Обход пропустил узлы");

        return result;
    }

    template<class Order, class Func>
    void traverse(Order order, Func&& action) const {
        walk(root_.get(), order, action);
    }

    template<class Func>
    AvlTree map(Func action) const{
        AvlTree result(comp_);

        traverse(TraversalOrder::LKP, [&](const T& value){
            result.insert(action(value));
        });
    }

    template<class Predicate>
    AvlTree were(Predicate predicate){
        AvlTree result(comp_);

        traverse(TraversalOrder::LKP, [&](const T& value) {
            if (predicate(value)) {
                result.insert(value);
            }
        });

        return result;
    }

    template<class Result, class Func>
    Result reduce(Func action, Result initial) const {
        traverse(TraversalOrder::LKP, [&](const T& value {
            initial = action(initial, value);
        }));

        return initial;
    }
};