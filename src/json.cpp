#include "json.hpp"

namespace json {
auto Object::find(const std::string_view key) -> Value* {
    for(auto& c : children) {
        if(c.key == key) {
            return &c.value;
        }
    }
    return nullptr;
}

auto Object::find(const std::string_view key) const -> const Value* {
    return const_cast<Object*>(this)->find(key);
}

auto Object::operator[](const std::string_view key) -> Value& {
    auto value = find(key);
    if(!value) {
        value = &children.emplace_back(Object::KeyValue{std::string(key), {}}).value;
    }
    return *value;
}
} // namespace json
