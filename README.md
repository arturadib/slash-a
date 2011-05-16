# Slash/A 

A programming language and C++ library for (quantitative) linear genetic programming.  

Copyright (C) 2004-2011 Artur B Adib.  

Licensed under GNU version 3.  
http://www.gnu.org/licenses

## Introduction

Slash/A is programming language and C++ library for quantitative applications of linear genetic programming (GP). Genetic programming is a machine learning method for randomly 'evolving' computer programs until they perform a given desired task [1,2]. Linear means that codes are expressed as a simple string of instructions [1] as opposed to the more complex tree structure originally adopted by GP practitioners [2].

When expressed in plain text (as opposed to Bytecode), a Slash/A program consists of a series atomic instructions separated by slashes, like this program that takes two user inputs and adds them:

    input/0/save/input/add/output/.

Or, more intelligibly,

    input/   # gets an input from user and saves it to register F
    0/       # sets register I = 0
    save/    # saves content of F into data vector D[I] (i.e. D[0] := F)
    input/   # gets another input, saves to F
    add/     # adds to F current data pointed to by I (i.e. D[0] := F)
    output/. # outputs result from F

The instructions are atomic in that they don't need any arguments (unlike some [x86 assembly](http://en.wikipedia.org/wiki/Assembly_language) instructions, for example), so **any random sequence of Slash/A instructions is a semantically correct program**. This is important in genetic programming as it enables the free mutation of any instruction without worrying about its number and types of arguments. Instruction argumentation is effectively done through fixed registers (see below).

When expressed as Bytecodes, a Slash/A program is represented by a simple C++ vector of unsigned numbers, each of which corresponds to an instruction. A mutation operation is thus a simple replacement of a number in such a vector by another random integer, while a crossing-over operation can be accomplished by simply cutting-and-pasting the appropriate vector segments into another vector.

Because one can map any instruction from a Turing-complete language (C, Pascal, etc) into a Slash/A code, **Slash/A is Turing-complete**, though it needn't be so (as one can pick and choose which instructions to enable).

