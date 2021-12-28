#include "object.h"
#include "scheme.h"

std::shared_ptr<Object> Cell::Eval(std::shared_ptr<Scope> scope) {
    if (!first_) {
        throw RuntimeError("Cannot call ()");
    }
    if (!Is<Symbol>(first_)) {
        throw RuntimeError("First element of cell is not a function");
    }
    auto function = scope->LookUp(As<Symbol>(first_)->GetName());
    if (function) {
        return function->Apply(scope, second_);
    } else {
        throw RuntimeError("Bad function");
    }
}
