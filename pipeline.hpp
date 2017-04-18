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
    void Fetch();  //Instruction Fetch stage
    void Issue();  //Indtruction Decode stage
    void Read();  //Execution stage
    void Exec(); //Memory stage
    void Write();  //Write Back stage
    Ins instToEnum(std::string ins);
    std::string enumToInst(Ins ins);
    void initInstMemory(Instruction_t **ins);
    void printCycle();

    bool running;
    int pc;     //Program counter
    int cycles; //Current number of cycles
    std::vector<std::string> instructions;
    std::unordered_map<std::string, int> branches; //Map of branch label to program counter number
    int intRegisters[INT_REG];      //Array of 32 bit Integer registers
    double fpRegisters[FLOAT_REG];  //Array of 64 bit floating point registers
    std::vector<Instruction_t*> fetched; //List of all fetched instructions for printing purposes

    std::regex rgx;

    //Buffers between each stage
    //There's 2 buffers for each to simulate concurrency
    //The first stage always writes to the first one, and the second stage
    //always reads from the second one and swaps the buffers if available
    IFIS_t ifis1;
    IFIS_t ifis2;
    ISRD_t isrd1;
    ISRD_t isrd2;
    RDEX_t rdex1;
    RDEX_t rdex2;
    EXWB_t exwb1;
    EXWB_t exwb2;
    //Because of stalling we need to keep buffers of each stage so we don't
    //loose the values for the next cycle
    //IF
    Instruction_t *ifStage;
    //Issue
    Instruction_t *isStage;
    //Read
    Instruction_t *rdStage;
    //Execute
    Instruction_t *exStage;
    //WB
    Instruction_t *wbStage;

    // std::ofstream output;
};

#endif
