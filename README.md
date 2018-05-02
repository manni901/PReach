# PReach: Calculating PathReachability in Probabilistic Networks
  This program offers 3 methods for calculating path reachability in probabilistic networks. A probabilistic network can be modelled as a graph with edge weights denoting the probability of that edge to occur.
  Please read the wiki for for information.
  
# Code
  To build the code and run a sampe example with different methods run the following commands.
```
$ make main
$ ./main test.txt test-sources.txt test-targets.txt random
$ ./main test.txt test-sources.txt test-targets.txt sausage
$ ./main test.txt test-sources.txt test-targets.txt sample-random 0.8 1000
$ ./main test.txt test-sources.txt test-targets.txt sample-fixed 0.8 1000 10 10
$ ./main test.txt test-sources.txt test-targets.txt sample-weighted 0.8 1000 10 10

General structure
$ ./main {network-file} {sources-file} {target-file} {method-name} {success-probability} {num-iterations} {probe-size} {probe-repeat}
```
