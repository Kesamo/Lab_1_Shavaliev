#pragma once

#include "ICollection.hpp"
#include "MutableSequenceArray.hpp"
#include "AvlIteration.hpp"
#include <functional> 
#include <memory> 
#include <iomanip>
#include <array>

// namespace TraversalOrder {
//     using Step = std::function<void()>;
//     using Policy = std::function<void(Step, Step, Step)>;

//     inline Policy LKP = [](Step root, Step left, Step right) {
//         left(); root(); right();
//     };

//     inline Policy KLP = [](Step root, Step left, Step right) {
//         root(); left(); right();
//     };

//     inline Policy KPL = [](Step root, Step left, Step right) {
//         root(); right(); left();
//     };

//     inline Policy LPK = [](Step root, Step left, Step right) {
//         left(); right(); root();
//     };

//     inline Policy PLK = [](Step root, Step left, Step right) {
//         right(); left(); root();
//     };

//     inline Policy PKL = [](Step root, Step left, Step right) {
//         right(); root(); left();
//     };
// }

namespace TraversalOrder {
    enum class Part : std::size_t {
        Root,
        Left,
        Right
    };

    using Policy = std::array<Part, 3>;

    inline Policy KLP{Part::Root, Part::Left, Part::Right};
    inline Policy KPL{Part::Root, Part::Right, Part::Left};
    inline Policy LKP{Part::Left, Part::Root, Part::Right};
    inline Policy LPK{Part::Left, Part::Right, Part::Root};
    inline Policy PLK{Part::Right, Part::Left, Part::Root};
    inline Policy PKL{Part::Right, Part::Root, Part::Left};
}

template <typename T, typename Compare = std::less<T>>
class AvlTree : public ICollection<T>, public avl::IEnumerable<T>{
    struct  Node{
        T value;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        int height = 1;
        
        Node(const T& value) : value(value) {};
    };

    using NodePtr = std::unique_ptr<Node>;
    
    NodePtr root_;
    Compare comp_;

    struct EnumerationState {};

    mutable std::shared_ptr<EnumerationState> enumerationState_;

    std::shared_ptr<EnumerationState> enumerationState() const {
        if (!enumerationState_)
            enumerationState_ = std::make_shared<EnumerationState>();

        return enumerationState_;
    }

    void invalidateEnumerators() noexcept {
        enumerationState_.reset();
    }


    class TreeEnumerator final : public avl::IEnumerator<T> {
        struct Task {
            const Node* node = nullptr;
            bool visit = false;
        };

        const Node* root_;
        const Node* current_ = nullptr;
        TraversalOrder::Policy rule_;

        MutableSequenceArray<Task> actions_;
        MutableSequenceArray<Task> stack_;

        std::size_t top_ = 0;
        std::weak_ptr<EnumerationState> state_;

        void validateRule() const {
            MutableSequenceArray<bool> used(3);

            for (std::size_t i = 0; i < rule_.size(); ++i) {
                const std::size_t index = indexOf(rule_[i]);

                if (index >= used.GetLength() || used.Get(index))
                    throw std::invalid_argument(
                        "Неверное правило обхода"
                    );

                used.Set(index, true);
            }
        }

        void checkValid() const {
            if (state_.expired())
                throw std::logic_error("Дерево изменено или уничтожено");
        }

        void push(Task task) {
            if (top_ >= stack_.GetLength())
                throw std::logic_error("Переполнен стек обхода");

            stack_.Set(top_, task);
            ++top_;
        }

        static std::size_t indexOf(TraversalOrder::Part part) {
            return static_cast<std::size_t>(part);
        }

        void expand(const Node* node) {
        actions_.Set(indexOf(TraversalOrder::Part::Root),{node, true});
        actions_.Set(indexOf(TraversalOrder::Part::Left),{node->left.get(), false});
        actions_.Set(indexOf(TraversalOrder::Part::Right),{node->right.get(), false});

        for (std::size_t i = rule_.size(); i > 0; --i) {
            Task task = actions_.Get(indexOf(rule_[i - 1]));

            if (task.node)
                push(task);
        }
    }

    public:
        TreeEnumerator(
            const Node* root,
            const TraversalOrder::Policy& rule,
            std::weak_ptr<EnumerationState> state
        )
            : root_(root),
              rule_(rule),
              actions_(3),
              stack_(root ? 2 * static_cast<std::size_t>(root->height) + 1 : 1),
              state_(std::move(state)) {
            validateRule();
            reset();
        }

        bool moveNext() override {
            checkValid();
            current_ = nullptr;

            while (top_ != 0) {
                Task task = stack_.Get(--top_);

                if (task.visit) {
                    current_ = task.node;
                    return true;
                }

                expand(task.node);
            }

            return false;
        }

        const T& current() const override {
            checkValid();

            if (!current_)
                throw std::out_of_range("Текущего элемента нет");

            return current_->value;
        }

        void reset() override {
            checkValid();
            top_ = 0;
            current_ = nullptr;

            if (root_)
                push({root_, false});
        }
    };

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
            node = std::make_unique<Node>(value);
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

    static std::size_t countNodes(const Node* node) {
        if (!node){
            return 0;
        }

        return 1 + countNodes(node->left.get()) + countNodes(node->right.get());
    }

    // template <typename Order, typename Visitor>
    // static void walk(const Node* node, Order& order, Visitor& visit) {
    //     if (!node)
    //         return;

    //     auto root = [&] {
    //         visit(node->value);
    //     };

    //     auto left = [&] {
    //         walk(node->left.get(), order, visit);
    //     };

    //     auto right = [&] {
    //         walk(node->right.get(), order, visit);
    //     };

    //     order(root, left, right);
    // }
    
