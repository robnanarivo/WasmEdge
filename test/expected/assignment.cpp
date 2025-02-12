// SPDX-License-Identifier: CC0-1.0
#include "gtest/gtest.h"
#include <experimental/expected.hpp>

using cxx20::expected;
using cxx20::unexpected;

TEST(AssignmentTest, SimpleAssignment) {
  expected<int, int> e1 = 42;
  expected<int, int> e2 = 17;
  expected<int, int> e3 = 21;
  expected<int, int> e4 = unexpected(42);
  expected<int, int> e5 = unexpected(17);
  expected<int, int> e6 = unexpected(21);

  e1 = e2;
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 17);
  EXPECT_TRUE(e2);
  EXPECT_EQ(*e2, 17);

  e1 = std::move(e2);
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 17);
  EXPECT_TRUE(e2);
  EXPECT_EQ(*e2, 17);

  e1 = 42;
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 42);

  auto unex = unexpected(12);
  e1 = unex;
  EXPECT_FALSE(e1);
  EXPECT_EQ(e1.error(), 12);

  e1 = unexpected(42);
  EXPECT_FALSE(e1);
  EXPECT_EQ(e1.error(), 42);

  e1 = e3;
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 21);

  e4 = e5;
  EXPECT_FALSE(e4);
  EXPECT_EQ(e4.error(), 17);

  e4 = std::move(e6);
  EXPECT_FALSE(e4);
  EXPECT_EQ(e4.error(), 21);

  e4 = e1;
  EXPECT_TRUE(e4);
  EXPECT_EQ(*e4, 21);
}

TEST(AssignmentTest, AssignmentDeletion) {
  struct has_all {
    has_all() noexcept = default;
    has_all(const has_all &) noexcept = default;
    has_all(has_all &&) noexcept = default;
    has_all &operator=(const has_all &) noexcept = default;
  };

  EXPECT_TRUE((std::is_assignable_v<expected<has_all, has_all> &,
                                    expected<has_all, has_all>>));

  struct except_move {
    except_move() noexcept = default;
    except_move(const except_move &) noexcept = default;
    except_move(except_move &&) noexcept(false) {}
    except_move &operator=(const except_move &) noexcept = default;
  };
  EXPECT_FALSE((std::is_assignable_v<expected<except_move, except_move> &,
                                     expected<except_move, except_move>>));
}

TEST(AssignmentTest, AssignmentThrowRecovery) {
  struct throw_move {
    int v;
    throw_move(int v) noexcept : v(v) {}
    throw_move(const throw_move &) noexcept = default;
    throw_move(throw_move &&) noexcept(false) { throw 0; }
    throw_move &operator=(const throw_move &) noexcept = default;
    throw_move &operator=(throw_move &&) noexcept(false) { throw 0; }
  };

  {
    expected<throw_move, int> e1 = 1;
    expected<throw_move, int> e2 = 2;
    expected<throw_move, int> e3 = unexpected(3);

    EXPECT_TRUE(e1);
    EXPECT_EQ(e1->v, 1);
    EXPECT_TRUE(e2);
    EXPECT_EQ(e2->v, 2);
    EXPECT_FALSE(e3);
    EXPECT_EQ(e3.error(), 3);

    EXPECT_NO_THROW(e1 = e2);
    EXPECT_TRUE(e1);
    EXPECT_EQ(e1->v, 2);
    EXPECT_TRUE(e2);
    EXPECT_EQ(e2->v, 2);
    EXPECT_NO_THROW(e3 = e2);
    EXPECT_TRUE(e3);
    EXPECT_EQ(e3->v, 2);
    EXPECT_TRUE(e2);
    EXPECT_EQ(e2->v, 2);
  }

  {
    expected<throw_move, int> e1 = 1;
    expected<throw_move, int> e2 = 2;
    expected<throw_move, int> e3 = unexpected(3);

    EXPECT_TRUE(e1);
    EXPECT_EQ(e1->v, 1);
    EXPECT_FALSE(e3);
    EXPECT_EQ(e3.error(), 3);

    EXPECT_ANY_THROW(e1 = std::move(e2));
    EXPECT_TRUE(e1);
    EXPECT_EQ(e1->v, 1);
    EXPECT_TRUE(e2);
    EXPECT_EQ(e2->v, 2);
    EXPECT_ANY_THROW(e3 = std::move(e2));
    EXPECT_FALSE(e3);
    EXPECT_EQ(e3.error(), 3);
    EXPECT_TRUE(e2);
    EXPECT_EQ(e2->v, 2);
  }
}
