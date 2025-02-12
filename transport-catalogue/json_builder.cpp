#include "json_builder.h"


namespace json {
    Builder::KeyItemContext Builder::Key(std::string &&key) {
        auto &top_node = nodes_stack_.top();
        if (top_node->IsDict() && !key_) {
            key_ = std::move(key);
        } else {
            throw std::logic_error("Key use not for Dict");
        }
        return KeyItemContext(*this);
    }

    Builder &Builder::Value(Node::Value value) {
        auto &top_node = nodes_stack_.top();
        if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("Empty key");
            }
            auto &dict = std::get<Dict>(top_node->GetValue());
            auto &dict_node = dict[std::move(*key_)];
            dict_node.GetValue() = std::move(value);
            key_ = std::nullopt;
            if (dict_node.IsArray() || dict_node.IsDict()) {
                nodes_stack_.push(&dict_node);
            }
        } else if (top_node->IsArray()) {
            auto &array = std::get<Array>(top_node->GetValue());
            array.push_back(Node());
            auto &array_element = array.back();
            array_element.GetValue() = std::move(value);
            if (array_element.IsArray() || array_element.IsDict()) {
                nodes_stack_.push(&array_element);
            }
        } else if (root_node_.IsNull()) {
            root_node_.GetValue() = std::move(value);
        } else {
            throw std::logic_error("Not correct value");
        }
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        auto &top_node = nodes_stack_.top();
        if (top_node->IsDict()) {
            Value(Dict());
        } else if (top_node->IsArray()) {
            Value(Dict());
        } else if (root_node_.IsNull()) {
            top_node->GetValue() = Dict();
        } else {
            Value(Dict());
        }
        return DictItemContext(*this);
    }

    Builder &Builder::EndDict() {
        auto &top_node = nodes_stack_.top();
        if (top_node->IsDict()) {
            nodes_stack_.pop();
        } else {
            throw std::logic_error("EndDict use not for Dict");
        }
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        auto &top_node = nodes_stack_.top();
        if (top_node->IsArray()) {
            Value(Array());
        } else if (top_node->IsDict()) {
            Value(Array());
        } else if (root_node_.IsNull()) {
            top_node->GetValue() = Array();
        } else {
            Value(Array());
        }
        return ArrayItemContext(*this);
    }

    Builder &Builder::EndArray() {
        auto &top_node = nodes_stack_.top();
        if (top_node->IsArray()) {
            nodes_stack_.pop();
        } else {
            throw std::logic_error("EndArray use not for Array");
        }
        return *this;
    }

    Node Builder::Build() {
        if (nodes_stack_.size() > 1) {
            throw std::logic_error("Stack have element:");
        } else if (root_node_.IsNull()) {
            throw std::logic_error("Empty root on build");
        }
        return root_node_;
    }

    Builder::ItemContext::ItemContext(Builder &builder) : builder_(builder) {
    }


    Builder::KeyItemContext::KeyItemContext(Builder &builder): ItemContext(builder) {
    }

    Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(std::move(value)));
    };


    Builder::DictItemContext Builder::ItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::ItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::ArrayItemContext::ArrayItemContext(Builder &builder): ItemContext(builder) {
    };

    Builder &Builder::ItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(std::move(value)));
    }

    Builder::DictItemContext::DictItemContext(Builder &builder): ItemContext(builder) {
    };

    Builder::KeyItemContext Builder::DictItemContext::Key(std::string &&key) {
        return builder_.Key(std::move(key));
    }

    Builder &Builder::ItemContext::EndDict() {
        return builder_.EndDict();
    }
}