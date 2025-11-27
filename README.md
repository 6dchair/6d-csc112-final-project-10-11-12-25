# PART 1: Simple compiler hardcoded using C language
#### (Source -> MIPS64 Assemby -> Machine code (binary : hex))

This project reads a source text file written in a simplified high-level language (adhering to C syntax) and generates: (1) MIPS64 assembly code saved to a .txt file, and (2) Machine code in both binary and hex formats saved to a .mc file.
Displayed on the terminal are the lines and whether each line is valid or erroneous (displays error type or its cause).

_For more details about the C source and header files and what they do, please refer to the ReadMe.md file in the "kore-desu-1" folder_

#### General flow:
    1. Read the source file line by line
    2. Check for errors in the line
        2.1 Program ends when an error is encountered
    3. Parse the valid line into a standardized statement structure (statement type, LHS, RHS, raw (full))
    4. Close the source file
    5. Generate assembly code from the parsed statement structures
    6. Generate the machine code using the generated assembly code text file
    7. End program execution
    
*Note:* This compiler currently supports only *variable declaration*, *assignment*, and *basic arithmetic operations*, following *C syntax*.

# PART 2: Custom language using Lex & Yacc 
_Work in progress_ 

*Tentative language name: **p.0** (read as "p-zero") - Prototype 0*
