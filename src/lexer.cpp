#include <string_view>
#include <vector>

#include "lexer.hpp"
#include "macros/unwrap.hpp"

namespace json {
namespace {
template <class T, class... Args>
static auto contains(const T value, const T key, const Args... args) -> bool {
    if(value == key) {
        return true;
    }
    if constexpr(sizeof...(Args) > 0) {
        return contains(value, args...);
    }
    return false;
};

struct StringReader {
    size_t           cursor;
    std::string_view str;

    auto peek() const -> std::optional<char> {
        assert_o(cursor < str.size());
        return str[cursor];
    }

    auto read() -> std::optional<char> {
        unwrap_oo(c, peek());
        cursor += 1;
        return c;
    }

    auto read(const int len) -> std::optional<std::string_view> {
        assert_o(cursor + len < str.size());
        const auto r = str.substr(cursor, len);
        cursor += len;
        return r;
    }

    auto is_eof() const -> bool {
        return cursor >= str.size();
    }

    template <class... Args>
    auto read_until(const Args... args) -> std::optional<std::string_view> {
        auto begin = cursor;
    loop:
        unwrap_oo(c, read());
        if(contains(c, args...)) {
            cursor -= 1;
            return str.substr(begin, cursor - begin);
        }
        goto loop;
    }
};

class Lexer {
    StringReader reader;

    template <class T>
    auto create_token() -> Token {
        return Token::create<T>();
    }

    template <class T, class... Args>
    auto create_token(Args... args) -> Token {
        return Token::create<T>(std::forward<Args...>(args...));
    }

    auto parse_string_token() -> std::optional<Token> {
        assert_o(reader.read()); // skip '"'
        auto str = std::string();
        while(true) {
            unwrap_oo(c, reader.read());
            if(c == '\\') {
                unwrap_oo(d, reader.read());
                str.push_back(d);
                continue;
            }
            if(c == '"') {
                break;
            }
            str.push_back(c);
        }
        return Token::create<token::String>(std::move(str));
    }

    auto expect_string(const std::string_view expect) -> bool {
        unwrap_ob(str, reader.read(expect.size()));
        return str == expect;
    }

    auto parse_boolean_token() -> std::optional<Token> {
        unwrap_oo(next, reader.peek());
        if(next == 't') {
            return expect_string("true") ? std::optional(create_token<token::Boolean>(true)) : std::nullopt;
        } else if(next == 'f') {
            return expect_string("false") ? std::optional(create_token<token::Boolean>(false)) : std::nullopt;
        } else {
            return std::nullopt;
        }
    }

    auto parse_null_token() -> std::optional<Token> {
        return expect_string("null") ? std::optional(create_token<token::Null>()) : std::nullopt;
    }

    auto parse_number_token() -> std::optional<Token> {
        auto len = 0;
        while(true) {
            unwrap_oo(next, reader.peek());
            switch(next) {
            case '+':
            case '-':
            case '.':
            case 'e':
            case 'E':
            case 'x':
                goto ok;
            }
            if(next >= '0' && next <= '9') {
                goto ok;
            }
            break;
        ok:
            reader.read();
            len += 1;
            continue;
        }
        reader.cursor -= len;
        unwrap_oo(buf, reader.read(len));
        errno        = 0;
        const auto v = std::strtod(std::string(buf).data(), NULL);
        return errno == 0 ? std::optional(create_token<token::Number>(v)) : std::nullopt;
    }

    auto parse_next_token() -> std::optional<Token> {
        unwrap_oo(next, reader.peek());
        switch(next) {
        case ' ':
        case '\n':
            reader.read();
            return create_token<token::WhiteSpace>();
        case '{':
            reader.read();
            return create_token<token::LeftBrace>();
        case '}':
            reader.read();
            return create_token<token::RightBrace>();
        case '[':
            reader.read();
            return create_token<token::LeftBracket>();
        case ']':
            reader.read();
            return create_token<token::RightBracket>();
        case ',':
            reader.read();
            return create_token<token::Comma>();
        case ':':
            reader.read();
            return create_token<token::Colon>();
        case '"':
            return parse_string_token();
        case 't':
        case 'f':
            return parse_boolean_token();
        case 'n':
            return parse_null_token();
        case '+':
        case '-':
        case '.':
            return parse_number_token();
        }
        if(next >= '0' && next <= '9') {
            return parse_number_token();
        }
        WARN("unexpected character: '", next, "'");
        return std::nullopt;
    }

  public:
    auto tokenize() -> std::optional<std::vector<Token>> {
        auto tokens = std::vector<Token>();
        while(!reader.is_eof()) {
            unwrap_oo_mut(token, parse_next_token());
            // ignore white space
            if(token.get<token::WhiteSpace>()) {
                continue;
            }
            tokens.push_back(std::move(token));
        }
        return tokens;
    }

    auto get_current_pos() const -> std::pair<int, int> {
        auto line  = 1;
        auto chara = 1;

        const auto limit = std::min(reader.cursor, reader.str.size());
        for(auto i = 0u; i < limit; i += 1) {
            if(reader.str[i] == '\n') {
                line += 1;
                chara = 1;
            } else {
                chara += 1;
            }
        }
        return {line, chara};
    }

    Lexer(const std::string_view str) : reader({0, str}) {}
};
} // namespace

auto tokenize(const std::string_view str) -> Result<std::vector<Token>, StringError> {
    auto lexer = Lexer(str);
    auto ret_o = lexer.tokenize();
    if(!ret_o) {
        const auto [line, chara] = lexer.get_current_pos();
        return StringError(build_string("lexer error at line ", line, ", character ", chara));
    } else {
        return std::move(ret_o.value());
    }
}
} // namespace json
