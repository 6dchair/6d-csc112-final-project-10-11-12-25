#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "machine_code.h"

// map register name "r0".."r31" to number
static int RegisterNumber(const char *r) {
    if(r[0] != 'r') 
        return -1;
    int n = atoi(r + 1);
    if(n < 0 || n > 31) 
        return -1;
    return n;
}

// Encode R-type instruction: opcode rs rt rd shamt funct
static uint32_t Encode_R_Type(uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t funct) {
    return (0 << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | funct;
}

// Encode I-type instruction: opcode rs rt immediate
static uint32_t Encode_I_Type(uint8_t opcode, uint8_t rs, uint8_t rt, int16_t imm) {
    return (opcode << 26) | (rs << 21) | (rt << 16) | ((uint16_t)imm & 0xFFFF);
}

// print 32-bit binary
// static void print_bin(uint32_t code, FILE *out) {
//     for(int i = 31; i >= 0; i--) {
//         fprintf(out, "%c", (code & (1u << i)) ? '1' : '0');
//     }
// }

// Main function: converts assembly from the .asm file into macine code 
int MachineFromAssembly(const char *asm_file, const char *out_file) {
    FILE *in = fopen(asm_file, "r");
    if(!in) 
        return 0;
    FILE *out = fopen(out_file, "w");
    if(!out) {
        fclose(in); 
        return 0; 
    }

    char line[256]; // buffer for each input line
   // char last_buffer[256] = "";  // remember the previous buffer label/comment

    // read each line of the assembly file
    while(fgets(line, sizeof(line), in)) {
        // remove newline
        line[strcspn(line, "\r\n")] = '\0';

        // skip leading spaces
        char *p = line;
        while(*p && isspace(*p)) 
            p++;

        // skip cpmments and blank lines
        if(*p == '#' || *p == '\0')
            continue;

        // variables for parsed registers and immediate
        char r1[8], r2[8], r3[8];
        int imm;
        uint32_t code = 0; // encoded 32-bit instruction
        int matched = 0; // flag: 1 if instruction pattern recognized

        // Match instrcution patterns one by one by using sscanf
        // daddiu
        if(sscanf(line, "daddiu %7[^,], %7[^,], #%i", r1, r2, &imm) == 3) {
            int rt = RegisterNumber(r1), rs = RegisterNumber(r2);
            if(rt >= 0 && rs >= 0) {
                code = Encode_I_Type(0x19, rs, rt, imm); // opcode daddiu
                matched = 1;
            }
        } // daddu 
        else if(sscanf(line, "daddu %7[^,], %7[^,], %7s", r1, r2, r3) == 3) {
            int rd = RegisterNumber(r1), rs = RegisterNumber(r2), rt = RegisterNumber(r3);
            if(rd >=0 && rs >=0 && rt >=0) {
                code = Encode_R_Type(rs, rt, rd, 0, 0x21); // funct daddu
                matched = 1;
            }
        } // dsubu
        else if(sscanf(line, "dsubu %7[^,], %7[^,], %7s", r1, r2, r3) == 3) {
            int rd = RegisterNumber(r1), rs = RegisterNumber(r2), rt = RegisterNumber(r3);
            if(rd >=0 && rs >=0 && rt >=0) {
                code = Encode_R_Type(rs, rt, rd, 0, 0x23); // funct dsubu
                matched = 1;
            }
        } // dmul 
        else if(sscanf(line, "dmul %7[^,], %7[^,], %7s", r1, r2, r3) == 3) {
            int rd = RegisterNumber(r1), rs = RegisterNumber(r2), rt = RegisterNumber(r3);
            if(rd >=0 && rs >=0 && rt >=0) {
                code = Encode_R_Type(rs, rt, rd, 0, 0x18); // funct dmul
                matched = 1;
            }
        } // ddiv
        else if(sscanf(line, "ddiv %7[^,], %7[^,], %7s", r1, r2, r3) == 3) {
            int rd = RegisterNumber(r1), rs = RegisterNumber(r2), rt = RegisterNumber(r3);
            if(rd >=0 && rs >=0 && rt >=0) {
                code = Encode_R_Type(rs, rt, rd, 0, 0x1A); // funct ddiv
                matched = 1;
            }
        }

        // if(matched) {
        //     fprintf(out, "0x%08X ", code);
        //     print_bin(code, out);
        //     fprintf(out, "\n");
        // }
        
        // If an instruction was matched, extract and print encoded fields
        if(matched) {
            // extract each field from the 32-bit code
            uint8_t opcode = (code >> 26) & 0x3F;
            uint8_t rs     = (code >> 21) & 0x1F;
            uint8_t rt     = (code >> 16) & 0x1F;
            uint8_t rd     = (code >> 11) & 0x1F;
            uint8_t shamt  = (code >> 6)  & 0x1F;
            uint8_t funct  =  code        & 0x3F;
            uint16_t imm   =  code        & 0xFFFF;

            // print the formatted machine code in both binary field breakdown and hex
            if(opcode == 0) { 
                // R-type format
                fprintf(out, "%06b %05b %05b %05b %05b %06b : 0x%08X",
                    opcode, rs, rt, rd, shamt, funct, code);
            } // I-type format
            else {
                fprintf(out, "%06b %05b %05b %016b : 0x%08X",
                    opcode, rs, rt, imm, code);
            }

            // Peek the next line to decide whether to add a single or double newline
            // (double newline means new group or statement)
            long pos = ftell(in); // save current position
            char peek[256];
            if(fgets(peek, sizeof(peek), in)) {
                char *q = peek;
                while (*q && isspace(*q))
                    q++;

                // If next line is blank or a label/comment -> double newline (new buffer)
                if(*q == '\0' || *q == '#') {
                    fprintf(out, "\n\n");  // two newlines = end of a buffer
                } else {
                    fprintf(out, "\n");    // one newline = same buffer (same statement)
                }

                fseek(in, pos, SEEK_SET);  // restore file position or go back to where we were
            }
        }

    }

    fclose(in);
    fclose(out);
    return 1;
}
