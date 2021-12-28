#pragma once

#include <string>
#include <unordered_map>
#include <type_traits>

#include "error.h"
#include "functions.h"
#include "object.h"

class Object;

class Scope {
public:
    Scope(const std::unordered_map<std::string, std::shared_ptr<Object>>& symbols)
        : symbols_(symbols) {
    }

    void Define(const std::string& name, const std::shared_ptr<Object>& obj) {
        symbols_[name] = obj;
    }

    void Reset(const std::string& name, const std::shared_ptr<Object>& obj) {
        symbols_[name] = obj;
    }

    std::shared_ptr<Object> LookUp(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return it->second;
        }
        if (parent_) {
            return parent_->LookUp(name);
        }
        throw NameError("Unknown symbol");
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> symbols_;

    std::shared_ptr<Scope> parent_;
};

class Interpreter {
public:
    Interpreter() : global_scope_(GetBuiltInFunctions()) {
    }

    std::shared_ptr<Object> Eval(std::shared_ptr<Object> expression);

    std::shared_ptr<Object> Parse(const std::string& expression);

    std::string Run(const std::string& expression);

    std::unordered_map<std::string, std::shared_ptr<Object>> GetBuiltInFunctions();

private:
    Scope global_scope_;
};
