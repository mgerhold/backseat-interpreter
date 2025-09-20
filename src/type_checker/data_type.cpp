#include <type_checker/data_type.hpp>

namespace type_checker {
    [[nodiscard]] auto DataType::from_builtin_type(BuiltinDataType type) -> std::unique_ptr<DataType> {
        switch (type) {
            case BuiltinDataType::String:
                return std::make_unique<String>();
            case BuiltinDataType::U64:
                return std::make_unique<U64>();
        }
        throw std::runtime_error{ "Unknown builtin data type." };
    }
} // namespace type_checker
