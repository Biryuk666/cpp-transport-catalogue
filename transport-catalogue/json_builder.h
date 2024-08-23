#pragma once

#include "json.h"
#include <string>
#include <vector>

namespace json {

    class Builder {
        class Context;
        class KeyContext;
        class DictItemContext;
        class ArrayItemContext;

    public:
        
        KeyContext Key(std::string key);
        Builder& Value(Node::Value value);        
        DictItemContext StartDict();
        Builder& EndDict();        
        ArrayItemContext StartArray();
        Builder& EndArray();        
        Node Build();
        
    private:
        Node root_; 
        std::vector<Node*>stack_;

        Node* GetCurrentNode();
        const Node* GetCurrentNode() const;
        Node* InsertNode(Node node);

        class Context {
        public:
            Context(Builder& builder) : builder_(builder) {}
            KeyContext Key(std::string key);
            Builder& Value(Node::Value value);    
            DictItemContext StartDict();
            Builder& EndDict();  
            ArrayItemContext StartArray();
            Builder& EndArray();

        private:
            Builder& builder_;

        };

        class KeyContext : public Context {
        public:
            KeyContext(Builder& builder) : Context(builder) {}
            KeyContext Key(std::string key) = delete;
            DictItemContext Value(Node::Value value);
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
        };
        
        class DictItemContext : public Context {
        public:
            DictItemContext(Builder& builder) : Context(builder) {}
            Builder& Value(Node::Value value) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;

        };

        class ArrayItemContext : public Context {
        public:
            ArrayItemContext(Builder& builder) : Context(builder) {}
            KeyContext Key(std::string key) = delete;
            Builder& EndDict() = delete;
            ArrayItemContext Value(Node::Value value);
        };

    };

    

} // namespace json