#include "../Callable.hpp"

#include <catch2/catch_all.hpp>
#include <variant>
#include <cstdint>

/** See also VariantTest for more exercise of OverloadSet. */

using namespace Callable;

namespace {
    struct X {
        int32_t member{0};

        X() = default;
        explicit X(int32_t value)
            : member{value} {}
        explicit X(int32_t value, std::shared_ptr<X>)
            : member{value} {}
    };

    struct TestVisitor {
        static int32_t x;
        int32_t &operator()(std::monostate) const { return x; }
    };

    int32_t TestVisitor::x{};

    template <auto T>
    struct TakesNonTypeParameter{};
}

TEST_CASE("Compilation") {
    {
        constexpr auto visitor_1 = OverloadSet{TestVisitor{}, &X::member},
                       visitor_2{visitor_1},
                       visitor_3{OverloadSet{TestVisitor{}, &X::member}};
        constexpr auto visitor_4{OverloadSet<TestVisitor>{}};
    }

    {
        /** These function objects are of literal types. */
        constexpr auto f1 = UniquePtrFactory<X>::Function;
        constexpr auto f2 = UniquePtrFactory<X, int>::Function;
        /** How to get multiple signatures. */
        constexpr auto f3 = OverloadSet{f1, f2};
        constexpr auto f4 = UniquePtrFactory<X, int, std::unique_ptr<X>>::Function;

        {
            auto p1 = f1();
            REQUIRE(p1->member == 0);
            auto p2 = f2(1);
            REQUIRE(p2->member == 1);

            /** Demonstrate move of argument. */
            auto p3 = f4(2, std::move(p2));
            REQUIRE(p3->member == 2);
            REQUIRE(!p2);
        }

        {
            auto p1 = f3();
            REQUIRE(p1->member == 0);
            auto p2 = f3(1);
            REQUIRE(p2->member == 1);
        }

        TakesNonTypeParameter<f1> t1{};
        TakesNonTypeParameter<f2> t2{};
    }

    {
        /** These function objects are of literal types. */
        constexpr auto f1 = SharedPtrFactory<X>::Function;
        constexpr auto f2 = SharedPtrFactory<X, int32_t>::Function;
        /** How to get multiple signatures. */
        constexpr auto f3 = OverloadSet{f1, f2};
        constexpr auto f4 = UniquePtrFactory<X, int32_t, std::shared_ptr<X>>::Function;

        {
            auto p1 = f1();
            REQUIRE(p1->member == 0);
            auto p2 = f2(1);
            REQUIRE(p2->member == 1);

            /** Demonstrate move of argument. */
            auto p3 = f4(2, std::move(p2));
            REQUIRE(p3->member == 2);
            REQUIRE(!p2);
        }

        {
            auto p1 = f3();
            REQUIRE(p1->member == 0);
            auto p2 = f3(1);
            REQUIRE(p2->member == 1);
        }

        TakesNonTypeParameter<f1> t1{};
        TakesNonTypeParameter<f2> t2{};
    }

    {
        /** These function objects are of literal types. */
        constexpr auto f1 = Constantly<0>::Function;
        constexpr auto f2 = Constantly<0, int32_t>::Function;
        /** How to get multiple signatures. */
        constexpr auto f3 = OverloadSet{f1, f2};

        REQUIRE(f1() == 0);
        REQUIRE(f2(1) == 0);

        REQUIRE(f3() == 0);
        REQUIRE(f3(1) == 0);

        TakesNonTypeParameter<f1> t1{};
        TakesNonTypeParameter<f2> t2{};
    }

    {
        constexpr auto f1 = UniqueMaker<X>();
        constexpr auto f2 = UniqueMaker<X, int32_t>();

        constexpr auto f3 = OverloadSet{f1, f2};
        constexpr auto f4 = UniqueMaker<X, int32_t, std::shared_ptr<X>>();

        {
            auto p1 = f1();
            REQUIRE(p1->member == 0);
            auto p2 = f2(1);
            REQUIRE(p2->member == 1);

            auto p3 = f4(2, std::move(p2));
            REQUIRE(p3->member == 2);
            REQUIRE(!p2);

            auto p4 = f2({});
            auto p5 = f1(0);
        }

        {
            auto p1 = f3();
            REQUIRE(p1->member == 0);
            auto p2 = f3(1);
            REQUIRE(p2->member == 1);
        }
    }
}
