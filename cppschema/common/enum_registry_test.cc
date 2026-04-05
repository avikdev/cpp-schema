#include "cppschema/common/enum_registry.h"
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

using testing::Pair;

namespace {

// A global / file-scoped enum.
enum class BasicColorEnum {
    RED,
    GREEN,
    BLUE,
};

DEFINE_ENUM_CONVERSION_FUNCTION(BasicColorEnum, RED, GREEN, BLUE);

TEST(EnumRegistryTest, StringToEnumVal) {
    using ToEnumFunc = EnumRegistry::ToEnumFunc<BasicColorEnum>;
    const ToEnumFunc toEnum = EnumRegistry::instance().getToEnum<BasicColorEnum>();
    ASSERT_NE(toEnum, nullptr);

    EXPECT_EQ(toEnum("RED"), BasicColorEnum::RED);
    EXPECT_EQ(toEnum("GREEN"), BasicColorEnum::GREEN);
    EXPECT_EQ(toEnum("BLUE"), BasicColorEnum::BLUE);
    EXPECT_EQ(toEnum("INVALID"), std::nullopt);
}

TEST(EnumRegistryTest, EnumValToStringAndOrdinal) {
    using ToInfoFunc = EnumRegistry::ToInfoFunc<BasicColorEnum>;
    const ToInfoFunc toInfo = EnumRegistry::instance().getToInfo<BasicColorEnum>();
    ASSERT_NE(toInfo, nullptr);

    EXPECT_THAT(toInfo(BasicColorEnum::RED), Pair("RED", 0));
    EXPECT_THAT(toInfo(BasicColorEnum::GREEN), Pair("GREEN", 1));
    EXPECT_THAT(toInfo(BasicColorEnum::BLUE), Pair("BLUE", 2));
}

enum class SimilarEnum1 {
    A, B, C
};

DEFINE_ENUM_CONVERSION_FUNCTION(SimilarEnum1, A, B, C);

enum class SimilarEnum2 {
    X, Y, Z
};

DEFINE_ENUM_CONVERSION_FUNCTION(SimilarEnum2, X, Y, Z);

TEST(EnumRegistryTest, SimilarEnums) {
    using ToEnumFunc1 = EnumRegistry::ToEnumFunc<SimilarEnum1>;
    const ToEnumFunc1 toEnum1 = EnumRegistry::instance().getToEnum<SimilarEnum1>();
    ASSERT_NE(toEnum1, nullptr);

    using ToEnumFunc2 = EnumRegistry::ToEnumFunc<SimilarEnum2>;
    const ToEnumFunc2 toEnum2 = EnumRegistry::instance().getToEnum<SimilarEnum2>();
    ASSERT_NE(toEnum2, nullptr);

    EXPECT_EQ(toEnum1("A"), SimilarEnum1::A);
    EXPECT_EQ(toEnum1("B"), SimilarEnum1::B);
    EXPECT_EQ(toEnum1("C"), SimilarEnum1::C);
    EXPECT_EQ(toEnum1("X"), std::nullopt);

    EXPECT_EQ(toEnum2("X"), SimilarEnum2::X);
    EXPECT_EQ(toEnum2("Y"), SimilarEnum2::Y);
    EXPECT_EQ(toEnum2("Z"), SimilarEnum2::Z);
    EXPECT_EQ(toEnum2("A"), std::nullopt);
}

}  // namespace