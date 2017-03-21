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
    NOINST
};

//Basically a catch-all for instructions
//Has timing for which cycle it enters and exits each stage
//And has variables for each argument and possible parsing of it
//Not every value is used for every instruction, only the relevant ones are set
typedef struct InsType {
    Ins in;
    std::vector<std::string> args;
    int regDest;
    int regSource1;
    int regSource2;
    int im;         //immediate value
    int memLoc;     //memory location
    //Timing
    int IFin;
    int IFout;
    int IDin;
    int IDout;
    int EXin;
    int EXout;
    int MEMin;
    int MEMout;
    int WBin;
    int WBout;
    char raw;
    char waw;
    char struc;
} Instruction_t;

typedef struct IFIDBuf {
    Instruction_t insBuf;
    bool wait;
} IFID_t;

typedef struct IDEXBuf {
    Instruction_t insBuf;
} IDEX_t;

typedef struct EXMEMBuf {
    Instruction_t insBuf;
} EXMEM_t;

typedef struct MEMWBBuf {
    Instruction_t insBuf;
} MEMWB_t;

std::string trim_left(const std::string& str);
std::string trim_right(const std::string& str);
std::string trim(const std::string& str);

#endif
