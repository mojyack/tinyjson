#pragma once
#include <optional>

namespace json{
auto escape_char(char c) -> std::optional<char>;
auto unescape_char(char c) -> std::optional<char>;
}
