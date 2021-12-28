#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <regex>
#include "error.h"

struct SymbolToken {
    std::string name;

    SymbolToken() = default;

    SymbolToken(const std::string& n) : name(n) {
    }

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

struct QuoteToken {  // кавычка(')
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int64_t value = 0;

    ConstantToken() = default;

    ConstantToken(const int64_t& v) : value(v) {
    }

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

struct BooleanToken {
    bool value;

    BooleanToken() = default;

    BooleanToken(const bool& v) : value(v) {
    }

    bool operator==(const BooleanToken& other) const {
        return value == other.value;
    }
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BooleanToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in) : in_(in) {
        Next();
    }

    bool IsEnd() {
        return !token_;
    }

    void Next();

    Token GetToken() {
        return *token_;
    }

private:
    std::optional<Token> token_;
    std::istream* const in_;
};