#include <stack>
#include <optional>
#include "json.h"

namespace json {
    class Builder final {
        class KeyItemContext;
        class ArrayItemContext;
        class DictItemContext;
        class ItemContext;

    public:
        ~Builder() = default;

        Builder() {
            nodes_stack_.push(&root_node_);
        }

        KeyItemContext Key(std::string &&key);
        Builder &Value(Node::Value value);
        DictItemContext StartDict();
        Builder &EndDict();
        ArrayItemContext StartArray();
        Builder &EndArray();
        Node Build();

    private:
        Node root_node_;
        std::stack<Node *> nodes_stack_;
        std::optional<std::string> key_;

        class ItemContext {
        public:
            explicit ItemContext(Builder &builder);
            ArrayItemContext StartArray();
            DictItemContext StartDict();
            Builder &EndArray();
            Builder &EndDict();

        protected:
            Builder &builder_;
        };

        class KeyItemContext final : public ItemContext {
        public:
            explicit KeyItemContext(Builder &builder);
            DictItemContext Value(Node::Value value);
            Builder &EndArray() = delete;
            Builder &EndDict() = delete;
        };

        class ArrayItemContext final : public ItemContext {
        public:
            explicit ArrayItemContext(Builder &builder);
            ArrayItemContext Value(Node::Value value);
            Builder &EndDict() = delete;
        };

        class DictItemContext final : public ItemContext {
        public:
            explicit DictItemContext(Builder &builder);
            KeyItemContext Key(std::string &&key);
            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
            Builder &EndArray() = delete;
        };
    };
}
