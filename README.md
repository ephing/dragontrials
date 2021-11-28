# dragontrials
A host of optional challenging assignments for eecs 665  

## Trial 1: dragonlex
Create a program that takes in a token specification and spits out a program that can tokenize the language using that specification (effectively, write flex from scratch)  

## Trial 2: dragonsdt
Create a program that takes a grammar specification and spits out a program that can determine if a list of tokens (output from Trial 1) is included in the language of that grammar using an LL(1) parser table.  
In addition, program must execute syntax directed translations while evaluating the list of tokens (old spec gave no indication of how the input would be formatted, new spec provided the C++ requirement)  

## Trial 3: dragonparse
Create a program that takes a grammar specification and spits out a program that can determine if a list of tokens (output from Trial 1) is included in the language of that grammar using an SLR parser table. Syntax-directed translations are not required.  

## Trial 4
Extend Trial 3 to output the parser automaton used to build the table using dot.  

## Trial 5
Write an interpreter for the compiled language from the in class projects (CShanty) with the same semantics as the compiled version.  

## Trial 6
Add a flag to the CShanty compiler to allow it to compile to MIPS in addition to x64.  
