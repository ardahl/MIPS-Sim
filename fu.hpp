#ifndef FU_HPP_
#define FU_HPP_
#include "defines.hpp"
//Functional Units

class FunctionalUnit {
protected:
    Instruction_t* inst;
    int cycles;

public:
    FunctionalUnit(int wait) { cycles = count = wait; }
    virtual bool running() { return --count; }             //Decrements counter. Returns false once counter is 0
    virtual void execute(Instruction_t* instruction)=0;     //Performs the operation
    virtual bool validOp(Ins in)=0;
    int count;
};

class DataUnit: public FunctionalUnit {
public:
    DataUnit(int cycles): FunctionalUnit(cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class IntegerUnit: public FunctionalUnit {
public:
    IntegerUnit(int cycles): FunctionalUnit(cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class FPAdder: public FunctionalUnit {
public:
    FPAdder(int cycles): FunctionalUnit(cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class FPMult: public FunctionalUnit {
public:
    FPMult(int cycles): FunctionalUnit(cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class FPDiv: public FunctionalUnit {
public:
    FPDiv(int cycles): FunctionalUnit(cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

#endif
