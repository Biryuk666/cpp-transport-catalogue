#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
    using namespace std::literals;

class Node;

// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    const Value& GetValue() const;

    Node() = default;
    Node(Array array);
    Node(bool value);    
    Node(Dict map);
    Node(int value);
    Node(std::string value);
    Node(std::nullptr_t);
    Node(double value);

    bool IsArray() const;
    bool IsBool() const;
    bool IsDouble() const;
    bool IsInt() const;
    bool IsMap() const;
    bool IsNull() const;
    bool IsPureDouble() const;
    bool IsString() const;

    const Array& AsArray() const;
    bool AsBool() const;
    double AsDouble() const;    
    int AsInt() const;
    const Dict& AsMap() const;
    const std::string& AsString() const;

private:
    Value value_;
};

inline bool operator==(const Node& lhs, const Node& rhs) { 
    return lhs.GetValue() == rhs.GetValue();
}  
inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
} 

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

struct ValuePrinter {
    std::ostream& output;
    int indent_step = 4;
    int indent = 0;
    std::ostream& operator()(std::nullptr_t);
    std::ostream& operator()(Array array);
    std::ostream& operator()(bool value);
    std::ostream& operator()(Dict dict);
    std::ostream& operator()(std::string value);

    template <typename Type>
    std::ostream& operator()(Type value);
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);
void PrintNode(const Node& node, PrintContext& context);

}  // namespace json