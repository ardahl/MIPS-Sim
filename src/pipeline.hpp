#ifndef PIPELINE_HPP_
#define PIPELINE_HPP_

#include "scoreboard.hpp"
#include <regex>

class Pipeline {
public:
    Pipeline();
    Pipeline(std::string input, std::string data, std::string config, std::string out);
    ~Pipeline();
    void run();

private:
    void Fetch();  //Instruction Fetch stage
    void Issue();  //Indtruction Decode stage
    void Read();  //Execution stage
    void Exec(); //Memory stage
    void Write();  //Write Back stage
    void initInstMemory(Instruction_t *ins);
    void parseInstruction(Instruction_t *inst);
    void printCycle(std::ofstream &f);
    bool running();

    // bool running;
    bool fetching;
    int pc;     //Program counter
    int cycles; //Current number of cycles
    Memory* mem;
    Scoreboard* sb;
    std::unordered_map<std::string, int> branches; //Map of branch label to program counter number
    int intRegisters[INT_REG];      //Array of 32 bit Integer registers
    double fpRegisters[FLOAT_REG];  //Array of 64 bit floating point registers

    std::regex rgx;
    std::ofstream log;
    std::vector<Instruction_t*> fetched; //List of all fetched instructions for printing purposes

    //Buffers between each stage
    //There's 2 buffers for each to simulate concurrency
    //The first stage always writes to the first one, and the second stage
    //always reads from the second one and swaps the buffers if available
    IFIS_t ifis1;
    IFIS_t ifis2;
    int ifisC;
    ISRD_t isrd1;
    ISRD_t isrd2;
    int isrdC;
    std::vector<RDEX_t> rdex1;
    std::vector<RDEX_t> rdex2;
    int rdexC;
    std::vector<EXWB_t> exwb1;
    std::vector<EXWB_t> exwb2;
    int exwbC;
    //Because of stalling we need to keep buffers of each stage so we don't
    //loose the values for the next cycle
    //IF
    Instruction_t *ifStage;
    bool cmiss;
    int tmpPC;
    //Issue
    Instruction_t *isStage;
    bool parsed;
    //Read
    std::vector<Instruction_t*> rdStage;
    bool branching;
    bool waitForCache;
    //Execute
    std::vector<Instruction_t*> exStage;
    std::vector<bool> exCycles;
    std::vector<int> exIndex;
    //WB
    std::vector<Instruction_t*> wbStage;

    // std::ofstream output;
};

#endif
