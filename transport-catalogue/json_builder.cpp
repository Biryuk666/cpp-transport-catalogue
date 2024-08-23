#include "json_builder.h"
#include <stdexcept>

using namespace std;

namespace json {

    const Node* Builder::GetCurrentNode() const {
        if (stack_.empty()) throw logic_error("Attempt to change finalized JSON");
        return stack_.back();
    }

    Node* Builder::GetCurrentNode() {
        return const_cast<Node*>((static_cast<const Builder*>(this))->GetCurrentNode());
    }

    Node* Builder::InsertNode(Node node) {
        if (stack_.empty() && root_.IsNull()) {
            root_ = Node(node);
            return &root_;
        } else if (GetCurrentNode()->IsArray()) {
            Array& current_node = const_cast<Array&>(GetCurrentNode()->AsArray());
            current_node.push_back(node);
            return &current_node.back();
        } else if (GetCurrentNode()->IsString()) {
            const std::string key = GetCurrentNode()->AsString();
            stack_.pop_back();
            if (GetCurrentNode()->IsDict()) {
                Dict& current_node = const_cast<Dict&>(GetCurrentNode()->AsDict());
                current_node[key] = node;
                return &current_node[key];
            } else {
                throw std::logic_error("Key() outside a dict"s);
            }
        } else {
            throw std::logic_error("New object in wrong context"s);
        }
    }

    Builder::KeyContext Builder::Key(string key) {
        if (GetCurrentNode()->IsDict()) {
            Dict& current_node = const_cast<Dict&>(GetCurrentNode()->AsDict());
            current_node[key] = key;
            stack_.push_back(&current_node[key]);
        } else {
            throw std::logic_error("Key() outside a dict"s);
        }
        return Builder::KeyContext(*this);
    }

    Builder& Builder::Value(Node::Value value) {
        InsertNode(value);
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        stack_.push_back(InsertNode(Dict{}));
        return Builder::DictItemContext(*this);
    }

    Builder& Builder::EndDict() {
        if (GetCurrentNode()->IsDict()) {
            stack_.pop_back();
        } else {
            throw std::logic_error("EndDict() outside a dict"s);
        }
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        stack_.push_back(InsertNode(Array{}));
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndArray() {
        if (GetCurrentNode()->IsArray()) {
            stack_.pop_back();
        } else {
            throw std::logic_error("EndArray() outside an array"s);
        }
        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull() || !stack_.empty()) {
            throw std::logic_error("Attempt to build JSON which isn't finalized"s);
        }
        return root_;
    }

    Builder::KeyContext Builder::Context::Key(string key) {
        return builder_.Key(key);
    }

    Builder& Builder::Context::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder::DictItemContext Builder::Context::StartDict() {
        return builder_.StartDict();
    }

    Builder& Builder::Context::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext Builder::Context::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::Context::EndArray() {
        return builder_.EndArray();
    }

    Builder::DictItemContext Builder::KeyContext::Value(Node::Value value) {
        return DictItemContext(Context::Value(value));
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(Context::Value(value));
    }

} // namespace json