#include <format>

#include "json.hpp"

namespace json {
namespace {
auto deparse_object(std::string& str, const Object& object) -> void;

auto deparse_value(std::string& str, const Value& value) -> void {
    switch(value.get_index()) {
    case Value::index_of<Number>:
        str += std::format("{}", value.as<Number>().value);
        break;
    case Value::index_of<String>:
        str += "\"";
        for(const auto c : value.as<String>().value) {
            switch(c) {
            case '"':
                str += "\\\"";
                break;
            case '\\':
                str += "\\\\";
                break;
            default:
                str += c;
                break;
            }
        }
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
        // remove trailing comma
        if(str.back() == ',') {
            str.back() = ']';
        } else {
            // empty array
            str += ']';
        }
        break;
    case Value::index_of<Object>:
        deparse_object(str, value.as<Object>());
        break;
    }
}

auto deparse_object(std::string& str, const Object& object) -> void {
    str += "{";
    for(const auto& [key, value] : object.children) {
        str += "\"";
        str += key;
        str += "\":";
        deparse_value(str, value);
        str += ",";
    }
    if(str.back() == ',') {
        str.pop_back(); // remove trailing comma
    }

    str += "}";
}
} // namespace

auto deparse(const Object& object) -> std::string {
    auto ret = std::string();
    deparse_object(ret, object);
    return ret;
}
} // namespace json
