# Assembler and Simulator

#### Description

This is a pair of related C++ files, where Assembler is a program used to convert assembly code into machine code, 
and Simulator is a program that executes the machine code output by the Assembler and simulates the running process of the Manchester Baby.

#### Instructions

How to use them
1. First, you need to ensure that the assembly code to be compiled is correctly placed in a file named assemblyCode.txt and that the file is located in the same folder as the two source code files.
2. Use the bash command: g++ assembler.cpp -o assembler to compile assembler.cpp.
3. Use the bash command: g++ simulator.cpp -o simulator to compile simulator.cpp.
4. Then, use the bash command: ./assembler to convert the assembly code into machine code.
5. Use the bash command: ./simulator to run the machine code.
6. If you want to see the compilation and execution of other assembly code programs, you can open the otherTestCode folder, which contains some test code we have provided.

#### More details

About common errors in assembly language source code:
These two programs can automatically detect most errors and provide indications of the error types.
Although we have not provided test cases for error codes, you can still modify correct code to test the robustness of the program.

About the instruction set of the Manchester Baby:
Since the SUB operation requires two operands, in our program, we have expanded the operand “001” into “LDP” (loadPositive).
This allows you to directly load data from memory into the calculator.

About output of the program:
Assembler:
	It will automatically print the assembly code read without comments and empty lines, as well as the symbol table read from the assembly language.

Simulator:
	It will print key values related to the Manchester Baby.
	Step: The count of the “fetch-decode-execute-printout” cycle. At the initial state, step begins with 0 and be increased by one from the start of the cycle.
	CI: The column of the memory that the program is executing.
	PI: The exact content machine code of the CI.
	IAF: Signifies “Immediate Addressing Flag”. Processor will regard “address (PI from 0 to 12)” as operand value if it equals to 1 and will not track to target address.
	Target address: Program track to here to do some operations such as load, store and so on.
	Accumulator: A register to temporarily store the calculation result of the program. When the program ends, its value will be output as the result.
