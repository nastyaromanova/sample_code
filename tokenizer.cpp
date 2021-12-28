#include "tokenizer.h"

bool IsFirstSymbolToken(char symbol) {
    return std::isalpha(symbol) || symbol == '<' || symbol == '=' || symbol == '>' ||
           symbol == '*' || symbol == '/' || symbol == '#';
}

bool IsMiddleSymbolToken(char symbol) {
    return IsFirstSymbolToken(symbol) || std::isdigit(symbol) || symbol == '?' || symbol == '!' ||
           symbol == '-';
}

bool IsDotToken(char symbol) {
    return symbol == '.';
}

bool IsBracketToken(char symbol) {
    return symbol == '(' || symbol == ')';
}

bool IsFirstConstantToken(char symbol) {
    return std::isdigit(symbol) || symbol == '+' || symbol == '-';
}

bool IsMiddleConstantToken(char symbol) {
    return std::isdigit(symbol);
}

void Tokenizer::Next() {
    char symbol = in_->get();

    while (symbol != EOF && std::isspace(symbol)) {
        symbol = in_->get();
    }

    if (symbol == EOF) {
        token_.reset();
    } else if (symbol == '\'') {
        token_ = QuoteToken();
    } else if (IsDotToken(symbol)) {
        token_ = DotToken();
    } else if (IsBracketToken(symbol)) {
        if (symbol == '(') {
            token_ = BracketToken::OPEN;
        } else {
            token_ = BracketToken::CLOSE;
        }
    } else if (symbol == '#' && (in_->peek() == 't' || in_->peek() == 'f')) {
        token_ = BooleanToken{in_->get() == 't'};
    } else if ((symbol == '+' || symbol == '-') && !isdigit(in_->peek())) {
        std::string cur = "";
        cur += symbol;
        token_ = SymbolToken(cur);
    } else if (IsFirstSymbolToken(symbol)) {
        std::string cur = "";
        cur += symbol;
        symbol = in_->peek();
        while (IsMiddleSymbolToken(symbol)) {
            cur += static_cast<char>(in_->get());
            symbol = in_->peek();
        }
        token_ = SymbolToken(cur);
    } else if (IsFirstConstantToken(symbol)) {
        std::string cur = "";
        cur += symbol;
        symbol = in_->peek();
        while (IsMiddleConstantToken(symbol)) {
            cur += static_cast<char>(in_->get());
            symbol = in_->peek();
        }
        token_ = ConstantToken(std::stoll(cur));
    } else {
        throw SyntaxError("Syntax error");
    }
}
