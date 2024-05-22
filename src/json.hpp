#pragma once
#include <string>
#include <vector>

#define CUTIL_NS json
#include "util/error.hpp"
#include "util/result.hpp"
#include "util/string-map.hpp"
#include "util/variant.hpp"
#undef CUTIL_NS

namespace json {
struct Number;
struct String;
struct Boolean;
struct Null;
struct Array;
struct Object;

using Value = Variant<Number, String, Boolean, Null, Array, Object>;

struct Number {
    double value;
};

struct String {
    std::string value;
};

struct Boolean {
    bool value;
};

struct Null {
};

struct Array {
    std::vector<Value> value;
};

struct Object {
    StringMap<Value> children;
};

// helper
template <class Arg>
auto array_append(Array& array, Arg&& arg) -> void {
    array.value.push_back(Value(Tag<std::remove_cvref_t<Arg>>(), std::move(arg)));
}

template <class Arg, class... Args>
auto array_append(Array& array, Arg&& arg, Args&&... args) -> void {
    array.value.push_back(Value(Tag<std::remove_cvref_t<Arg>>(), std::move(arg)));
    array_append(array, std::forward<Args>(args)...);
}

template <class... Args>
auto make_array(Args&&... args) -> Array {
    auto array = Array();
    array.value.reserve(sizeof...(Args));
    array_append(array, std::forward<Args>(args)...);
    return array;
}

template <class Arg>
auto object_append(Object& object, const std::string_view key, Arg&& arg) -> void {
    // FIXME: use key directly
    object.children[std::string(key)] = Value(Tag<std::remove_cvref_t<Arg>>(), std::move(arg));
}

template <class Arg, class... Args>
auto object_append(Object& object, const std::string_view key, Arg&& arg, Args&&... args) -> void {
    object_append(object, key, std::move(arg));
    object_append(object, std::forward<Args>(args)...);
}

template <class Arg, class... Args>
auto make_object(const std::string_view key, Arg&& arg, Args&&... args) -> Object {
    auto object = Object();
    object_append(object, key, std::move(arg), std::forward<Args>(args)...);
    return object;
}

// parser.cpp
auto parse(const std::string_view str) -> Result<Object, StringError>;

// deparser.cpp
auto deparse(const Object& object) -> std::string;
} // namespace json
