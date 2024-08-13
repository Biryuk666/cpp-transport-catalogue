#include "json.h"
#include <stdexcept>
#include <utility>

using namespace std;

namespace json {
    namespace {
    
        Node LoadNode(istream& input);
        
        Node LoadArray(std::istream& input) {
            std::vector<Node> array;
            
            for (char ch; input >> ch && ch != ']';) {
                if (ch != ',') {
                    input.putback(ch);
                }
                
                array.push_back(LoadNode(input));
            }
        
            if (!input) {
                throw ParsingError("Failed to read array from stream"s);
            }
        
            return Node(array);
        }

        std::string LoadLiteral(std::istream& input) {
            std::string str;
            
            while (std::isalpha(input.peek())) {
                str.push_back(static_cast<char>(input.get()));
            }
            return str;
        }
        
        Node LoadBool(std::istream& input) {
            const auto str = LoadLiteral(input);
        
            if (str == "true"sv) {
                return Node(true);
            } else if (str == "false"sv) {
                return Node(false);
            }
            throw ParsingError("Failed to read bool from stream"s);
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node(nullptr);
            }
            throw ParsingError("Failed to read null from stream"s);
        }

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }

        }
        
        Node LoadString(std::istream& input) {
            using namespace std::literals;
            
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadDict(std::istream& input) {
            Dict dictionary;
        
            for (char ch; input >> ch && ch != '}';) {
                
                if (ch == '"') {
                    std::string key = LoadString(input).AsString();
        
                    if (input >> ch && ch == ':') {
                        
                        if (dictionary.count(key) != 0) {
                            throw ParsingError("Duplicate key: '"s + key);
                        }
        
                        dictionary.emplace(std::move(key), LoadNode(input));
                        
                    } else {
                        throw ParsingError("':' expected. Found: '"s + ch);
                    }
                    
                } else if (ch != ',') {
                    throw ParsingError("',' expected. Found: '"s + ch);
                }
            }
        
            if (!input) {
                throw ParsingError("Failed to read dictionary from stream"s);
            }
            
            return Node(dictionary);    
        }
        
        Node LoadNode(std::istream& input) {
            char ch;
            
            if (!(input >> ch)) {
                throw ParsingError(""s);
            } else {
                switch (ch) {
                case '[':
                    return LoadArray(input);
                case '{':
                    return LoadDict(input);
                case '"':
                    return LoadString(input);
                case 't': case 'f':
                    input.putback(ch);
                    return LoadBool(input);
                case 'n':
                    input.putback(ch);
                    return LoadNull(input);
                default:
                    input.putback(ch);
                    return LoadNumber(input);
                }
            }
        }
        
    } // namespace

Node::Node(Array array) : value_(move(array)) {}    
Node::Node(bool value) : value_(value) {}
Node::Node(Dict map) : value_(move(map)) {}
Node::Node(double value) : value_(value) {}
Node::Node(int value) : value_(value) {}
Node::Node(nullptr_t) : Node() {}
Node::Node(string value) : value_(move(value)) {}


bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsDouble() const {
    return IsPureDouble() || IsInt();
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}
  
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}    
 
bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}    

const Array& Node::AsArray() const {
    if (IsArray()) return get<Array>(value_);
    throw logic_error("Value type is not an array"s);
}

bool Node::AsBool() const {
    if (IsBool()) return get<bool>(value_);
    throw logic_error("Value type is not a bool"s);
}

double Node::AsDouble() const {
    if (IsPureDouble()) return get<double>(value_);
    else if (IsInt()) return AsInt();
    throw logic_error("Value type is not a double"s);
}

int Node::AsInt() const {
    if (IsInt()) return get<int>(value_);;
    throw logic_error("Value type is not an int"s);
}

const Dict& Node::AsMap() const {
    if (IsMap()) return get<Dict>(value_);
    throw logic_error("Value type is not a dictionary"s);
}

const string& Node::AsString() const {
    if (IsString()) return get<string>(value_);
    throw logic_error("Value type is not a string"s);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

const Node::Value& Node::GetValue() const {
    return value_;
}

void PrintString(std::string value, std::ostream& out){
    out << '"';
    for (const char& ch : value) {
        switch (ch) {
            case '\n':
                out << "\\n"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '"':
                out << "\\\""sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out << ch;
                break;
        }
    }
    out << '"';
}

void PrintValue (const Array& nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_context.PrintIndent();
        PrintNode(node, inner_context);
    }
    out.put('\n');
    context.PrintIndent();
    out.put(']');
}

void PrintValue(const bool& value, const PrintContext& context) {
    context.out << boolalpha << value;
}

void PrintValue(const Dict& nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_context.PrintIndent();
        PrintString(key, context.out);
        out << ": "sv;
        PrintNode(node, inner_context);
    }
    out.put('\n');
    context.PrintIndent();
    out.put('}');
}

void PrintValue(const std::nullptr_t&, const PrintContext& context) {
    context.out << "null"sv;
}

void PrintValue(const std::string& value, const PrintContext& context) {
    PrintString(value, context.out);
}

template <typename Value>
void PrintValue(const Value& value, const PrintContext& context) {
    context.out << value;
}

void PrintNode(const Node& node, PrintContext& context) {
    std::visit(
        [&context](const auto& value){ PrintValue(value, context); },
        node.GetValue());
} 

void Print(const Document& doc, std::ostream& out) {
    PrintContext context{out};
    PrintNode(doc.GetRoot(), context);
}

}  // namespace json