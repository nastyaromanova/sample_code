#include "scheme.h"
#include "tokenizer.h"
#include "parser.h"
#include "error.h"

std::shared_ptr<Object> Interpreter::Eval(std::shared_ptr<Object> expression) {
    if (!expression) {
        throw RuntimeError("() cannot be evaluated");
    }
    return expression->Eval(std::make_shared<Scope>(global_scope_));
}

std::shared_ptr<Object> Interpreter::Parse(const std::string& expression) {
    std::stringstream ss(expression);
    Tokenizer tokenizer(&ss);
    auto expr = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Unexpected token at the end");
    }
    return expr;
}

std::string Interpreter::Run(const std::string& expression) {
    std::ostringstream ss;
    auto source = Parse(expression);
    std::shared_ptr<Object> evaluated = Eval(source);
    auto output = evaluated ? std::string(*evaluated) : "()";
    return output;
}