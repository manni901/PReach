#include <vector>
#include <bitset>
#include <iostream>
#include <assert.h>
#include <typeinfo>
#include <sstream>

#include <lemon/list_graph.h>
#include <lemon/bfs.h>

#include <fstream>
#include <map>
#include <ctime>
#include <sys/time.h>

#include <sys/time.h> // for gettimeofday
#include <stdlib.h> // drand48

#include "Stl.h"

#define MAX_NODES 512
#define MAX_EDGES 6144

using namespace std;

typedef bitset<MAX_NODES> Nodes_T;
typedef bitset<MAX_EDGES> Edges_T;
typedef unsigned long ulong;

// macro to smoth up the use of bitsets
#define FOREACH_BS(v, vSet)	  \
	for (size_t v=vSet._Find_first(); v!=vSet.size(); v=vSet._Find_next(v))

/*A class modeling a vertex cut*/
class Cut{
    Nodes_T middle; // The nodes in the
    Nodes_T left; // Set of nodes on the left
    Nodes_T right; // Set of nodes on the right
    Edges_T coveredEdges; // Set of edges covered by this cut (left and middle)

public:
    Cut(){}

    // consturctor for a specified cut
	Cut(Nodes_T& _left, Nodes_T& _middle, Nodes_T& _right, Edges_T& _covered):
		left(_left), middle(_middle), right(_right), coveredEdges(_covered){}

    Nodes_T& getMiddle(){return middle;}
    Nodes_T& getRight(){return right;}
    Nodes_T& getLeft(){return left;}
    Edges_T& getCoveredEdges(){return coveredEdges;}
    int size(){return middle.count();}
};

class Term{
    Nodes_T z; // Reachable nodes
    Nodes_T w; // Unreachable nodes
    Edges_T x; // Present edges
    Edges_T y; // Absent edges
    double coeff; // Duh!

public:
    Term(){}

    //Term(Nodes_T& _z, Nodes_T& _w, Edges_T& _x, Edges_T& _y, double _coeff):
    //    z(_z), w(_w), x(_x), y(_y), coeff(_coeff){}

    Term(Nodes_T& _z, Nodes_T& _w, double _coeff):
        z(_z), w(_w), coeff(_coeff){}

    Term(Nodes_T& _z, Nodes_T& _w):
        z(_z), w(_w){
            coeff = 0.0;
        }

    /*Multiply by a new edge term*/
    void multiply(int subscript, double p, bool inverse){
        if (inverse){
            y.set(subscript);
            coeff *= (1-p);
        }else{
            x.set(subscript);
            coeff *= p;
        }
    }

    double getCoeff(){return coeff;}
    void addToCoeff(double increment){coeff += increment;}
    bool hasZ(){return z.any();}

    /*Checks for collapsing, returns the set of nodes it collapses to in newZ
    returns true if collapses*/
    bool collapse(Edges_T& midEdges, map< int, vector<int> >& edgeTerminals, Nodes_T& endNodes, Nodes_T& newZ){
        int edges[MAX_EDGES][2];
        int count;
        vector<int> terminals;
        Nodes_T visited;
        Nodes_T copy;

        /////FIRST: traverse the x edges and see which end nodes are reachable
        count = 0;
        FOREACH_BS(edge, x){ // form the list of edges to traverse
            terminals = edgeTerminals[edge];
            edges[count][0] = terminals[0];
            edges[count][1] = terminals[1];
            count ++;
        }
        visited = z;
        while (true){ // traverse the edges, setting targets as true until nothing changes
            copy = visited;
            for (int i=0; i<count; i++){
                if (visited[edges[i][0]]){
                    visited.set(edges[i][1]);
                }
            }
            if (copy == visited) break;
        }
        Nodes_T reachable = endNodes & visited; // This is now the set of sure reachable nodes

        /////SECOND: traverse all edges except y and see which end nodes are unreachable
        Edges_T yInverse = midEdges & ~y;
        count = 0;
        FOREACH_BS(edge, yInverse){
            terminals = edgeTerminals[edge];
            edges[count][0] = terminals[0];
            edges[count][1] = terminals[1];
            count ++;
        }
        visited = z;
        while (true){ // traverse the edges, setting targets as true until nothing changes
            copy = visited;
            for (int i=0; i<count; i++){
                if (visited[edges[i][0]]){
                    visited.set(edges[i][1]);
                }
            }
            if (copy == visited) break;
        }
        Nodes_T unreachable = endNodes & ~visited; // This is now the set of sure unreachable nodes

        /////LAST: if all endNodes are either reachable or unreachable: collapsed
        /////else (at least one node is neither reachable nor unreachable): doesn't collapse
        if (endNodes == (reachable|unreachable)){
            newZ = reachable;
            return true;
        }else{
            return false;
        }
    }
};

