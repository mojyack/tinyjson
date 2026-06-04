#include "escape.hpp"
#include "macros/assert.hpp"

namespace json {
namespace {
const std::pair<char, char> table[] = {
    {'"', '"'},
    {'\\', '\\'},
    {'/', '/'},
    {'b', '\b'},
    {'f', '\f'},
    {'n', '\n'},
    {'r', '\r'},
    {'t', '\t'},
};
}

// TODO: support \u???? representations

auto escape_char(char c) -> std::optional<char> {
    for(const auto [a, b] : table) {
        if(b == c) {
            return a;
        }
    }
    return std::nullopt;
}

auto unescape_char(const char c) -> std::optional<char> {
    for(const auto [a, b] : table) {
        if(a == c) {
            return b;
        }
    }
    bail("unknown escape: {}", c);
}
} // namespace json
