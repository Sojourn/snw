#include "catch.hpp"
#include "intrusive_list.h"
#include <vector>

struct node {
    int value{ 0 };
    snw::intrusive_list_node il_node{};
};

struct node_a : node {
};

struct node_b : node {
};

using list = snw::intrusive_list<node, &node::il_node>;

bool equals(const list& lhs, std::initializer_list<int> rhs) {
    std::vector<int> lhs_values;
    std::vector<int> rhs_values{rhs};

    for(const node& node: lhs) {
        lhs_values.push_back(node.value);
    }

    return lhs_values == rhs_values;
}

TEST_CASE("empty list") {
    list l;
    CHECK(l.empty());
}

TEST_CASE("push back") {
    list l;
    node n1{ 1 };
    node n2{ 2 };

    l.push_back(n1);
    CHECK(!l.empty());
    CHECK(l.front().value == 1);
    CHECK(l.back().value == 1);

    l.push_back(n2);
    CHECK(!l.empty());
    CHECK(l.front().value == 1);
    CHECK(l.back().value == 2);
}

TEST_CASE("push front") {
    list l;
    node n1{ 1 };
    node n2{ 2 };

    l.push_front(n1);
    REQUIRE(equals(l, {1}));

    l.push_front(n2);
    REQUIRE(equals(l, {2, 1}));
}

TEST_CASE("insert at beginning of empty list") {
    list l;
    node n1{ 1 };

    l.insert(l.begin(), n1);
    CHECK(equals(l, {1}));
}

TEST_CASE("insert at beginning of non-empty list") {
    list l;
    node n1{ 1 };
    node n2{ 2 };

    l.push_back(n1);
    l.insert(l.begin(), n2);
    CHECK(equals(l, {2, 1}));
}

TEST_CASE("insert at end of empty list") {
    list l;
    node n1{ 1 };

    l.insert(l.end(), n1);
    CHECK(equals(l, {1}));
}

TEST_CASE("insert at end of non-empty list") {
    list l;
    node n1{ 1 };
    node n2{ 2 };

    l.push_back(n1);
    l.insert(l.end(), n2);
    CHECK(equals(l, {1, 2}));
}

TEST_CASE("insert in the middle of a list") {
    list l;
    node n1{ 1 };
    node n2{ 2 };
    node n3{ 3 };

    l.push_back(n1);
    l.push_back(n2);
    l.insert(++l.begin(), n3);

    CHECK(equals(l, {1, 3, 2}));
}

TEST_CASE("explicit clear") {
    node n1{ 1 };
    node n2{ 2 };
    node n3{ 3 };

    list l;
    l.push_back(n1);
    l.push_back(n2);
    l.push_back(n3);

    CHECK(equals(l, {1, 2, 3}));

    l.clear();

    CHECK(l.empty());
    CHECK(!n1.il_node.is_linked());
    CHECK(!n2.il_node.is_linked());
    CHECK(!n3.il_node.is_linked());
}

TEST_CASE("implicit clear") {
    node n1{ 1 };
    node n2{ 2 };
    node n3{ 3 };

    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        REQUIRE(equals(l, {1, 2, 3}));
    }

    CHECK(!n1.il_node.is_linked());
    CHECK(!n2.il_node.is_linked());
    CHECK(!n3.il_node.is_linked());
}

TEST_CASE("implicit unlink") {
    list l;

    REQUIRE(l.empty());
    {
        node n1{ 1 };
        l.push_back(n1);
        REQUIRE(equals(l, {1}));
        {
            node n2{ 2 };
            l.push_back(n2);
            REQUIRE(equals(l, {1, 2}));
            {
                node n3{ 3 };
                l.push_back(n3);
                REQUIRE(equals(l, {1, 2, 3}));
            }
            REQUIRE(equals(l, {1, 2}));
        }
        REQUIRE(equals(l, {1}));
    }
    REQUIRE(l.empty());
}

TEST_CASE("explicit unlink") {
    node n1{ 1 };
    node n2{ 2 };
    node n3{ 3 };

    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        n1.il_node.unlink();
        CHECK(equals(l, {2, 3}));
    }
    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        n2.il_node.unlink();
        CHECK(equals(l, {1, 3}));
    }
    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        n3.il_node.unlink();
        CHECK(equals(l, {1, 2}));
    }
}

TEST_CASE("erase") {
    node n1{ 1 };
    node n2{ 2 };
    node n3{ 3 };

    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        l.erase(l.begin());
        CHECK(equals(l, {2, 3}));
    }
    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        l.erase(++l.begin());
        CHECK(equals(l, {1, 3}));
    }
    {
        list l;
        l.push_back(n1);
        l.push_back(n2);
        l.push_back(n3);

        l.erase(++(++l.begin()));
        CHECK(equals(l, {1, 2}));
    }
}
