#pragma once
#include "json.hpp"
#include "lexer.hpp"

namespace json {
auto parse(std::vector<Token> tokens, bool allow_trailing_commas) -> std::optional<Object>;
} // namespace json
