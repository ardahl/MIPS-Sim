#include "defines.hpp"

//Even though this is called defines.cpp, it's really helpers.cpp. It includes
//various general purpose functions

// Helper functions for trimming whitespace from a string
//Found here: https://gist.github.com/dedeexe/9080526
//Left trim
std::string trim_left(const std::string& str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(str.find_first_not_of(pattern));
}

//Right trim
std::string trim_right(const std::string& str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(0,str.find_last_not_of(pattern) + 1);
}

//Left and Right trim
std::string trim(const std::string& str) {
  return trim_left(trim_right(str));
}

Ins instToEnum(std::string ins) {
    //Can't do cases for strings
    if(ins == "LI") {
        return LI;
    }
    else if(ins == "LUI") {
        return LUI;
    }
    else if(ins == "LW") {
        return LW;
    }
    else if(ins == "SW") {
        return SW;
    }
    else if(ins == "L.D") {
        return LD;
    }
    else if(ins == "S.D") {
        return SD;
    }
    else if(ins == "DADD") {
        return DADD;
    }
    else if(ins == "DADDI") {
        return DADDI;
    }
    else if(ins == "DSUB") {
        return DSUB;
    }
    else if(ins == "DSUBI") {
        return DSUBI;
    }
    else if(ins == "AND") {
        return AND;
    }
    else if(ins == "ANDI") {
        return ANDI;
    }
    else if(ins == "OR") {
        return OR;
    }
    else if(ins == "ORI") {
        return ORI;
    }
    else if(ins == "ADD.D") {
        return ADDD;
    }
    else if(ins == "SUB.D") {
        return SUBD;
    }
    else if(ins == "MUL.D") {
        return MULD;
    }
    else if(ins == "DIV.D") {
        return DIVD;
    }
    else if(ins == "J") {
        return J;
    }
    else if(ins == "BNE") {
        return BNE;
    }
    else if(ins == "BEQ") {
        return BEQ;
    }
    else if(ins == "HLT") {
        return HLT;
    }
    else {
        return NOINST;
    }
}

std::string enumToInst(Ins ins) {
    //Can't do cases for strings
    if(ins == LI) {
        return "LI";
    }
    else if(ins == LUI) {
        return "LUI";
    }
    else if(ins == LW) {
        return "LW";
    }
    else if(ins == SW) {
        return "SW";
    }
    else if(ins == LD) {
        return "L.D";
    }
    else if(ins == SD) {
        return "S.D";
    }
    else if(ins == DADD) {
        return "DADD";
    }
    else if(ins == DADDI) {
        return "DADDI";
    }
    else if(ins == DSUB) {
        return "DSUB";
    }
    else if(ins == DSUBI) {
        return "DSUBI";
    }
    else if(ins == AND) {
        return "AND";
    }
    else if(ins == ANDI) {
        return "ANDI";
    }
    else if(ins == OR) {
        return "OR";
    }
    else if(ins == ORI) {
        return "ORI";
    }
    else if(ins == ADDD) {
        return "ADD.D";
    }
    else if(ins == SUBD) {
        return "SUB.D";
    }
    else if(ins == MULD) {
        return "MUL.D";
    }
    else if(ins == DIVD) {
        return "DIV.D";
    }
    else if(ins == J) {
        return "J";
    }
    else if(ins == BNE) {
        return "BNE";
    }
    else if(ins == BEQ) {
        return "BEQ";
    }
    else if(ins == UNKNOWN) {
        return "UNKNOWN";
    }
    else if(ins == HLT) {
        return "HLT";
    }
    else {
        return "";
    }
}
