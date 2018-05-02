#include "Term.h"
#include "gtest/gtest.h"

namespace {
TEST(PolynomialTest, AddEdgeTest) {
  Polynomial poly;
  
    EXPECT_EQ ( true, poly.AddEdge(1, 0.2));
	EXPECT_EQ ( false, poly.AddEdge(1, 0.2));  // Adding the same edge twice 
	EXPECT_EQ ( false, poly.AddEdge(-2, 0.2)); // Edge index is negative
	EXPECT_EQ ( false, poly.AddEdge(1, 1.4));   // Probability is greater than 1
	EXPECT_EQ ( false, poly.AddEdge(44, 0.2));  //Edge index is greater than number of nodes
  
  
}
} 