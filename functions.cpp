#include <vector>

#include "functions.h"
#include "object.h"
#include "scheme.h"

std::vector<std::shared_ptr<Object>> GetArgsList(const std::shared_ptr<Object>& obj) {
    if (!obj) {
        return {};
    }

    auto current_obj = As<Cell>(obj);
    std::vector<std::shared_ptr<Object>> list;
    while (current_obj) {
        list.push_back(current_obj->GetFirst());
        auto next_obj = current_obj->GetSecond();

        if (next_obj && !Is<Cell>(next_obj)) {
            throw RuntimeError("Something wrong with list object");
        }
        current_obj = As<Cell>(next_obj);
    }
    return list;
}

std::vector<std::shared_ptr<Object>> EvalArgsList(const std::shared_ptr<Scope>& scope,
                                                  std::shared_ptr<Object>& obj) {
    auto args = GetArgsList(obj);
    std::vector<std::shared_ptr<Object>> eval;
    for (auto& arg : args) {
        if (!arg) {
            throw RuntimeError("Something wrong with list object : it is empty");
        }
        eval.push_back(arg->Eval(scope));
    }
    return eval;
}

template <typename ExpectedType, std::size_t ExpectedCount = 0, typename It>
void ValidateArgs(It begin, It end) {
    std::size_t count = end - begin;
    if (ExpectedCount && count != ExpectedCount) {
        throw RuntimeError("More or less arguments expected");
    }
    while (begin != end) {
        if (!Is<ExpectedType>(*begin)) {
            throw RuntimeError("Get unexpected type");
        }
        ++begin;
    }
}

template <typename ExpectedType>
class IsExpectedType : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 1) {
            throw RuntimeError("Expected one argument");
        }
        return std::make_shared<Boolean>(Is<ExpectedType>(list.front()));
    }
};

using IsNumber = IsExpectedType<Number>;
using IsBoolean = IsExpectedType<Boolean>;
using IsPair = IsExpectedType<Cell>;
using IsSymbol = IsExpectedType<Symbol>;

class IsNull : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 1) {
            throw RuntimeError("Expected one argument");
        }
        return std::make_shared<Boolean>(!list.front());
    }
};

bool IsListImpl(std::shared_ptr<Object> head) {
    while (Is<Cell>(head)) {
        head = As<Cell>(head)->GetSecond();
    }
    return !head;
}

class IsList : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 1) {
            throw RuntimeError("Expected one argument");
        }
        return std::make_shared<Boolean>(IsListImpl(list.front()));
    }
};

template <typename T, typename It, typename BinaryFunction>
int64_t Fold(It begin, It end, int64_t in, BinaryFunction f) {
    while (begin != end) {
        auto value = As<T>(*begin)->GetValue();
        in = f(in, value);
        ++begin;
    }
    return in;
}

template <typename ObjectType, typename BinaryFunc, int64_t default_num>
class ArithmeticFolder : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        ValidateArgs<ObjectType>(list.begin(), list.end());
        return std::make_shared<Number>(
            Fold<ObjectType>(list.begin(), list.end(), default_num, BinaryFunc()));
    }
};

using Add = ArithmeticFolder<Number, std::plus<>, 0>;
using Multiply = ArithmeticFolder<Number, std::multiplies<>, 1>;

template <typename ObjectType, typename BinaryFunc>
class NotEmptyFolder : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        ValidateArgs<ObjectType>(list.begin(), list.end());
        if (list.empty()) {
            throw RuntimeError("Not enough arguments");
        }
        return std::make_shared<Number>(Fold<ObjectType>(
            ++list.begin(), list.end(), As<ObjectType>(list.front())->GetValue(), BinaryFunc()));
    }
};

using Subtract = NotEmptyFolder<Number, std::minus<int64_t>>;
using Divide = NotEmptyFolder<Number, std::divides<int64_t>>;

constexpr auto MinFunction = [](int64_t a, int64_t b) { return std::min(a, b); };
using Min = NotEmptyFolder<Number, decltype(MinFunction)>;

constexpr auto MaxFunction = [](int64_t a, int64_t b) { return std::max(a, b); };
using Max = NotEmptyFolder<Number, decltype(MaxFunction)>;

template <typename BinaryFunc>
class Comparison : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        ValidateArgs<Number>(list.begin(), list.end());
        if (list.size() < 2) {
            return std::make_shared<Boolean>(true);
        }
        auto begin = ++list.begin(), end = list.end();
        for (auto it = begin; it != end; ++it) {
            if (!BinaryFunc()(As<Number>(*std::prev(it))->GetValue(),
                              As<Number>(*it)->GetValue())) {
                return std::make_shared<Boolean>(false);
            }
        }
        return std::make_shared<Boolean>(true);
    }
};

using Equal = Comparison<std::equal_to<int64_t>>;
using Less = Comparison<std::less<int64_t>>;
using Greater = Comparison<std::greater<int64_t>>;
using LessEqual = Comparison<std::less_equal<int64_t>>;
using GreaterEqual = Comparison<std::greater_equal<int64_t>>;

class Abs : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        ValidateArgs<Number, 1>(list.begin(), list.end());
        return std::make_shared<Number>(std::llabs(As<Number>(list.front())->GetValue()));
    }
};

class Not : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 1) {
            throw RuntimeError("Expected one argument");
        }
        return std::make_shared<Boolean>(Is<Boolean>(list.front()) &&
                                         !As<Boolean>(list.front())->GetValue());
    }
};

