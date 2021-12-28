#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_set>

#include "error.h"

class Object;

class Number;

class Symbol;

class Boolean;

class Cell;

class Scope;

class Object {
public:
    virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> scope) = 0;

    virtual operator std::string() const {
        throw RuntimeError("Cannot print abstract object");
    }

    virtual std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                          std::shared_ptr<Object> args) {
        throw RuntimeError("Cannot call apply from the abstract object");
    }

    virtual ~Object() = default;

private:
    std::unordered_set<std::shared_ptr<Object>> ptrs_;
};

class Number : public Object, public std::enable_shared_from_this<Number> {
public:
    Number() = default;

    Number(int64_t value) : value_(value) {
    }

    int GetValue() const {
        return value_;
    }

    std::shared_ptr<Object> Eval(std::shared_ptr<Scope> scope) override {
        return std::make_shared<Number>(value_);
    }

    operator std::string() const override {
        return std::to_string(value_);
    }

private:
    int64_t value_ = 0;
};

class Symbol : public Object, public std::enable_shared_from_this<Symbol> {
public:
    Symbol(const std::string& name) : name_(name) {
    }

    const std::string& GetName() const {
        return name_;
    }

    std::shared_ptr<Object> Eval(std::shared_ptr<Scope> scope) override {
        return std::make_shared<Symbol>(name_);
    }

    operator std::string() const override {
        return name_;
    }

private:
    std::string name_;
};

class Boolean : public Object, public std::enable_shared_from_this<Boolean> {
public:
    Boolean(const bool& value) : value_(value) {
    }

    bool GetValue() const {
        return value_;
    }

    std::shared_ptr<Object> Eval(std::shared_ptr<Scope> scope) override {
        return std::make_shared<Boolean>(value_);
    }

    operator std::string() const override {
        return value_ ? "#t" : "#f";
    }

private:
    bool value_;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj);

template <class T>
bool Is(const std::shared_ptr<Object>& obj);

class Cell : public Object, public std::enable_shared_from_this<Cell> {
public:
    Cell() = default;

    Cell(std::shared_ptr<Object> ptr) : first_(std::move(ptr)), second_(nullptr) {
    }

    void SetFirst(std::shared_ptr<Object> first) {
        first_ = std::move(first);
    }

    void SetSecond(std::shared_ptr<Object> second) {
        second_ = std::move(second);
    }

    void SetIsHead(bool is_head) {
        is_head_ = is_head;
    }

    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }

    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    bool GetIsHead() const {
        return is_head_;
    }

    std::shared_ptr<Object> Eval(std::shared_ptr<Scope> scope) override;

    operator std::string() const override {
        std::stringstream ss;

        if (is_head_) {
            ss << "(";
        }

        if (!first_) {
            ss << "()";
        } else {
            ss << static_cast<std::string>(*first_);
        }

        if (!second_) {
            ss << ")";
            return ss.str();
        }

        if (!Is<Cell>(second_)) {
            ss << " . " << static_cast<std::string>(*second_) << ")";
        } else {
            ss << " " << static_cast<std::string>(*second_);
        }

        return ss.str();
    }

private:
    std::shared_ptr<Object> first_ = nullptr, second_ = nullptr;
    bool is_head_ = false;
};

class Function : public Object {
public:
    virtual ~Function() = default;

    std::shared_ptr<Object> Eval(std::shared_ptr<Scope> scope) override {
        throw RuntimeError("Cannot eval function");
    }

    operator std::string() const override {
        throw RuntimeError("Cannot print function");
    }
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return obj && typeid(*obj) == typeid(T);
}
