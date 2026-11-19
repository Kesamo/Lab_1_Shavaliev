#include "AVL.hpp"

#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;

    if (!std::getline(std::cin, line))
        throw std::runtime_error("Ввод завершён");

    return line;
}

int readInt(const std::string& prompt) {
    std::istringstream input(readLine(prompt));
    int value;

    if (!(input >> value))
        throw std::invalid_argument("Введите целое число");

    input >> std::ws;
    if (!input.eof())
        throw std::invalid_argument("После числа есть лишние символы");

    return value;
}

int main() {
    AvlTree<int> tree;
    std::string savedTree;
    bool hasSavedTree = false;

    while (true) {
        std::cout
            << "\nРазмер: " << tree.size()
            << ", высота: " << tree.height() << '\n'
            << "1 — вставить\n"
            << "2 — удалить\n"
            << "3 — найти\n"
            << "4 — показать дерево строкой\n"
            << "5 — обход\n"
            << "6 — сохранить в строку\n"
            << "7 — прочитать из строки\n"
            << "8 — заполнить случайными числами\n"
            << "9 — очистить\n"
            << "0 — выход\n";

        try {
            const int command = readInt("Выбор: ");

            if (command == 0)
                return 0;

            switch (command) {
            case 1: {
                int value = readInt("Число: ");
                std::cout << (tree.insert(value)
                    ? "Добавлено\n" : "Уже есть\n");
                break;
            }

            case 2: {
                int value = readInt("Число: ");
                std::cout << (tree.erase(value)
                    ? "Удалено\n" : "Не найдено\n");
                break;
            }

            case 3: {
                int value = readInt("Число: ");
                std::cout << (tree.contains(value)
                    ? "Найдено\n" : "Не найдено\n");
                break;
            }

            case 4:
                std::cout << tree.serialize() << '\n';
                break;

            case 5: {
                std::cout
                    << "1 КЛП, 2 КПЛ, 3 ЛКП, "
                    << "4 ЛПК, 5 ПЛК, 6 ПКЛ\n";

                int order = readInt("Обход: ");
                std::unique_ptr<Sequence<int>> values;

                switch (order) {
                case 1:
                    values = tree.traverse(TraversalOrder::KLP);
                    break;
                case 2:
                    values = tree.traverse(TraversalOrder::KPL);
                    break;
                case 3:
                    values = tree.traverse(TraversalOrder::LKP);
                    break;
                case 4:
                    values = tree.traverse(TraversalOrder::LPK);
                    break;
                case 5:
                    values = tree.traverse(TraversalOrder::PLK);
                    break;
                case 6:
                    values = tree.traverse(TraversalOrder::PKL);
                    break;
                default:
                    throw std::invalid_argument("Обход должен быть от 1 до 6");
                }

                for (std::size_t i = 0; i < values->GetLength(); ++i)
                    std::cout << values->Get(i) << ' ';

                std::cout << '\n';
                break;
            }

            case 6: {
                savedTree = tree.serialize();
                hasSavedTree = true;
                std::cout << "Сохранено: " << savedTree << '\n';
                break;
            }

            case 7: {
                std::string text = readLine(
                    "Строка дерева (Enter — использовать сохранённую): "
                );

                if (text.empty()) {
                    if (!hasSavedTree)
                        throw std::invalid_argument("Сначала сохраните дерево");
                    text = savedTree;
                }

                tree.deserialize(text);
                std::cout << "Дерево восстановлено\n";
                break;
            }

            case 8: {
                int count = readInt("Количество (0..100000): ");
                int seed = readInt("Зерно генератора: ");

                if (count < 0 || count > 100000)
                    throw std::invalid_argument("Неверное количество");

                std::mt19937 generator(seed);
                std::uniform_int_distribution<int> distribution(
                    -1000000, 1000000
                );

                for (int i = 0; i < count; ++i)
                    tree.insert(distribution(generator));

                std::cout << "Готово\n";
                break;
            }

            case 9:
                tree.clear();
                std::cout << "Дерево очищено\n";
                break;

            default:
                std::cout << "Нет такого пункта\n";
            }
        } catch (const std::exception& error) {
            if (std::cin.eof())
                return 0;

            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}