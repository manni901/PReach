#include "Term.h"
#include "gtest/gtest.h"

namespace {
TEST(PolynomialTest, SimpleMultiplyTest) {
  Term term;
  term.Multiply(0, 0.8, true /*is_present*/);
  term.Multiply(1, 0.6, false /*is_present*/);

  double expected_coeff = 0.8 * (1 - 0.6);
  EXPECT_EQ(term.GetCoefficient(), expected_coeff);
  EXPECT_EQ(term.GetPresentCount(), 1);
  EXPECT_EQ(term.GetAbsentCount(), NUM_EDGES - 1);
  
  
  EXPECT_EQ ( true, term.Multiply (2,0.2,true));
  EXPECT_EQ ( true, term.Multiply (3,0.8,true));
  EXPECT_EQ (false, term.Multiply (2,0.1,false)); // Adding edge again 
  EXPECT_EQ (false, term.Multiply (5,13.1,false)); // Probability greater than 1
  EXPECT_EQ (false, term.Multiply (24,0.1,true)); //Edge index greater than total number of indices
  EXPECT_EQ (false, term.Multiply (-2,0.1,true)); //Edge index is negative
  EXPECT_EQ (false, term.Multiply (24,-0.3,true)); //Edge probability is negative
  
  
}
} // namespace