    static NodePtr cloneNode(const Node* node) {
        if (!node) return nullptr;

        NodePtr copy = std::make_unique<Node>(node->value);
        copy->height = node->height;
        copy->left = cloneNode(node->left.get());
        copy->right = cloneNode(node->right.get());
        return copy;
    }

    void swap(AvlTree& other) {
        if (this == &other)
            return;

        invalidateEnumerators();
        other.invalidateEnumerators();

        using std::swap;
        swap(comp_, other.comp_);
        root_.swap(other.root_);
    }

    static void serializeNode(const Node* node, std::ostream& output) {
        if (!node) return;

        std::ostringstream valueStream;

        if (std::is_floating_point_v<T>) {
            valueStream << std::setprecision(
                std::numeric_limits<T>::max_digits10
            );
        }

        valueStream << node->value;

        if (!valueStream)
            throw std::runtime_error("Не удалось сохранить значение");

        output << '{';

        for (char ch : valueStream.str()) {
            if (ch == '}' || ch == '\\'){
                output << '\\';
            }

            output << ch;
        }

        output << '}';

        if (!node->left && !node->right)
            return;

        output << '(';
        serializeNode(node->left.get(), output);
        output << ')';

        output << '[';
        serializeNode(node->right.get(), output);
        output << ']';
    }

    static void expectChar(std::istream& input, char expected) {
    char actual;
    if (!input.get(actual) || actual != expected)
        throw std::invalid_argument("Неверный формат дерева");
    }

    static std::string readValueText(std::istream& input) {
        std::string text;
        char ch;

        while (input.get(ch)) {
            if (ch == '}')
                return text;

            if (ch == '\\') {
                if (!input.get(ch) || (ch != '}' && ch != '\\'))
                    throw std::invalid_argument("Неверное экранирование");
            }

            text += ch;
        }

        throw std::invalid_argument("Не закрыто значение узла");
    }

    static T parseValue(const std::string& text) {
        if constexpr (std::is_same_v<T, std::string>) {
            return text;
        } else {
            T value{};
            std::istringstream input(text);

            if (!(input >> value))
                throw std::invalid_argument("Неверное значение узла");

            input >> std::ws;
            if (!input.eof())
                throw std::invalid_argument("Лишние символы в значении");

            return value;
        }
    }

    NodePtr deserializeNode(
        std::istream& input,
        const T* lower,
        const T* upper,
        std::size_t depth
    ) {
        if (depth > 128)
            throw std::invalid_argument("Слишком глубокое дерево");

        expectChar(input, '{');
        T value = parseValue(readValueText(input));

        if ((lower && !comp_(*lower, value)) ||
            (upper && !comp_(value, *upper))) {
            throw std::invalid_argument("Нарушен порядок дерева поиска");
        }

        auto node = std::make_unique<Node>(value);

        if (input.peek() == '(') {
            expectChar(input, '(');

            if (input.peek() != ')')
                node->left = deserializeNode(
                    input, lower, &node->value, depth + 1
                );

            expectChar(input, ')');
            expectChar(input, '[');

            if (input.peek() != ']')
                node->right = deserializeNode(
                    input, &node->value, upper, depth + 1
                );

            expectChar(input, ']');
        }

        updateHeight(node.get());

        if (getBalance(node.get()) < -1 || getBalance(node.get()) > 1)
            throw std::invalid_argument("Дерево не сбалансировано как AVL");

        return node;
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

    AvlTree(const Compare& comp) : comp_(comp) {}

    bool insert(const T& value) override {
        invalidateEnumerators();
        return insertNode(root_, value);
    }

    bool erase(const T& value) override {
        invalidateEnumerators();
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
        invalidateEnumerators();
        root_.reset();
    }
    
    int height() const {
        return root_ ? root_->height : 0;
    }

    std::unique_ptr<Sequence<T>> traverse(TraversalOrder::Policy order) const {
        auto result = std::make_unique<MutableSequenceArray<T>>(size());
        auto enumerator = getEnumerator(order);

        for (std::size_t i = 0; i < result->GetLength(); ++i) {
            if (!enumerator->moveNext())
                throw std::logic_error("Обход пропустил узлы");

            result->Set(i, enumerator->current());
        }

        if (enumerator->moveNext())
            throw std::logic_error("Обход посетил слишком много узлов");

        return result;
    }

    template<class Func>
    void traverse(TraversalOrder::Policy order, Func&& action) const {
        auto enumerator = getEnumerator(order);

        while (enumerator->moveNext())
            action(enumerator->current());
    }

    template<class Func>
    AvlTree map(Func action) const{
        AvlTree result(comp_);

        traverse(TraversalOrder::LKP, [&](const T& value) {
            result.insert(action(value));
        });

        return result;
    }

    template<class Predicate>
    AvlTree where(Predicate predicate){
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
        traverse(TraversalOrder::LKP, [&](const T& value) {
            initial = action(initial, value);
        });

        return initial;
    }

    std::string serialize() const {
        std::ostringstream output;
        serializeNode(root_.get(), output);
        return output.str();
    }

    void deserialize(const std::string& text) {
        if (text.empty()) {
            root_.reset();
            return;
        }

        std::istringstream input(text);
        NodePtr newRoot = deserializeNode(input, nullptr, nullptr, 0);

        if (input.peek() != std::char_traits<char>::eof())
            throw std::invalid_argument("Лишние символы после дерева");

        root_ = std::move(newRoot);
    }

    std::unique_ptr<avl::IEnumerator<T>> getEnumerator() const override {
        return getEnumerator(TraversalOrder::LKP);
    }

    std::unique_ptr<avl::IEnumerator<T>>
    getEnumerator(const TraversalOrder::Policy& rule) const {
        return std::make_unique<TreeEnumerator>(root_.get(), rule, enumerationState());
    }


};