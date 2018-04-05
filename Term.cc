#include "Term.h"

Term::Term() {
  coef_ = 1.0;
  X_.reset();
  Y_.set();
}

void Term::Multiply(int edge_index, double p, bool isPresent) {
  if (isPresent) {
    X_.set(edge_index);
    coef_ *= p;
  } else {
    Y_.reset(edge_index);
    coef_ *= (1 - p);
  }
}