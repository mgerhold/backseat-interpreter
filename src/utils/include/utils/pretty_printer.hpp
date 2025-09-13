#pragma once

#include <concepts>
#include <string>
#include <vector>
#include <ranges>
#include <experimental/meta>
#include <type_checker/statements.hpp>
#include <utils/enum_to_string.hpp>
#include <tl/optional.hpp>

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
[[nodiscard]] consteval auto get_parent() -> std::meta::info {
    static constexpr auto context = std::meta::access_context::current();
    static constexpr auto bases = std::define_static_array(bases_of(dealias(^^T), context));
    if constexpr (bases.empty()) {
        return ^^T;
    } else if constexpr (bases.size() > 1uz) {
        throw std::logic_error{ "Multiple inheritance is not supported." };
    } else {
        using Base = [: type_of(bases.front()) :];
        return get_parent<Base>();
    }
}

template<typename T>
consteval auto get_all_types_in_hierarchy(std::vector<std::meta::info>& types) -> void {
    template for (constexpr auto child : std::define_static_array(children_of(^^T))) {
        types.push_back(child);
        get_all_types_in_hierarchy<typename [: child :]>(types);
    }
}

template<typename T>
consteval auto get_all_types_in_hierarchy() -> std::vector<std::meta::info> {
    auto types = std::vector<std::meta::info>{};
    get_all_types_in_hierarchy<T>(types);
    return types;
}

[[nodiscard]] consteval auto get_inheritance_chain(std::meta::info base, std::meta::info derived) -> std::vector<std::meta::info> {
    static constexpr auto context = std::meta::access_context::current();
    auto const bases = bases_of(dealias(derived), context);
    if (is_same_type(base, derived)) {
        return std::vector{ base };
    }
    if (bases.empty()) {
        throw std::logic_error{ "Class has no base class." };
    }
    if (bases.size() != 1uz) {
        throw std::logic_error{ "Multiple inheritance is not supported." };
    }
    auto chain = get_inheritance_chain(base, type_of(bases.front()));
    chain.push_back(dealias(derived));
    return chain;
}

template<typename Derived>
[[nodiscard]] consteval auto get_inheritance_chain() -> std::vector<std::meta::info> {
    return get_inheritance_chain(get_parent<Derived>(), ^^Derived);
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
        using Parent = [: get_parent<Type>() :];
        [[maybe_unused]] Parent* _;
        auto type_index = 0uz;
        auto max_chain_length = std::optional<usize>{};
        auto max_chain_length_type_index = std::optional<usize>{};
        template for (constexpr auto type : std::define_static_array(get_all_types_in_hierarchy<typename [: get_parent<Type>() :]>())) {
            auto const maybe_downcasted = dynamic_cast<[: type :] const*>(std::addressof(object));
            if (maybe_downcasted != nullptr) {
                static constexpr auto chain = std::define_static_array(get_inheritance_chain<typename [: type :]>());
                static constexpr auto chain_size = chain.size();
                if (not max_chain_length.has_value() or max_chain_length.value() < chain_size) {
                    max_chain_length = chain_size;
                    max_chain_length_type_index = type_index;
                }
            }
            ++type_index;
        }

        type_index = 0uz;
        template for (constexpr auto type : std::define_static_array(get_all_types_in_hierarchy<typename [: get_parent<Type>() :]>())) {
            if (type_index == max_chain_length_type_index) {
                static constexpr auto chain = std::define_static_array(get_inheritance_chain<typename [: type :]>());
                auto is_first = true;
                template for (constexpr auto type_in_chain : chain) {
                    if (not is_first) {
                        std::print("<-");
                    }
                    std::print("{}", identifier_of(type_in_chain));
                    is_first = false;
                }
                std::println("{{");
                template for (constexpr auto type_in_chain : chain) {
                    static constexpr auto context = std::meta::access_context::current();
                    template for (constexpr auto member : std::define_static_array(nonstatic_data_members_of(dealias(type_in_chain), context))) {
                        std::print("{:{}}{}: ", "", indentation + 2uz, identifier_of(member));
                        pretty_print(object.[: member :], indentation + 2uz, false);
                        std::println(",");
                    }
                    auto const downcasted = dynamic_cast<[: type_in_chain :] const*>(std::addressof(object));
                    if (downcasted == nullptr) {
                        throw std::logic_error{ "Unreachable" };
                    }
                    auto const& object = *downcasted; // Shadowing the outer variable.
                    template for (constexpr auto member : std::define_static_array(members_of(dealias(type_in_chain), context))) {
                        if constexpr (
                            has_identifier(member)
                            and is_function(member)
                            and is_const(member)
                            and not is_pure_virtual(member)
                            and requires { { object.[: member :]() }; }
                        ) {
                            std::print("{:{}}{}(): ", "", indentation + 2uz, identifier_of(member));
                            auto&& value = object.[: member :]();
                            pretty_print(std::forward<decltype(value)>(value), indentation + 2uz, false);
                            std::println(",");
                        }
                    }
                }
                std::print("{:{}}}}", "", indentation);
            }
            ++type_index;
        }
    }
    if (print_ending_newline) {
        std::println();
    }
}
