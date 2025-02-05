#include <vector>

#include "json.hpp"
#include "lexer.hpp"
#include "macros/unwrap.hpp"

namespace json {
namespace {
class Parser {
  private:
    size_t             cursor;
    std::vector<Token> tokens;

    template <class T>
    auto create_value() -> Value {
        return Value::create<T>();
    }

    template <class T, class... Args>
    auto create_value(Args... args) -> Value {
        return Value::create<T>(std::forward<Args...>(args...));
    }

    auto peek() -> const Token* {
        ensure(cursor < tokens.size());
        return &tokens[cursor];
    }

    template <class T>
    auto peek_type() -> const T* {
        unwrap(next, peek());
        return next.get<T>();
    }

    auto read() -> const Token* {
        unwrap(next, peek());
        cursor += 1;
        return &next;
    }

    template <class T>
    auto read_type() -> const T* {
        unwrap(next, read());
        return next.get<T>();
    }

    auto parse_value() -> std::optional<Value> {
        unwrap(token, peek());
        switch(token.get_index()) {
        case Token::index_of<token::LeftBrace>:
            return parse_object();
        case Token::index_of<token::LeftBracket>:
            return parse_array();
        case Token::index_of<token::String>:
            read();
            return create_value<String>(token.as<token::String>().value);
        case Token::index_of<token::Number>:
            read();
            return create_value<Number>(token.as<token::Number>().value);
        case Token::index_of<token::Boolean>:
            read();
            return create_value<Boolean>(token.as<token::Boolean>().value);
        case Token::index_of<token::Null>:
            read();
            return create_value<Null>();
        default:
            return std::nullopt;
        }
    }

    auto parse_object() -> std::optional<Value> {
        ensure(read_type<token::LeftBrace>());
        auto children = std::vector<Object::KeyValue>();
        if(peek_type<token::RightBrace>()) {
            read();
            return create_value<Object>(std::move(children));
        }
    loop:
        unwrap(key, read_type<token::String>());
        ensure(read_type<token::Colon>());
        unwrap_mut(value, parse_value());
        children.emplace_back(key.value, std::move(value));

        unwrap(next, read());
        switch(next.get_index()) {
        case Token::index_of<token::RightBrace>:
            return create_value<Object>(std::move(children));
        case Token::index_of<token::Comma>:
            goto loop;
        default:
            return std::nullopt;
        }
    }

    auto parse_array() -> std::optional<Value> {
        ensure(read_type<token::LeftBracket>());
        auto array = Array();
        if(peek_type<token::RightBracket>()) {
            read();
            return create_value<Array>(std::move(array));
        }
    loop:
        unwrap_mut(value, parse_value());
        array.value.push_back(std::move(value));

        unwrap(next, read());
        switch(next.get_index()) {
        case Token::index_of<token::RightBracket>:
            return create_value<Array>(std::move(array));
        case Token::index_of<token::Comma>:
            goto loop;
        default:
            return std::nullopt;
        }
    }

  public:
    auto parse() -> std::optional<Object> {
        unwrap_mut(value, parse_object());
        return std::move(value.as<Object>());
    }

    auto get_error() -> std::string {
        return std::format("parser error at token {} of ", cursor, tokens.size());
    }

    Parser(std::vector<Token> tokens) : cursor(0), tokens(std::move(tokens)) {}
};
} // namespace

auto parse(std::vector<Token> tokens) -> std::optional<Object> {
    auto parser = Parser(std::move(tokens));
    auto ret_o  = parser.parse();
    if(!ret_o) {
        bail("{}", parser.get_error());
    } else {
        return std::move(ret_o.value());
    }
}

auto parse(const std::string_view str) -> std::optional<Object> {
    unwrap_mut(token, tokenize(str));
    unwrap_mut(object, parse(std::move(token)));
    return std::move(object);
}
} // namespace json
