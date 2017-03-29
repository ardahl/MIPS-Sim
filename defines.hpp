#ifndef DEFINES_HPP_
#define DEFINES_HPP_

#include <string>
#include <vector>

#define DATA_BEGIN 0x100
#define WORD_SIZE 32
#define INT_REG 32
#define FLOAT_REG 32

enum Ins {
    LI,
    LUI,
    LW,
    SW,
    LD,
    SD,
    DADD,
    DADDI,
    DSUB,
    DSUBI,
    AND,
    ANDI,
    OR,
    ORI,
    ADDD,
    SUBD,
    MULD,
    DIVD,
    J,
    BNE,
    BEQ,
    HLT,
    UNKNOWN,    //Used in fetch stage since we don't know what the instruction is yet
    NOINST
};

//Basically a catch-all for instructions
//Has timing for which cycle it enters and exits each stage
//And has variables for each argument and possible parsing of it
//Not every value is used for every instruction, only the relevant ones are set
typedef struct InsType {
    std::string line;
    Ins in;                         //Enum for instruction type
    std::vector<std::string> args;  //Up to 3 instruction arguments
    int regDest;
    int regSource1;
    int regSource2;
    int im;         //immediate value
    int memLoc;     //memory location
    //Timing
    int IFin;
    int IFout;
    int ISin;
    int ISout;
    int RDin;
    int RDout;
    int EXin;
    int EXout;
    int WBin;
    int WBout;
    char raw;
    char waw;
    char struc;
} Instruction_t;

typedef struct IFISBuf {
    Instruction_t insBuf;
} IFIS_t;

typedef struct ISRDBuf {
    Instruction_t insBuf;
} ISRD_t;

typedef struct RDEXBuf {
    Instruction_t insBuf;
} RDEX_t;

typedef struct EXWBBuf {
    Instruction_t insBuf;
} EXWB_t;

std::string trim_left(const std::string& str);
std::string trim_right(const std::string& str);
std::string trim(const std::string& str);

#endif
