#pragma once
#include "json.hpp"
#include "lexer.hpp"

namespace json {
auto parse(std::vector<Token> tokens) -> std::optional<Object>;
} // namespace json
