#include "Polynomial.h"

void Polynomial::AddEdge(int edge_index, double p) {
  // a new vector to accumulate the new terms
  vector<Term> new_terms;
  for (auto &term : terms_) {
    // Multiply by X term
    Term xTerm = term;
    xTerm.Multiply(edge_index, p, false);
    new_terms.push_back(xTerm);
    // Multiply by Y term
    Term yTerm = term;
    yTerm.Multiply(edge_index, p, true);
    new_terms.push_back(yTerm);
  }
  // swap newTerms with terms
  new_terms.swap(terms_);
}

void Polynomial::Collapse(Edges &mid_edges, Nodes &end_nodes,
                          EDGE_INFO &edge_terminals) {
  vector<Term> new_terms;
  for (auto &term : terms_) {
    // check collapsing of term
    Nodes z; // will hold the nodes to which the term collapses (Z)
    bool collapsed = term.Collapse(mid_edges, end_nodes, edge_terminals, z);
    if (collapsed) { // term DOES collapse to z
      // Now we find the corresponding endTerm, or create it
      Term end_term;
      end_term.SetCoefficient(0.0);
      string end_term_id = z.to_string();
      if (end_terms_.find(end_term_id) != end_terms_.end()) {
        // endTerm found
        end_term = end_terms_[end_term_id];
      } else {
        // endTerm not found, create it
        // Nodes w = end_nodes & ~z;
        end_term.SetReachableNodes(z); // Term(z, w)
      }
      end_term.AddCoefficient(term.GetCoefficient());
      end_terms_[end_term_id] = end_term;
    } else { // term DOES NOT collapse
      new_terms.push_back(term);
    }
  }
  new_terms.swap(terms_); // replace terms with the new collapsed terms
  //cout << "Number of terms: " << terms_.size() << " End Terms: " << end_terms_.size() << "\n";
}

double Polynomial::GetResult() {
  // SANITY CHECKS
  //assert(terms_.size() == 2);
  double totalCoeff =
      terms_.front().GetCoefficient() + terms_.back().GetCoefficient();
 // assert(totalCoeff < 1.01 && totalCoeff > 0.99);

  if (terms_.front().HasReachableNodes()) {
    //assert(!terms_.back().HasReachableNodes());
    return terms_.front().GetCoefficient();
  } else {
    //assert(terms_.back().HasReachableNodes());
    return terms_.back().GetCoefficient();
  }
}

// Advances the polynomial: prepares it for the next cut.
// Transfers endTerms to terms, and reinitializes endTerms
void Polynomial::Advance() {
  // SANITY CHECK
  //assert(terms_.size() == 0);
  // copy endTerms to terms
  terms_ = vector<Term>();
  double totalCoeff = 0.0;
  for (auto& term : end_terms_) {
    totalCoeff += term.second.GetCoefficient();
    terms_.push_back(term.second);
  }
  // reinitialize endTerms
  end_terms_.clear();

  // SANITY CHECK
  //assert(totalCoeff < 1.01 && totalCoeff > 0.99);
}