#pragma once
#include <string>
#include <vector>

#include "util/variant.hpp"

namespace json {
namespace token {
struct String {
    std::string value;
};

struct Number {
    double value;
};

struct Boolean {
    bool value;
};

struct Null {};

struct WhiteSpace {};

struct LeftBrace {};

struct RightBrace {};

struct LeftBracket {};

struct RightBracket {};

struct Comma {};

struct Colon {};

using Token = Variant<String, Number, Boolean, Null, WhiteSpace, LeftBrace, RightBrace, LeftBracket, RightBracket, Comma, Colon>;
} // namespace token

using Token = token::Token;

auto tokenize(std::string_view str) -> std::optional<std::vector<Token>>;
} // namespace json
