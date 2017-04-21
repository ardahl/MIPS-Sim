#ifndef FU_HPP_
#define FU_HPP_
#include "memory.hpp"
//Functional Units

class FunctionalUnit {
protected:
    Instruction_t* inst;
    Memory* mem;
    int cycles;

public:
    FunctionalUnit(Memory *m, int wait) { mem = m; cycles = count = wait; }
    virtual bool running() { return --count; }             //Decrements counter. Returns false once counter is 0
    virtual void execute(Instruction_t* instruction)=0;     //Performs the operation
    virtual bool validOp(Ins in)=0;
    int count;
};

class DataUnit: public FunctionalUnit {
public:
    DataUnit(Memory *m, int cycles): FunctionalUnit(m, cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class IntegerUnit: public FunctionalUnit {
public:
    IntegerUnit(Memory *m, int cycles): FunctionalUnit(m, cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class FPAdder: public FunctionalUnit {
public:
    FPAdder(Memory *m, int cycles): FunctionalUnit(m, cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class FPMult: public FunctionalUnit {
public:
    FPMult(Memory *m, int cycles): FunctionalUnit(m, cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

class FPDiv: public FunctionalUnit {
public:
    FPDiv(Memory *m, int cycles): FunctionalUnit(m, cycles) {}
    void execute(Instruction_t* instruction);
    bool validOp(Ins in);
};

#endif
