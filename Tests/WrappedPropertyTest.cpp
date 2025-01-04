#include <gtest/gtest.h>
#include <vtutil/vtutil.h>

TEST(math, add)
{
  EXPECT_EQ(2, 2);

  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}