#ifndef __AUDACITY_VARIANT_HPP__
#define __AUDACITY_VARIANT_HPP__

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>
#include <variant>
#include <stdexcept>

namespace Variant {
    namespace detail {
        /** Help to define Visit() below. */
        template <typename Visitor, typename Variant>
        struct VisitHelperReturn {
            using Var = std::remove_reference_t<Variant>;
            using Alt = std::variant_alternative_t<0, Var>;
            using QAlt = std::conditional_t<
                std::is_const_v<Var>, const Alt, Alt>;
            using Arg = std::conditional_t<std::is_lvalue_reference_v<Variant>,
                                           std::add_lvalue_reference_t<QAlt>,
                                           std::add_rvalue_reference_t<QAlt>>;
            using type = decltype(std::invoke(std::declval<Visitor &&>(), std::declval<Arg>()));
        };

        /** Help to define Visit() below. */
        template <typename Visitor, typename Variant>
        [[noreturn]] auto VisitHelperBad(Visitor &&, Variant &&) ->
            typename VisitHelperReturn<Visitor &&, Variant &&>::type {
            /**
             * Fail through here when the variant holds no value.
             * Should really throw std::bad_variant_access but that may not be available.
             */
            throw std::invalid_argument{"Bad variant"};
        }

        /** Help to define Visit() below. */
        template <size_t Index, typename Visitor, typename Variant>
        decltype(auto) VisitHelperFunction(Visitor &&visitor, Variant &&variant) {
            /** Invoke visitor at most once. */
            const auto pValue = std::get_if<Index>(&variant);

            /** Trust VisitHelper to dispatch correctly. */
            assert(pValue);
            if constexpr (std::is_lvalue_reference_v<Variant>) {
                return std::invoke(std::forward<Visitor>(visitor), (*pValue));
            } else {
                return std::invoke(std::forward<Visitor>(visitor), std::move(*pValue));
            }
        }

        /** Help to define Visit() below. */
        template <size_t Index, typename Visitor, typename Variant>
        auto TypeCheckedVisitHelperFunction(Visitor &&visitor, Variant &&variant)
            -> typename VisitHelperReturn<Visitor &&, Variant &&>::type {
            static_assert(std::is_same_v<typename VisitHelperReturn<Visitor &&, Variant &&>::type,
                                         std::invoke_result_t<decltype(VisitHelperFunction<Index, Visitor &&, Variant &&>),
                                                              Visitor &&, Variant &&>>,
                          "Visitor must return the same type for all alternatives of the Variant.");
            return VisitHelperFunction<Index>(std::forward<Visitor>(visitor), std::forward<Variant>(variant));
        }

        /** Help to define Visit() below. */
        template <size_t ... Indices, typename Visitor, typename Variant>
        decltype(auto)
        VisitHelper(std::index_sequence<Indices ...>, Visitor&& visitor, Variant&& variant) {
            constexpr auto size = sizeof...(Indices);
            using Return = typename VisitHelperReturn<Visitor &&, Variant &&>::type;
            using Function = Return (*)(Visitor &&, Variant &&);
            static constexpr std::array<Function, size + 1> jumpTable{
                TypeCheckedVisitHelperFunction<Indices> ...,
                VisitHelperBad};
            auto function = jumpTable[std::min(variant.index(), size)];
            return function(std::forward<Visitor>(visitor), std::forward<Variant>(variant));
        }

        /** Standard in C++20. */
        template <typename T>
        struct type_identity {
            using type = T;
        };

        /** Unevaluated. */
        auto deduce_variant(...) -> void;

        template <typename ... Types>
        auto deduce_variant(std::variant<Types ...>& variant) ->
            type_identity<std::remove_reference_t<decltype(variant)>>;

        template <typename ... Types>
        auto deduce_variant(std::variant<Types ...>&& variant) ->
            type_identity<std::remove_reference_t<decltype(variant)>>;

        template <typename ... Types>
        auto deduce_variant(const std::variant<Types ...>& variant) ->
            type_identity<std::remove_reference_t<decltype(variant)>>;

        template <typename ... Types>
        auto deduce_variant(const std::variant<Types ...>&& variant) ->
            type_identity<std::remove_reference_t<decltype(variant)>>;

        template <typename T> using deduced_variant =
            typename decltype(deduce_variant(std::declval<T>()))::type;

        template <typename ForwardType, typename Variant>
        decltype(auto) forward_variant(Variant& variant) {
            return std::forward<ForwardType(variant)>;
        }

        template <typename ForwardType, typename Variant>
        decltype(auto) forward_variant(const Variant& variant) {
            return std::forward<ForwardType>(variant);
        }
    }

    /**
     * Mimic some of std::visit, for the case of one visitor only
     * This is necessary because of limitations of the macOS implementation of
     * some of the C++17 standard library without a minimum version of 10.13, and
     * so let's use this even when not needed on the other platforms, instead of
     * having too much conditional compilation
     */
    template<typename Visitor, typename Variant,
        typename VariantBase = detail::deduced_variant<Variant&&>>
    decltype(auto) Visit(Visitor&& visitor, Variant&& variant) {
        using ForwardType = std::conditional_t<std::is_lvalue_reference_v<Variant>,
                                               std::add_lvalue_reference_t<VariantBase>, VariantBase>;
        constexpr auto size = std::variant_size_v<VariantBase>;
        return detail::VisitHelper(std::make_index_sequence<size> {},
                                   std::forward<Visitor>(visitor),
                                   detail::forward_variant<ForwardType>(variant));
    }
}

#endif
