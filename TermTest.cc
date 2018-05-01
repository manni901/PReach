#include "Term.h"
#include "gtest/gtest.h"

namespace {
TEST(TermTest, SimpleMultiplyTest) {
  Term term;
  term.Multiply(0, 0.8, true /*is_present*/);
  term.Multiply(1, 0.6, false /*is_present*/);

  double expected_coeff = 0.8 * (1 - 0.6);
  EXPECT_EQ(term.GetCoefficient(), expected_coeff);
  EXPECT_EQ(term.GetPresentCount(), 1);
  EXPECT_EQ(term.GetAbsentCount(), NUM_EDGES - 1);
}
} // namespace