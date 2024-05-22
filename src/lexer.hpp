#pragma once
#include <string>

#define CUTIL_NS json
#include "util/error.hpp"
#include "util/result.hpp"
#include "util/variant.hpp"
#undef CUTIL_NS

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

auto tokenize(std::string_view str) -> Result<std::vector<Token>, StringError>;
} // namespace json