class Polynomial{
    vector<Term> terms; // regular terms with start nodes and middle edges
    map<string, Term> endTerms; // black holes with end nodes, the key is z nodes as ulong

public:
    Polynomial(vector<Term>& _terms): terms(_terms){}

    /*Adds and edge: mutiplies the whole polynomial by
    pX_subscript + (1-p)Y_subscript
    DOES NOT COLLAPSE AUTOMATICALLY*/
    void addEdge(int subscript, double p){
        //a new vector to accumulate the new terms
        vector<Term> newTerms;
        FOREACH_STL(term, terms){
            //Multiply by X term
            Term xTerm = term;
            xTerm.multiply(subscript, p, false);
            newTerms.push_back(xTerm);
            //Multiply by Y term
            Term yTerm = term;
            yTerm.multiply(subscript, p, true);
            newTerms.push_back(yTerm);
        }END_FOREACH;
        //swap newTerms with terms
        newTerms.swap(terms);
    }

    /*Advances the polynomial: prepares it for the next cut.
    transfers endTerms to terms, and reinitializes endTerms*/
    void advance(){
        //SANITY CHECK
        assert(terms.size() == 0);
        //copy endTerms to terms
        terms = vector<Term>();
        double totalCoeff = 0.0;
        for (map<string, Term>::iterator iter = endTerms.begin(); iter != endTerms.end(); ++iter){
            totalCoeff += iter->second.getCoeff();
            terms.push_back(iter->second);
        }
        //reinitialize endTerms
        endTerms = map<string, Term>();

        //SANITY CHECK
        assert(totalCoeff < 1.01 && totalCoeff > 0.99);
    }

    double getResult(){
        //SANITY CHECKS
        assert(terms.size() == 2);
        double totalCoeff = terms.front().getCoeff() + terms.back().getCoeff();
        assert(totalCoeff < 1.01 && totalCoeff > 0.99);

        if (terms.front().hasZ()){
            assert(!terms.back().hasZ());
            return terms.front().getCoeff();
        }else{
            assert(terms.back().hasZ());
            return terms.back().getCoeff();
        }
    }

    /*Collapses the polynomial: iterates over each term
    to collapse it if possible.
    midEdges are all edges currently considered between
    the past cut and the next one.
    edgeTerminals is a hash from edge id to its source and target ids
    endNodes are the nodes in the next cut*/
    void collapse(Edges_T& midEdges, map< int, vector<int> >& edgeTerminals, Nodes_T& endNodes){
        vector<Term> newTerms;
        FOREACH_STL(term, terms){
            // check collapsing of term
            Nodes_T z; // will hold the nodes to which the term collapses (Z)
            bool collapsed = term.collapse(midEdges, edgeTerminals, endNodes, z);
            if (collapsed){ // term DOES collapse to z
                // Now we find the corresponding endTerm, or create it
                Term endTerm;
                string endTermId = z.to_string< char,char_traits<char>,allocator<char> >();
                if (endTerms.find(endTermId) != endTerms.end()){ // endTerm found
                    endTerm = endTerms[endTermId];
                }else{ // endTerm not found, create it
                    Nodes_T w = endNodes & ~z;
                    endTerm = Term(z, w);
                }
                endTerm.addToCoeff(term.getCoeff());
                endTerms[endTermId] = endTerm;
            }else { // term DOES NOT collapse
                newTerms.push_back(term);
            }
        }END_FOREACH;
        newTerms.swap(terms); //replace terms with the new collapsed terms
    }
};





