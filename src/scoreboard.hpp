#ifndef SCOREBOARD_HPP_
#define SCOREBOARD_HPP_

#include "fu.hpp"

class Scoreboard {
public:
    Scoreboard(std::string configUnits, Memory *m);
    ~Scoreboard();
    //For the Scoreboard
    issue_t attemptIssue(Instruction_t *instruct);
    bool canRead(Instruction_t *instruct);
    bool canWrite(Instruction_t *instruct);
    void printSB(std::ofstream &f);
    //For the functional units
    int execute(Instruction_t* inst);
    bool running(int index);

private:
    int numData, numInt, numAdd, numMult, numDiv;       //number of integer units is always 1
    int usedData, usedInt, usedAdd, usedMult, usedDiv;
    int total;
    //Functional units
    FunctionalUnit** FU;
//Instruction Status
    //This is implicitly taken care of by the pipeline
//Functional Unit Status
    //Busy: indicates whether the unit is being used
    bool *busy;
    //Op: Operation being performed (add, mul, div, mod, etc.)
    Instruction_t **op;
    //Fi: Destination register
    int *Fi;            //register index
    //Fj,Fk: source registers
    int *Fj, *Fk;
    //Qj,Qk: Functional units that will produce the source registers Fj,Fk
    std::string *Qj, *Qk;
    //Rj,Rk: Flags that indicate whether Fj,Fk are read and not yet read
    bool *Rj;
    bool *Rk;
//Register Status
    //Indicates, for each register, which functional unit will write results into it
    std::string regInt[INT_REG];
    std::string regFloat[FLOAT_REG];

    std::string instructionFU(Ins i);
    bool WAW(std::string type, int dest);
    int getIndex(std::string FU, int ind);
    void getRegTypes(int index, char &d, char &s1, char &s2);
    std::string indexToUnit(int index);
};

#endif
