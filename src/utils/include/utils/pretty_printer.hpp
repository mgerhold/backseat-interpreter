#pragma once

#include <concepts>
#include <string>
#include <vector>
#include <ranges>
#include <experimental/meta>
#include <type_checker/statements.hpp>
#include <utils/enum_to_string.hpp>

[[nodiscard]] consteval auto children_of(std::meta::info const base_type) -> std::vector<std::meta::info> {
    auto parent_namespace = parent_of(base_type);
    if (not is_namespace(parent_namespace)) {
        return {};
    }

    auto children = std::vector<std::meta::info>{};
    static constexpr auto context = std::meta::access_context::current();
    for (auto const member : members_of(parent_namespace, context)) {
        if (is_type(member) and is_class_type(member)) {
            auto const does_inherit_base =
                std::ranges::any_of(bases_of(member, context), [&](auto const& base) {
                    return is_same_type(type_of(base), base_type);
                });
            if (does_inherit_base) {
                children.push_back(member);
            }
        }
    }
    return children;
}

template<typename BaseType>
[[nodiscard]] consteval auto child_pointer_types() -> std::vector<std::meta::info> {
    auto pointer_types = std::vector<std::meta::info>{};
    template for (constexpr auto type : std::define_static_array(children_of(dealias(^^BaseType)))) {
        using Type = [:type:];
        using PointerType = Type const*;
        pointer_types.push_back(dealias(^^PointerType));
    }
    return pointer_types;
}

template<typename T>
struct RevealType;

template<typename T>
[[nodiscard]] auto try_downcast(T const& object) -> auto {
    static constexpr auto child_types = std::define_static_array(
        [] {
            auto children = child_pointer_types<T>();
            children.push_back(^^typename [: dealias(^^std::decay_t<T>) :] const*);
            return children;
        }()
    );
    using Result = [: substitute(^^std::variant, child_types) :];

    template for (constexpr auto child_type : child_types) {
        auto const downcasted = dynamic_cast<[: child_type :]>(std::addressof(object));
        if (downcasted != nullptr) {
            return Result{ downcasted };
        }
    }

    return Result{ std::addressof(object) };
}

template<typename T>
auto pretty_print(
    T&& object,
    usize const indentation = 0uz,
    bool const print_ending_newline = true
) -> void {
    using Type = std::decay_t<T>;

    if constexpr (std::integral<Type> or std::floating_point<Type> or std::same_as<Type, bool>) {
        std::print("{}", object);
    } else if constexpr (std::same_as<Type, std::string> or std::same_as<Type, char const*>) {
        std::print("\"{}\"", object);
    } else if constexpr (std::is_enum_v<Type>) {
        std::print("{}::{}", identifier_of(dealias(^^Type)), utils::enum_to_string(object));
    } else if constexpr (requires { { object.has_value() }; { object.value() }; }) {
        if (object.has_value()) {
            pretty_print(object.value(), indentation, false);
        } else {
            std::print("null");
        }
    } else if constexpr (requires { { *object }; }) {
        if (object == nullptr) {
            std::print("null");
            return;
        }
        std::print("&");
        pretty_print(*object, indentation, false);
    } else if constexpr (std::ranges::range<Type>) {
        std::println("[");
        for (auto const& element : object) {
            std::print("{:{}}", "", indentation + 2uz);
            pretty_print(element, indentation + 2uz, false);
            std::println(",");
        }
        std::print("{:{}}]", "", indentation);
    } else if constexpr (is_class_type(dealias(^^Type)) and parent_of(^^Type) != ^^std) {
        auto const maybe_downcasted = try_downcast(object);
        auto const can_be_downcasted = not std::holds_alternative<typename [: dealias(^^Type) :] const*>(maybe_downcasted);

        if constexpr (has_identifier(dealias(^^Type))) {
            std::println("{}{{", identifier_of(dealias(^^Type)));
        } else {
            // std::println("<anonymous type>{{");
            std::println("{}{{", display_string_of(dealias(^^Type)));
        }
        static constexpr auto context = std::meta::access_context::current();
        template for (constexpr auto member : std::define_static_array(nonstatic_data_members_of(dealias(^^Type), context))) {
            std::print("{:{}}{}: ", "", indentation + 2uz, identifier_of(member));
            pretty_print(object.[: member :], indentation + 2uz, false);
            std::println(",");
        }

        template for (constexpr auto member : std::define_static_array(members_of(dealias(^^Type), context))) {
            if constexpr (
                has_identifier(member)
                and is_function(member)
                and is_const(member)
                and requires { { object.[: member :]() }; }
            ) {
                std::print("{:{}}{}(): ", "", indentation + 2uz, identifier_of(member));
                auto&& value = object.[: member :]();
                pretty_print(std::forward<decltype(value)>(value), indentation + 2uz, false);
                std::println(",");
            }
        }
        std::print("{:{}}}}", "", indentation);

        if (can_be_downcasted) {
            std::visit(
                [&](auto const child_pointer) {
                    pretty_print(*child_pointer, indentation, false);
                },
                maybe_downcasted
            );
        }
    }
    if (print_ending_newline) {
        std::println();
    }
}
