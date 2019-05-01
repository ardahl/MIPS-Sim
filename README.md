# README #

This repo is for a class project to implement a simulation of the MIPS architecture using the CDC 6600 scoreboard. It uses a simplified MIPS instruction set detailed below. This simulator has both a data cache and an instruction cache. The data cache is 2-way set associative with a total of 4 words blocks and uses a least recently used replacement strategy. Both caches share the same data bus meaning they can stall each other.

### Instruction Set ###
Data Transfer:

* LW $d, i($s) - Load word (4 bytes), $d = MEM[$s + i]:4

* SW $d, i($s) - Save word (4 bytes), MEM[$s + i]:4 = $d

* L.D $d, i(%s) - Load floating point (8 bytes), $d = MEM[$s + i]:8

* S.D $d, i(%s) - Save floating point (8 bytes), MEM[$s + i]:4 = $d


Control:

* J label - Unconditional jump to branch label

* BEQ $s, $t, label - Branch on equal, if($s == $t) jump to label

* BNE $s, $t, label - Branch on not equal, if($s != $t) jump to label


Arithmetic/Logic:

* DADD $d, $s, $t - Add, $d = $s + $t

* DADDI $d, $s, i - Add Immediate, $d = $s + i

* DSUB $d, $s, $t - Subtract, $d = $s - $t

* DSUBI $d, $s, i - Subtract Immediate, $d = $s - i

* AND $d, $s, $t - Bitwise And, $d = $s & $t

* ANDI $d, $s, i - Bitwise And Immediate, $d = $s & i

* OR $d, $s, $t - Bitwise Or, $d = $s | $t

* ORI $d, $s, i - Bitwise Or Immediate, $d = $s | i

* LI $d, i - Load Immediate, $d = i

* LUI $d, i - Load Upper Immediate (load i into upper 16 bits), $d = i << 16

* ADD.D $d, $s, $t - Floating Point Add, $d = $s + $t

* MUL.D $d, $s, $t - Floating Point Multiply, $d = $s * $t

* DIV.D $d, $s, $t - Floating Point Division, $d = $s / $t

* SUB.D $d, $s, $t - Floating Point Subtraction, $d = $s - $t


Special:

* HLT - Halts program execution

### Input Formats ###
To run: ./simulator inst.txt data.txt config.txt result.txt

inst.txt - Text file of the program using the instructions above

data.txt - Text file containing the data that can be read by loads. The data is a string of binary of a variable amount of 32-bit words, one per line.

config.txt - Configuration file for the functional units and instruction cache. Only 1 integer FU is available.

    FP Adder: <number of units>, <cycle count>

    FP Multiplier: <number of units>, <cycle count>

    FP Divider: <number of units>, <cycle count>

    I-Cache: <number of blocks>, <block size in words>

result.txt - Name of the file to print the output trace to. Trace includes the list of instructions run and at what cycle it exits each
pipeline stage as well as if the instruction caused a data hazard.
