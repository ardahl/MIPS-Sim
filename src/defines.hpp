#ifndef DEFINES_HPP_
#define DEFINES_HPP_

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>     //cout
#include <fstream>      //ifstream, ofstream
#include <locale>
#include <algorithm>

#define DATA_BEGIN 0x100
#define WORD_SIZE 4 //4 bytes
#define DOUBLE_WORD_SIZE 8
#define INT_REG 32
#define FLOAT_REG 32
#define MEM_SIZE 128 //32 4-byte words

typedef unsigned char byte_t;

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

typedef int issue_t;
#define ISS_FAILED -1

//Basically a catch-all for instructions
//Has timing for which cycle it enters and exits each stage
//And has variables for each argument and possible parsing of it
//Not every value is used for every instruction, only the relevant ones are set
typedef struct InsType {
    std::string fetchedRaw;         //Line as fetched from memory, including all spaces/tabs
    std::string line;               //trimmed version of fetchedRaw
    Ins in;                         //Enum for instruction type
    std::vector<std::string> args;  //Up to 3 instruction arguments
    int regDest;
    int regSource1;
    int regSource2;
    int im;         //immediate value
    int memLoc;     //memory location
    std::string label;
    issue_t index;
    int R1, R2, R3; //register values
    double F1, F2, F3;
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
    char iCacheHit;
} Instruction_t;

typedef struct IFISBuf {
    Instruction_t *insBuf;
} IFIS_t;

typedef struct ISRDBuf {
    Instruction_t *insBuf;
} ISRD_t;

typedef struct RDEXBuf {
    Instruction_t *insBuf;
} RDEX_t;

typedef struct EXWBBuf {
    Instruction_t *insBuf;
} EXWB_t;

std::string trim_left(const std::string& str);
std::string trim_right(const std::string& str);
std::string trim(const std::string& str);
Ins instToEnum(std::string ins);
std::string enumToInst(Ins ins);

template<typename T>
void print(std::ofstream &f, T t, int width) {
    f << std::left << std::setw(width) << std::setfill(' ') << t;
}

//Case insensitive string find
//https://stackoverflow.com/questions/3152241/case-insensitive-stdstring-find
// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct my_equal {
    my_equal( const std::locale& loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};

// find substring (case insensitive)
template<typename T>
int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale()) {
    typename T::const_iterator it = std::search(str1.begin(), str1.end(),
        str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));
    if(it != str1.end()) {
        return it - str1.begin();
    }
    else {
        return -1; // not found
    }
}

#endif
