#pragma once

#include <variant>
#include <type_traits>

template <class TValue, class TError>
class [[nodiscard]] TErrorOr {
    static constexpr int TMP = 0;
    static constexpr int VAL = 1;
    static constexpr int ERR = 2;

public:
    TErrorOr()
        : Base_(std::in_place_index<TMP>, std::monostate{})
    {
    }

    TErrorOr(const TError& error)
        : Base_(std::in_place_index<ERR>, error)
    {
    }

    TErrorOr(const TValue& value)
        : Base_(std::in_place_index<VAL>, value)
    {
    }

    [[nodiscard]] bool Initialized() const {
        return Base_.index() != TMP;
    }

    [[nodiscard]] bool Fail() const {
        return Base_.index() != VAL;
    }

    [[nodiscard]] bool Succeed() const {
        return Base_.index() == VAL;
    }

    operator bool() const {
        return Succeed();
    }

    TError& Error() const {
        return std::get<ERR>(Base_);
    }

    TError& Error() {
        return std::get<ERR>(Base_);
    }

    TValue& Value() {
        return std::get<VAL>(Base_);
    }

    TValue& Value() const {
        return std::get<VAL>(Base_);
    }

private:
    std::variant<std::monostate, TValue, TError> Base_;
};

template <class TError>
class [[nodiscard]] TErrorOr<void, TError> {
    static constexpr int VAL = 0;
    static constexpr int ERR = 1;

public:
    TErrorOr()
            : Base_(std::in_place_index<VAL>, std::monostate{})
    {
    }

    TErrorOr(const TError& error)
            : Base_(std::in_place_index<ERR>, error)
    {
    }

    [[nodiscard]] bool Fail() const {
        return Base_.index() != VAL;
    }

    [[nodiscard]] bool Succeed() const {
        return Base_.index() == VAL;
    }

    operator bool() const {
        return Succeed();
    }

    TError& Error() const {
        return std::get<ERR>(Base_);
    }

    TError& Error() {
        return std::get<ERR>(Base_);
    }

private:
    using TState = std::variant<std::monostate, TError>;
    TState Base_;
};
