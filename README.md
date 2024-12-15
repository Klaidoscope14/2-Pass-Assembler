Assembler Implementation

Overview

This project implements an assembler for a hypothetical assembly language. The assembler processes assembly code to generate machine code, providing error checking, intermediate representations, and listing files.

Features

Input

Accepts assembly source files (e.g., input.asm).

Functionality

First Pass:

Cleans and parses instructions into labels, mnemonics, and operands.

Validates labels, instructions, and operands.

Generates intermediate instructions.

Checks for duplicate labels and undefined references.

Second Pass:

Generates machine code based on validated instructions.

Produces listing and machine code files.

Outputs

Machine Code: Binary file with the assembled code.

Log File: Details errors and warnings encountered during assembly.

Listing File: Provides a human-readable version of the assembled code, including program counters and generated machine instructions.

Error Handling

Detects syntax errors, invalid labels, and undefined instructions.

Flags missing or unexpected operands.

Issues warnings for unused labels and missing HALT instruction.

File Structure

Source Files

asm.cpp: Main implementation of the assembler.

Key Classes and Functions

interm Struct: Represents an intermediate instruction.

checker Class:

isDecimal, isOctal, isHexadecimal: Validate operand formats.

Core Functions:

cleanInstr: Cleans input instructions by removing whitespace and comments.

seperateInstr: Splits instructions into label, mnemonic, and operand.

checkValidity: Validates parsed instructions.

writeMachineCode: Generates machine code file.

writeListingFile: Generates listing file.

writeErrors: Logs errors and warnings.

Opcode Table

The assembler supports the following mnemonics:

Mnemonic

Opcode

Operand Type

data

N/A

Value

ldc

00

Value

adc

01

Value

ldl

02

Offset

stl

03

Offset

ldnl

04

Offset

stnl

05

Offset

add

06

None

sub

07

None

shl

08

None

shr

09

None

adj

0A

Value

a2sp

0B

None

sp2a

0C

None

call

0D

Offset

return

0E

None

brz

0F

Offset

brlz

10

Offset

br

11

Offset

HALT

12

None

SET

N/A

Value

Usage

Compilation

Compile the assembler using a C++ compiler (e.g., g++):

$ g++ asm.cpp -o assembler

Running the Assembler

Run the assembled program with the input assembly file:

$ ./assembler input.asm

Output Files

input.log: Contains errors and warnings.

input.lst: Contains a detailed listing of the program.

input.o: Binary machine code file.

Future Enhancements

Support for macros and additional directives.

Improved error messages with suggestions.

Enhanced debugging features for generated machine code.

Integration with a simulator for executing generated code.

Contact

For questions or feedback, reach out to:

Email: support@assembler.com

Phone: +1-123-456-7890

