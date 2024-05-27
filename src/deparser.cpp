#include "json.hpp"

namespace json {
namespace {
auto deparse_object(std::string& str, const Object& object) -> void;

auto deparse_value(std::string& str, const Value& value) -> void {
    switch(value.get_index()) {
    case Value::index_of<Number>:
        str += std::to_string(value.as<Number>().value);
        break;
    case Value::index_of<String>:
        str += "\"";
        str += value.as<String>().value;
        str += "\"";
        break;
    case Value::index_of<Boolean>:
        str += value.as<Boolean>().value ? "true" : "false";
        break;
    case Value::index_of<Null>:
        str += "null";
        break;
    case Value::index_of<Array>:
        str += "[";
        for(const auto& e : value.as<Array>().value) {
            deparse_value(str, e);
            str += ",";
        }
        str.pop_back(); // remove trailing comma
        str += "]";
        break;
    case Value::index_of<Object>:
        deparse_object(str, value.as<Object>());
        break;
    }
}

auto deparse_object(std::string& str, const Object& object) -> void {
    str += "{";

    const auto& c = object.children;
    for(auto i = c.begin(); i != c.end(); i = std::next(i)) {
        const auto& [key, value] = *i;
        str += "\"";
        str += key;
        str += "\":";
        deparse_value(str, value);
        str += ",";
    }
    str.pop_back(); // remove trailing comma

    str += "}";
}
} // namespace

auto deparse(const Object& object) -> std::string {
    auto ret = std::string();
    deparse_object(ret, object);
    return ret;
}
} // namespace json