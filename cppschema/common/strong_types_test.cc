#include <map>
#include <set>
#include <string>

#include "cppschema/common/strong_types.h"
#include "gtest/gtest.h"

namespace {

// Define some strong types for testing
DEFINE_STRONG_UINT_TYPE(NodeId);
DEFINE_STRONG_UINT_TYPE(EdgeId);
DEFINE_STRONG_STRING_TYPE(Username);

TEST(StrongTypeTest, ConstructionAndAccess) {
    NodeId n1(42);
    EdgeId e1(100);

    EXPECT_EQ(n1.value, 42);
    EXPECT_EQ(e1.value, 100);

    // The following line should not compile, as they are different types
    // n1 = e1;
}

TEST(StrongTypeTest, ComparisonOperators) {
    NodeId n1(42);
    NodeId n2(42);
    NodeId n3(43);

    EXPECT_TRUE(n1 == n2);
    EXPECT_FALSE(n1 == n3);
    EXPECT_TRUE(n1 != n3);
    EXPECT_FALSE(n1 != n2);
}

TEST(StrongTypeTest, ExplicitConversion) {
    NodeId n(42);
    EdgeId e(100);

    // Explicit conversion to underlying type
    EXPECT_EQ(static_cast<uint32_t>(n), 42);
    EXPECT_EQ(static_cast<uint32_t>(e), 100);
}

TEST(StrongTypeTest, UseAsMapKey) {
    std::map<NodeId, std::string> nodeNames;
    nodeNames[NodeId(1)] = "Node 1";
    nodeNames[NodeId(2)] = "Node 2";

    EXPECT_EQ(nodeNames[NodeId(1)], "Node 1");
    EXPECT_EQ(nodeNames[NodeId(2)], "Node 2");
}

TEST(StrongTypeTest, UseAsSetElements) {
    std::set<NodeId> nodeSet;
    nodeSet.insert(NodeId(1));
    nodeSet.insert(NodeId(2));

    EXPECT_TRUE(nodeSet.find(NodeId(1)) != nodeSet.end());
    EXPECT_TRUE(nodeSet.find(NodeId(2)) != nodeSet.end());
    EXPECT_TRUE(nodeSet.find(NodeId(3)) == nodeSet.end());
}

TEST(StrongTypeTest, StrongStringType) {
    Username user1("alice");
    Username user2("bob");
    Username user3("alice");

    EXPECT_TRUE(user1 == user3);
    EXPECT_FALSE(user1 == user2);
}

}  // namespace
