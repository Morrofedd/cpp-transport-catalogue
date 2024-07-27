#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

    class Builder {
        class BaseContext;
        class KeyContext;
        class DictValueContext;
        class ArrayItemContext;
    public:
        Builder();
        Node Build();
        KeyContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictValueContext StartDict();
        ArrayItemContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;

        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);

        class BaseContext {
        public:
            BaseContext(Builder& builder)
                : builder_(builder)
            {
            }
            DictValueContext StartDict() {
                return builder_.StartDict();
            }
            BaseContext Value(Node::Value value) {
                return GetBuilder().Value(value);
            }
            ArrayItemContext StartArray() {
                return builder_.StartArray();
            }
            KeyContext Key(std::string key) {
                return builder_.Key(key);
            }
            BaseContext EndDict() {
                return builder_.EndDict();
            }
            BaseContext EndArray() {
                return builder_.EndArray();
            }
            Node Build() {
                return builder_.Build();
            }
        protected:
            Builder& GetBuilder() {
                return builder_;
            }
        private:
            Builder& builder_;
        };

        class DictValueContext : public BaseContext {
        public:
            DictValueContext(BaseContext base)
                : BaseContext(base)
            {
            }
            Node Build() = delete;
            BaseContext Value(Node::Value value) = delete;
            BaseContext EndArray() = delete;
            DictValueContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(BaseContext base) : BaseContext(base) {}
            Node Build() = delete;
            BaseContext EndDict() = delete;
            KeyContext Key() = delete;
            ArrayItemContext Value(Node::Value value){
                
                return ArrayItemContext(BaseContext::Value(std::move(value)));
            }
        };

        class KeyContext :public BaseContext
        {
        public:
            KeyContext(BaseContext base) : BaseContext(base) {}
            Node Build() = delete;
            KeyContext Key() = delete;
            BaseContext EndArray() = delete;
            BaseContext EndDict() = delete;

            DictValueContext Value(Node::Value value) {
                return DictValueContext(BaseContext::Value(std::move(value)));
            }
        };
    };

    
}  // namespace json