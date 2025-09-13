#pragma once

#include <string>

namespace interpreter {

    class Value {
    public:
        [[nodiscard]] Value() = default;
        Value(Value const& other) = delete;
        Value(Value&& other) noexcept = default;
        Value& operator=(Value const& other) = delete;
        Value& operator=(Value&& other) noexcept = default;
        virtual ~Value() = default;
    };

    class String final : public Value {
    private:
        std::string m_data;

    public:
        [[nodiscard]] explicit String(std::string data) : m_data{ std::move(data) } { }

        [[nodiscard]] auto data() const -> std::string const& {
            return m_data;
        }
    };

    class U64 final : public Value {
    private:
        std::uint64_t m_value;

    public:
        [[nodiscard]] explicit U64(std::uint64_t const value) : m_value{ value } { }

        [[nodiscard]] auto value() const -> std::uint64_t {
            return m_value;
        }
    };

} // namespace interpreter
