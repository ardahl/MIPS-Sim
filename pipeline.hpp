#ifndef PIPELINE_HPP_
#define PIPELINE_HPP_

#include "defines.hpp"
#include <unordered_map>
#include <fstream>      //ifstream, ofstream
#include <regex>

class Pipeline {
public:
    Pipeline();
    Pipeline(std::string input, std::string data, std::string config, std::string out);
    ~Pipeline();

    void run();
private:
    void readInstructions(std::string inst);
    void IF();  //Instruction Fetch stage
    void ID();  //Indtruction Decode stage
    void EX();  //Execution stage
    void MEM(); //Memory stage
    void WB();  //Write Back stage
    Ins instToEnum(std::string ins);
    std::string enumToInst(Ins ins);
    void resetStageMem(Instruction_t &ins);

    bool running;
    int pc;     //Program counter
    int cycles; //Current number of cycles
    std::vector<std::string> instructions;
    std::unordered_map<std::string, int> branches; //Map of branch label to program counter number
    int intRegisters[INT_REG];      //Array of 32 bit Integer registers
    double fpRegisters[FLOAT_REG];  //Array of 64 bit floating point registers

    std::regex rgx;

    //Buffers between each stage
    IFID_t ifid;
    IDEX_t idex;
    EXMEM_t exmem;
    MEMWB_t memwb;
    //Because of stalling we need to keep buffers of each stage so we don't
    //loose the values for the next cycle
    //IF
    Instruction_t ifStage;
    //ID
    Instruction_t idStage;
    //EX
    Instruction_t exStage;
    //MEM
    Instruction_t memStage;
    //WB
    Instruction_t wbStage;

    // std::ofstream output;
};

#endif