[1] [Genetic Programming: An Introduction](http://www.amazon.com/Genetic-Programming-Introduction-Artificial-Intelligence/dp/155860510X)  
[2] [Genetic Programming: On the Programming of Computers by Means of Natural Selection](http://www.amazon.com/Genetic-Programming-Computers-Selection-Adaptive/dp/0262111705)

## Features

- **Optimized for speed.** Slash/A's interpreter introduces a highly optimized Bytecode interpreter that rivals compiled code. With a Monte Carlo example, Slash/A's interpreter runs _only 2x_ slower than an analogous code written in pure optimized C++ code.
- **No function argumentation.** Because the code is expressed as a string of atomic instructions, any randomly generated code is semantically correct. Inspired by [Avida](http://avida.devosoft.org/);
- **Extensible instruction set.** Slash/A comes with a Default Instruction Set (DIS) covering most elementary instructions for flow control, mathematics, etc (see below), but users can introduce any number of custom instructions with simple C++ classes;

## Getting started

Slash/A only needs `g++` and its standard libraries. There are no other dependencies. 

Compile the Slash/A library `libslasha.a` first:

    $ cd lib
    $ make

The source `slash/main.cpp` contains a simple application of the Slash/A library (a command-line interpreter). Take a look at it as a first example on how to use the library (< 100 lines). To compile it:

    $ cd slash
    $ make

To test the interpreter, run one of the examples under `slash/examples`:

    $ cd slash
    $ ./slash examples/add.sla
    slash -- An interpreter for the Slash/A language
    Slash/A Revision 10, Copyright (C) Artur B. Adib

    Enter input #1: 3.14
    Enter input #2: 5 
    Output #1: 8.14

    Total number of operations: 6
    Total number of invalid operations: 0
    Total number of inputs before an output: 2

## Memory resources

The Slash/A interpreter exposes two registers: one integer, `I`, and one floating-point, `F`. All other data is stored in a floating-point vector `D[i]`.

An intuitive depiction of the memory resources available to Slash/A programs is sketched below:

             _________________
      D[i]: |  |  |  |  |  |  | ...  << Data vector D[i]
            |__|__|__|__|__|__|
        i :  1  2  3  4  5  6
                   ^ 
                   |
                   \--[I=3]          << Integer register. Load/save operations would access D[3].

                      [F=3.1415]     << Floating-point register


## Current limitations

- It is not possible to enter floating-point constants directly into the code. It is expected that if a floating-point constant is important to solve the problem at hand, the evolving codes will find their own way to construct them (see examples below).

## Default instruction set (DIS)

_NOTE: For performance reasons, all DIS instructions are implemented inline in `lib/SlashA_DIS.hpp`. (The compiler will inline the code whenever possible, avoiding a round-trip to a new function)._

**Numeric instructions**

* `0-(?)`: sets I := 0,1,2,...,(?), where the maximum number (?) is determined at runtime by the user;

**Register-Register commands**

* `itof`: copies I into F (F := I);
*	`ftoi`: copies the absolute value of F into I, rounding off to the nearest integer ( I := round(abs(F)) );
*	`inc`: increments register F (F := F + 1);
* `dec`: decrements register F (F := F - 1);

**Memory-Register commands**

*	`load`: loads data at position I into F (F := D[I]);
*	`save`: reverse of the above (D[I] := F);
*	`swap`: swaps the contents of F and D[I];
*	`cmp`: returns F := 0 if F == D[I], and F := -1 otherwise.

**Flow control**

* `label`: next code position (x+1) gets the label I;
*	`gotoifp`: if F >= 0, goes to the code position with the previously defined label I;
*	`jumpifn`: if F<0, jumps to the instruction following the corresponding jumphere instruction;
*	`jumphere`: see above;
*	`loop`: will loop I times the following block (block ends at the next endloop);
*	`endloop`: see above.

**I/O**

*	`input`: sets F to the next float number in the input buffer;
*	`output`: writes the content of F to the output buffer; 

**Mathematics**

*	`add`: adds number at data vector position I to F (F := F + D[I]);
*	`sub`: F := F - D[I];
*	`mul`: F := F * D[I];
*	`div`: F := F / D[I] - division by zero is protected;
*	`abs`: F := |F|;
*	`sign`: F := -F;
*	`exp`: F := e ^ F;
*	`log`: natural logarithm, F := log(F);
*	`sin`: F := sin(F);
*	`pow`: F := F ^ D[I];
*	`ran`: returns a random number in F between 0 and 1 (F := ran(0,1));

**Other**

*	`nop`: no operation;

**Source-code interpreter**

_(These 'instructions' are only implemented in the `source2ByteCode()` function; there are no Bytecode instructions corresponding to them)_

*	`/`: interprets and executes the word before its appearance as an instruction - only way to separate instructions;
*	`#`: end-of-line - will cause the interpreter to ignore the rest of the current line (including slashes);
* `.`: end-of-code - will cause the interpreter to stop interpreting the rest of the code.

## Error handling

Every invalid operation is ignored during program execution (e.g. going to an undefined label, reading from an unsaved variable, etc).

## Examples

_Throughout the examples, capital letters such as **X**, **Y**, etc stand for input values._

**Arithmetics: X+Y**

    input/0/save/input/add/output/.

**Power: X^4**

* by direct exponentiation

        4/itof/0/save/input/pow/output/.

* by manual iteration	

        input/0/save/mul/mul/mul/output/.

* by automatic iteration

        input/0/save/1/save/#  D[0] = X, D[1] = X
        2/itof/save/#  D[2] contains the loop counter (=2)
        0/label/#  main loop
        	1/load/0/mul/1/save/#  F := F * X, D[1] := X^n
        	2/load/dec/save/#  decreases loop counter
        0/gotoifp/#  loops three times
        1/load/output/.

**Polynomial X^3 + X^2 + X**

    input/0/save/mul/1/save/0/mul/2/save/# D[0] := X, D[1] := X^2, D[2] := X^3
    1/add/0/add/output/.

**Random number generation (prints 10 random numbers between 0 and 1)**

    9/itof/0/save/# D[0] is the counter
    0/label/#
    	ran/output/#
    	0/load/dec/save/#
    0/gotoifp/.

**Area of circle of radius X (illustrates how to construct floating-point constants)**

* method 1 (PI = 31415 * 10^-4)

        input/0/save/mul/save/# D[0] := X^2
        31415/itof/1/save/# D[1] := 31415
        4/itof/sign/2/save/# D[2] := -4
        10/itof/2/pow/# F := 10^(-4)
        1/mul/# F := 3.1415
        0/mul/output/.# output is 3.1415 * X^2

* method 2 (PI = 31415 / (100 * 100) )

        100/itof/0/save/mul/save/# D[0] := 10^4
        31415/itof/0/div/save/# D[0] := 3.1415
        input/1/save/mul/# F := X^2
        0/mul/output/.# output is 3.1415 * X^2

**Equality test using `jumpifn` (outputs 1 if two inputs are identical, 0 otherwise)**

    input/0/save/#
    input/0/sub/abs/sign/save/# D[0] < 0 only if inputs are different
    0/itof/1/save/# default answer is 1 (in D[1])
    0/load/
    jumpifn/#
    	1/itof/1/save/# changes default answer if inputs are identical
    jumphere/#
    1/load/output/.

**Integer/Fractional number classification (outputs 1 if X is integer, 0 otherwise)**

    input/0/save/ftoi/itof/# D[0] := X, F := round(X)
    0/div/save/# D[0] == 1 if X is an integer, D[0] != 1 otherwise
    1/itof/0/sub/abs/sign/0/save/# D[0] == 0 if X is an integer, D[0] > 0 otherwise
    0/itof/1/save/# D[1] := 0
    0/load/# F < 0 if X is fractional
    jumpifn/#
    	1/itof/save/# D[1] := 1
    jumphere/#
    1/load/output/.

**Integer/Fractional classification with nested jumpifn instructions (outputs 1 if both X and Y are integers, 0 otherwise)**

    input/0/save/input/1/save/# D[0] := X, D[1] := Y
    0/load/ftoi/itof/# F := round(X)
    0/div/save/# D[0] == 1 if X is an integer, D[0] != 1 otherwise
    1/itof/0/sub/abs/sign/0/save/# D[0] == 0 if X is an integer, D[0] < 0 otherwise
    0/itof/2/save/# default answer is 0 (either X or Y is not an integer)
    0/load/#
    jumpifn/# will jump if X is not an integer
    	1/load/ftoi/itof/# F := round(Y)
    	1/div/save/# D[1] == 1 if Y is an integer, D[1] != 1 otherwise
    	1/itof/1/sub/abs/sign/1/save/# D[1] == 0 if X is an integer, D[1] < 0 otherwise
    	jumpifn/#
    		1/itof/2/save/# X and Y are integers!
    	jumphere/#
    jumphere/#
    2/load/output/.

**Positive/Negative classification loop using an If-Then-Else construction (outputs 1 if X is positive, 0 otherwise)**

    0/label/#
    	input/0/save/#
    	jumpifn/# If positive...
    		1/itof/output/# ...print 1
    	jumphere/
    	0/load/sign/#
    	jumpifn/# Else...
    		0/itof/output/# ...print 0
    	jumphere/#
    0/itof/gotoifp/. infinite loop (use Ctrl+C to abort)

**Factorial function**

    input/0/save/dec/1/save/0/mul/save/# D[0] := X*(X-1), D[1] := X-1
    dec/dec/dec/
    jumpifn/# we're not interested in 2!, 1!, or 0! ...
    	0/label/#
    		1/load/dec/save/0/mul/save/# D[1] decreases from X to 1, D[0] := X*(X-1)*...*D[1]
    		1/load/dec/dec/# makes sure we exit the loop if D[1] == 1
    	0/gotoifp/#
    	0/load/output/#
    jumphere/.

**Monte Carlo (hit-and-miss) evaluation of the area of a circle of unit radius (=3.1415...)**

    7/itof/0/save/10/itof/0/pow/save/# D[0] is the number of MC points (here 10^7)
    0/itof/1/save/# D[1] contains the total number of points so far
    0/itof/2/save/# D[2] contains the number of hits inside a quadrant
    0/label/#
            ran/3/save/mul/save/# D[3] := x^2
            ran/4/save/mul/# F := y^2
            3/add/save/# D[3] := x^2 + y^2
            1/itof/3/sub/# F < 0 if missed
            jumpifn/
                    2/load/inc/save/# D[2] := D[2] + 1 only if it's a hit
            jumphere/
            1/load/inc/save/# D[1] := D[1] + 1 always
            0/load/dec/save/# decrease counter
    0/gotoifp/#
    2/load/1/div/save/# D[1] := hits / total
    4/itof/1/mul/# F := 4 * (area of quadrant)
    output/.