class And : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto unevaluated_list = GetArgsList(obj);
        if (unevaluated_list.empty()) {
            return std::make_shared<Boolean>(true);
        }
        std::shared_ptr<Object> result;
        for (const auto& unevaluated_arg : unevaluated_list) {
            if (unevaluated_arg) {
                result = unevaluated_arg->Eval(scope);
            } else {
                result = nullptr;
            }
            if (Is<Boolean>(result) && !As<Boolean>(result)->GetValue()) {
                return std::make_shared<Boolean>(false);
            }
        }
        return result;
    }
};

class Or : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = GetArgsList(obj);
        if (list.empty()) {
            return std::make_shared<Boolean>(false);
        }
        std::shared_ptr<Object> result;
        for (const auto& unevaluated_arg : list) {
            if (unevaluated_arg) {
                result = unevaluated_arg->Eval(scope);
            } else {
                result = nullptr;
            }
            if (!Is<Boolean>(result) || As<Boolean>(result)->GetValue()) {
                return result;
            }
        }
        return result;
    }
};

class Quote : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto cell = As<Cell>(obj);
        if (cell->GetSecond()) {
            throw RuntimeError("Expected one argument");
        }
        return cell->GetFirst();
    }
};

class Cons : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 2) {
            throw RuntimeError("Expected two arguments");
        }
        auto cell = std::make_shared<Cell>(Cell(list.front()));
        cell->SetIsHead(true);
        cell->SetSecond(list.back());
        return cell;
    }
};

class Car : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 1 || !Is<Cell>(list.front())) {
            throw RuntimeError("Expected other as an argument");
        }
        return As<Cell>(list.front())->GetFirst();
    }
};

class Cdr : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 1 || !Is<Cell>(list.front())) {
            throw RuntimeError("Expected other as an argument");
        }
        auto second = As<Cell>(list.front())->GetSecond();
        if (!second) {
            return nullptr;
        }
        auto tail = second;
        if (Is<Cell>(tail)) {
            As<Cell>(tail)->SetIsHead(true);
        }
        return tail;
    }
};

template <typename It>
std::shared_ptr<Object> MakeAllListsImpl(It begin, It end) {
    if (begin == end) {
        return nullptr;
    }
    std::shared_ptr<Cell> head, cell;
    head = std::make_shared<Cell>(Cell(*begin));
    head->SetIsHead(true);
    cell = head;
    auto loop_begin = ++begin, loop_end = end;
    for (auto it = loop_begin; it != loop_end; ++it) {
        auto tmp_cell = std::make_shared<Cell>(*it);
        cell->SetSecond(tmp_cell);
        cell = tmp_cell;
    }
    return head;
}

class MakeList : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        return MakeAllListsImpl(list.begin(), list.end());
    }
};

class MakeListRef : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 2 || !IsListImpl(list.front()) || !Is<Number>(list.back())) {
            throw RuntimeError("Expected other as argument");
        }
        auto values = GetArgsList(list.front());
        int64_t n = As<Number>(list.back())->GetValue();
        if (n < 0 || n >= static_cast<int64_t>(values.size())) {
            throw RuntimeError("Out of range");
        }
        return values[n];
    }
};

class MakeListTail : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Scope> scope,
                                  std::shared_ptr<Object> obj) override {
        auto list = EvalArgsList(scope, obj);
        if (list.size() != 2 || !IsListImpl(list.front()) || !Is<Number>(list.back())) {
            throw RuntimeError("Expected other as argument");
        }
        auto values = GetArgsList(list.front());
        int64_t n = As<Number>(list.back())->GetValue();
        if (n < 0 || n > static_cast<int64_t>(values.size())) {
            throw RuntimeError("Out of range");
        }
        return MakeAllListsImpl(values.begin() + n, values.end());
    }
};

std::unordered_map<std::string, std::shared_ptr<Object>> Interpreter::GetBuiltInFunctions() {
    return {{"number?", std::make_shared<IsNumber>()},
            {"boolean?", std::make_shared<IsBoolean>()},
            {"pair?", std::make_shared<IsPair>()},
            {"symbol?", std::make_shared<IsSymbol>()},
            {"null?", std::make_shared<IsNull>()},
            {"list?", std::make_shared<IsList>()},
            {"+", std::make_shared<Add>()},
            {"*", std::make_shared<Multiply>()},
            {"-", std::make_shared<Subtract>()},
            {"/", std::make_shared<Divide>()},
            {"=", std::make_shared<Equal>()},
            {"<", std::make_shared<Less>()},
            {">", std::make_shared<Greater>()},
            {"<=", std::make_shared<LessEqual>()},
            {">=", std::make_shared<GreaterEqual>()},
            {"min", std::make_shared<Min>()},
            {"max", std::make_shared<Max>()},
            {"abs", std::make_shared<Abs>()},
            {"not", std::make_shared<Not>()},
            {"and", std::make_shared<And>()},
            {"or", std::make_shared<Or>()},
            {"quote", std::make_shared<Quote>()},
            {"cons", std::make_shared<Cons>()},
            {"car", std::make_shared<Car>()},
            {"cdr", std::make_shared<Cdr>()},
            {"list", std::make_shared<MakeList>()},
            {"list-ref", std::make_shared<MakeListRef>()},
            {"list-tail", std::make_shared<MakeListTail>()}};
}