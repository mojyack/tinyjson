#include "json.hpp"
#include "lexer.hpp"

#define CUTIL_NS json
#include "macros/assert.hpp"
#include "macros/unwrap.hpp"

namespace json {
namespace {
[[maybe_unused]] auto print_token(const Token& token) -> void {
    switch(token.get_index()) {
    case Token::index_of<token::WhiteSpace>:
        print("(white space)");
        break;
    case Token::index_of<token::LeftBrace>:
        print("{");
        break;
    case Token::index_of<token::RightBrace>:
        print("}");
        break;
    case Token::index_of<token::LeftBracket>:
        print("[");
        break;
    case Token::index_of<token::RightBracket>:
        print("]");
        break;
    case Token::index_of<token::Comma>:
        print(",");
        break;
    case Token::index_of<token::Colon>:
        print(":");
        break;
    case Token::index_of<token::String>:
        print("STR(", token.as<token::String>().value, ")");
        break;
    case Token::index_of<token::Boolean>:
        print("BOOL(", token.as<token::Boolean>().value ? "true" : "false", ")");
        break;
    case Token::index_of<token::Null>:
        print("NULL");
        break;
    case Token::index_of<token::Number>:
        print("NUM(", token.as<token::Number>().value, ")");
        break;
    }
}

auto print_object(const Object& o, int indent) -> void;

auto print_value(const Value& value, int indent) -> void {
    const auto prefix = std::string(indent, ' ');
    switch(value.get_index()) {
    case Value::index_of<Number>:
        std::cout << value.as<Number>().value;
        break;
    case Value::index_of<String>:
        std::cout << '"' << value.as<String>().value << '"';
        break;
    case Value::index_of<Boolean>:
        std::cout << (value.as<Boolean>().value ? "true" : "false");
        break;
    case Value::index_of<Null>:
        std::cout << "null";
        break;
    case Value::index_of<Array>:
        std::cout << "[";
        for(const auto& e : value.as<Array>().value) {
            print_value(e, indent);
            std::cout << ",";
        }
        std::cout << "]";
        break;
    case Value::index_of<Object>:
        print_object(value.as<Object>(), indent);
        break;
    }
}

auto print_object(const Object& o, const int indent) -> void {
    const auto prefix1 = std::string(indent, ' ');
    const auto prefix2 = std::string(indent + 4, ' ');
    print("{");
    const auto& c = o.children;
    for(auto i = c.begin(); i != c.end(); i = std::next(i)) {
        const auto& [key, value] = *i;
        std::cout << build_string(prefix2, "\"", key, "\": ");
        print_value(value, indent + 4);
        std::cout << "," << std::endl;
    }
    std::cout << prefix1 << "}";
}

auto operator==(const Array& a, const Array& b) -> bool;
auto operator==(const Object& a, const Object& b) -> bool;

auto operator==(const Value& a, const Value& b) -> bool {
    assert_b(a.get_index() == b.get_index());

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
    assert_b(v1.size() == v2.size());
    for(auto i = 0u; i < v1.size(); i += 1) {
        assert_b(v1[i] == v2[i]);
    }
    return true;
}

auto operator==(const Object& a, const Object& b) -> bool {
    const auto& x = a.children;
    const auto& y = b.children;
    assert_b(x.size() == y.size());
    for(auto i = 0u; i < x.size(); i += 1) {
        const auto vy = b.find(x[i].key);
        assert_b(vy);
        assert_b(x[i].value == *vy);
    }
    return true;
}

struct TestCase {
    Object      object;
    std::string string;
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
        "str2", String("\"string\"")),
    .string = R"(
    {
        "str1": "string",
        "str2": "\"string\""
    })",
};

auto test() -> Result<bool, StringError> {
    const auto tests = std::array{
        &basic_test,
        &array_test,
        &nest_test,
        &string_test,
    };

    for(const auto test : tests) {
        unwrap_re(parsed, parse(test->string));
        if(parsed != test->object) {
            return StringError("result mismatched");
        }
    }
    return true;
}

auto main() -> bool {
    const auto result = test();
    if(!result) {
        PRINT(result.as_error().cstr());
        return false;
    } else {
        PRINT("pass");
        return true;
    }
}
} // namespace
} // namespace json

auto main() -> int {
    return json::main() ? 0 : 1;
}
