C code for calculating circuit controllability and observability measures for a given circuit


# Input Format

Circuit file, using a format similar to netlists. It should be easy enough to create a traversable graph of the circuit with resizing vectors.
```
INPUTS <number of input lines>
1 2 3 4 5 // numbering for every input line
OUTPUTS <number of output lines>
11 12 13 14 15 // number for every output line
NODES <number of nodes>
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 // list of every node
GATES <number of gates>
<gatetype> <number of inputs> <list of input lines> <output lines>
AND 3 1 2 3 11
OR 2 2 3 6
FANOUT 3 3 7 8 9 // this has 1 input [3] and 3 outputs [7, 8, 9]
// etc etc
```


Sample netlist:


```
INPUTS 5
1 2 3 4 5
OUTPUTS 5
11 12 13 14 15
NODES 15
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
GATES 3
AND 3 1 2 3 11
OR 2 2 3 6
FANOUT 3 3 7 8 9
```

# Summary

- The scope of this program is calculating controllability and observability measures for combinational and sequential circuits
- It performs breadth first search and depth first search for controllability and observability with calculation of the cost function of controllability/observability at passing of each node
- It is implemented to be theoretically optimal and avoids recursion wherever possible
- Memory complexity of the algorithm is 4(2N+P+G) where N is the number of fault sites, P is the number of inputs+outputs, and G is the number of gates+fanouts
- Two variations of 32-bit Kogge-Stone Adder will be used to test the combinational circuit performance of the program, as it involves several deep chains of AND, OR, XOR, and NOT gates but can also be implemented with NAND and NOR gates
- A 16-bit shift register with scan chains will be used to test the sequential circuit performance of the program. This will enable testing of the program's capacity to deal with uncertain flip flop states
- Work Distribution: Shirley
- The program takes input in the form of a specified netlist format and outputs all measures for each node in the netlist
- Fanouts are also treated as fault sites, with a 1-to-3 fanout consisting 4 fault sites 
- The program currently works for combinational controllability and observability
