#pragma once

#include <tl/optional.hpp>

namespace type_checker {
    enum class BuiltinDataType {
        String,
        U64,
    };

    class DataType {
    public:
        [[nodiscard]] DataType() = default;
        DataType(DataType const& other) = delete;
        DataType(DataType&& other) noexcept = default;
        DataType& operator=(DataType const& other) = delete;
        DataType& operator=(DataType&& other) noexcept = default;
        virtual ~DataType() = default;

        [[nodiscard]] virtual auto as_builtin_type() const -> tl::optional<BuiltinDataType> = 0;
    };

    class String final : public DataType {
    public:
        [[nodiscard]] auto as_builtin_type() const -> tl::optional<BuiltinDataType> override {
            return BuiltinDataType::String;
        }
    };

    class U64 final : public DataType {
        [[nodiscard]] auto as_builtin_type() const -> tl::optional<BuiltinDataType> override {
            return BuiltinDataType::U64;
        }
    };
}
