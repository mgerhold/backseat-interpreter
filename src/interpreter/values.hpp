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

} // namespace interpreter
