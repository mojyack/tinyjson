#include <array>

#include "json.hpp"
#include "lexer.hpp"
#include "macros/assert.hpp"
#include "macros/unwrap.hpp"

namespace json {
namespace {
[[maybe_unused]] auto print_token(const Token& token) -> void {
    switch(token.get_index()) {
    case Token::index_of<token::WhiteSpace>:
        std::println("(white space)");
        break;
    case Token::index_of<token::LeftBrace>:
        std::println("{{");
        break;
    case Token::index_of<token::RightBrace>:
        std::println("}}");
        break;
    case Token::index_of<token::LeftBracket>:
        std::println("[");
        break;
    case Token::index_of<token::RightBracket>:
        std::println("]");
        break;
    case Token::index_of<token::Comma>:
        std::println(",");
        break;
    case Token::index_of<token::Colon>:
        std::println(":");
        break;
    case Token::index_of<token::String>:
        std::println("STR(", token.as<token::String>().value, ")");
        break;
    case Token::index_of<token::Boolean>:
        std::println("BOOL(", token.as<token::Boolean>().value ? "true" : "false", ")");
        break;
    case Token::index_of<token::Null>:
        std::println("NULL");
        break;
    case Token::index_of<token::Number>:
        std::println("NUM(", token.as<token::Number>().value, ")");
        break;
    }
}

auto print_object(const Object& o, int indent) -> void;

auto print_value(const Value& value, int indent) -> void {
    const auto prefix = std::string(indent, ' ');
    switch(value.get_index()) {
    case Value::index_of<Number>:
        std::print("{}", value.as<Number>().value);
        break;
    case Value::index_of<String>:
        std::print(R"("{}")", value.as<String>().value);
        break;
    case Value::index_of<Boolean>:
        std::print("{}", value.as<Boolean>().value ? "true" : "false");
        break;
    case Value::index_of<Null>:
        std::print("null");
        break;
    case Value::index_of<Array>:
        std::print("[");
        for(const auto& e : value.as<Array>().value) {
            print_value(e, indent);
            std::print(",");
        }
        std::print("]");
        break;
    case Value::index_of<Object>:
        print_object(value.as<Object>(), indent);
        break;
    }
}

auto print_object(const Object& o, const int indent) -> void {
    const auto prefix1 = std::string(indent, ' ');
    const auto prefix2 = std::string(indent + 4, ' ');
    std::println("{{");
    const auto& c = o.children;
    for(auto i = c.begin(); i != c.end(); i = std::next(i)) {
        const auto& [key, value] = *i;
        std::print(R"({}"{}": )", prefix2, key);
        print_value(value, indent + 4);
        std::println(",");
    }
    std::print("{}}}", prefix1);
}

auto operator==(const Array& a, const Array& b) -> bool;
auto operator==(const Object& a, const Object& b) -> bool;

auto operator==(const Value& a, const Value& b) -> bool {
    ensure(a.get_index() == b.get_index());

    switch(a.get_index()) {
    case Value::index_of<Number>:
        return a.as<Number>().value == b.as<Number>().value;
    case Value::index_of<String>:
        return a.as<String>().value == b.as<String>().value;
    case Value::index_of<Boolean>:
        return a.as<Boolean>().value == b.as<Boolean>().value;
    case Value::index_of<Null>:
        return true;
    case Value::index_of<Array>:
        return a.as<Array>() == b.as<Array>();
    case Value::index_of<Object>:
        return a.as<Object>() == b.as<Object>();
    }
    return false;
}

auto operator==(const Array& a, const Array& b) -> bool {
    const auto& v1 = a.value;
    const auto& v2 = b.value;
    ensure(v1.size() == v2.size());
    for(auto i = 0u; i < v1.size(); i += 1) {
        ensure(v1[i] == v2[i]);
    }
    return true;
}

auto operator==(const Object& a, const Object& b) -> bool {
    const auto& x = a.children;
    const auto& y = b.children;
    ensure(x.size() == y.size());
    for(auto i = 0u; i < x.size(); i += 1) {
        const auto vy = b.find(x[i].key);
        ensure(vy);
        ensure(x[i].value == *vy);
    }
    return true;
}

struct TestCase {
    Object      object;
    std::string string;
};

// lexer test
const auto lexer_test = TestCase{
    .object = json::Object(),
    .string = "{ \n \r\n \t }",
};

// basic type test
const auto basic_test = TestCase{
    .object = make_object(
        "integer", Number(1),
        "float", Number(0.1),
        "negative", Number(-1.0),
        "string", String("hello"),
        "true", Boolean(true),
        "false", Boolean(false),
        "null", Null(),
        "array", Array(),
        "object", Object()),
    .string = R"(
    {
        "integer": 1,
        "float": .1,
        "negative": -1.0,
        "string": "hello",
        "true": true,
        "false": false,
        "null": null,
        "array": [],
        "object": {}
    })",
};

// array test
const auto array_test = TestCase{
    .object = make_object(
        "array", make_array(Number(0.0), String("hello"), Boolean(true), Null(), Array(), Object())),
    .string = R"(
    {
        "array": [
            0.0,
            "hello",
            true,
            null,
            [],
            {}
        ]
    })",
};

// nest test
const auto nest_test = TestCase{
    .object = make_object(
        "array",
        make_array(
            make_array(Number(1), Number(2), Number(3)),
            make_object("1", Number(1), "2", Number(2), "3", Number(3))),
        "object",
        make_object("array", make_array(Number(1), Number(2), Number(3)),
                    "object", make_object("1", Number(1), "2", Number(2), "3", Number(3)))),
    .string = R"(
    {
        "array": [
            [1,2,3],
            {"1":1,"2":2,"3":3}
        ],
        "object": {
            "array": [1,2,3],
            "object": {"1":1,"2":2,"3":3}
        }
    })",
};

// string test
const auto string_test = TestCase{
    .object = make_object(
        "str1", String("string"),
        "str2", String(R"("string")"),
        "str3", String(R"(\string\)")),
    .string = R"(
    {
        "str1": "string",
        "str2": "\"string\"",
        "str3": "\\string\\"
    })",
};

// comment test
const auto comment_test = TestCase{
    .object = make_object(
        "a", Number(1),
        "b", Number(2),
        "c", Number(3),
        "d", Number(4)),
    .string = R"(
    {   // line comment
        "a": 1,
        // line 1
        // line 2
        // line 3
        "b": 2,
        /*
         * block comment
         */
        "c": 3,
        "d": /*inline*/ 4
    })",
};

// trailing comma test
const auto trailing_comma_test = TestCase{
    .object = make_object(
        "a", Number(1),
        "b", Number(2),
        "c", Number(3),
        "array", make_array(Number(1), Number(2), Number(3))),
    .string = R"(
    {
        "a": 1,
        "b": 2,
        "c": 3,
        "array": [1,2,3,],
    })",
};

auto test() -> bool {
    const auto tests = std::array{
        &lexer_test,
        &basic_test,
        &array_test,
        &nest_test,
        &string_test,
        &comment_test,
        &trailing_comma_test,
    };

    for(const auto test : tests) {
        unwrap(parsed1, parse(test->string));
        ensure(parsed1 == test->object);
        std::println("stage1 ok");
        const auto str = deparse(parsed1);
        std::println("{}", str);
        unwrap(parsed2, parse(str));
        ensure(parsed2 == test->object);
        std::println("stage2 ok");
    }
    return true;
}
} // namespace
} // namespace json

auto main() -> int {
    if(json::test()) {
        std::println("all pass");
        return 0;
    } else {
        return 1;
    }
}
