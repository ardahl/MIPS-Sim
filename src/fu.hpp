#ifndef FU_HPP_
#define FU_HPP_
#include "memory.hpp"
//Functional Units

#define BUSY 100000 //Some large value that will never be the miss penelty amount

class FunctionalUnit {
protected:
    Instruction_t* inst;
    Memory* mem;
    int cycles;

public:
    FunctionalUnit(Memory *m, int wait) { mem = m; cycles = count = wait; }
    virtual ~FunctionalUnit();
    virtual bool running() { return --count; }             //Decrements counter. Returns false once counter is 0
    virtual void execute(Instruction_t* instruction)=0;     //Performs the operation
    virtual bool validOp(Ins in)=0;
    int count;
};

//TODO: Double words should do it one word at a time, since the bus may become busy
//after the first word but before the second word
class DataUnit: public FunctionalUnit {
private:
    int fpcycles;
    int word;
public:
    DataUnit(Memory *m, int cyclesInt, int cyclesFP): FunctionalUnit(m, cyclesInt), fpcycles(cyclesFP), word(0) {}
    bool running();
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
