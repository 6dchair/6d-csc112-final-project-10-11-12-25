# PART 1:
## Source -> MIPS64 Assemby -> Machine code (Hardcoded using C language)

This project reads a source text file written in a simplified high-level language (adhering to C syntax) and generates: (1) MIPS64 assembly code saved to a .txt file, and (2) Machine code in both binary and hex formats saved to an .mc file.
And displayed on the terminal are the lines and whether each line is valid or erroneous (displays error type or its cause).

### For more details about the C source and header files and what they do, please refer to the ReadMe.md file in the "kore-desu-1" folder

### General flow:
    1. Read the source file line by line
    2. Check for errors in the line
        2.1 Program ends when an error is encountered
    3. Parse the valid line into a standardized statement structure (statement type, LHS, RHS, raw (full))
    4. Close the source file
    5. Analyze variable usage for register allocation
    6. Generate assembly code from the parsed statement structures
    7. Generate the machine code using the generated assembly code text file
    8. End program execution

# PART 2: 
