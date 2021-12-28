#include "parser.h"

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("It is empty");
    }

    Token token = tokenizer->GetToken();
    tokenizer->Next();

    if (auto bracket = std::get_if<BracketToken>(&token)) {
        if (*bracket == BracketToken::OPEN) {
            return ReadList(tokenizer);
        } else {
            throw SyntaxError("Wrong token");
        }
    } else if (auto symbol = std::get_if<SymbolToken>(&token)) {
        return std::shared_ptr<Object>(new Symbol(symbol->name));
    } else if (auto constant = std::get_if<ConstantToken>(&token)) {
        return std::shared_ptr<Object>(new Number(constant->value));
    } else if (auto boolean = std::get_if<BooleanToken>(&token)) {
        return std::shared_ptr<Object>(new Boolean(boolean->value));
    } else if (auto quote = std::get_if<QuoteToken>(&token)) {
        auto first_cell = std::shared_ptr<Object>(new Symbol("quote"));
        auto cell = std::shared_ptr<Object>(new Cell(first_cell));

        auto first_subcell = Read(tokenizer);
        auto subcell = std::shared_ptr<Object>(new Cell(first_subcell));

        As<Cell>(cell)->SetSecond(subcell);
        return cell;
    } else {
        throw SyntaxError("Wrong token");
    }
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    std::shared_ptr<Object> root{};
    std::shared_ptr<Cell> cell{};

    bool dotted = false, need_close_bracket = false;

    while (!tokenizer->IsEnd()) {
        Token token = tokenizer->GetToken();

        if (auto bracket = std::get_if<BracketToken>(&token);
            bracket && *bracket == BracketToken::CLOSE) {
            if (dotted) {
                throw SyntaxError("Need one more object");
            }
            tokenizer->Next();
            return root;
        } else if (auto dot = std::get_if<DotToken>(&token)) {
            if (need_close_bracket) {
                throw SyntaxError("Need close bracket");
            }
            if (!root) {
                throw SyntaxError("Dot as the first element of the list");
            }
            tokenizer->Next();
            dotted = true;
        } else {
            if (need_close_bracket) {
                throw SyntaxError("Need close bracket because it was dotted");
            }
            std::shared_ptr<Object> head = Read(tokenizer);
            if (!root) {
                root = std::shared_ptr<Object>(new Cell(head));
                As<Cell>(root)->SetIsHead(true);
                cell = As<Cell>(root);
            } else {
                if (dotted) {
                    dotted = false, need_close_bracket = true;
                    const auto& second = head;
                    if (second && Is<Cell>(second)) {
                        As<Cell>(second)->SetIsHead(false);
                    }
                    cell->SetSecond(second);
                } else {
                    auto tmp_cell = std::shared_ptr<Object>(new Cell(head));
                    cell->SetSecond(tmp_cell);
                    cell = As<Cell>(tmp_cell);
                }
            }
        }
    }
    throw SyntaxError("Unexpected end of expression");
}
