#include "pipeline.hpp"
#include <iostream>     //cout
#include <cstdlib>      //exit
#include <cstdio>       //printf

Pipeline::Pipeline() {
    std::string inst = "inst.txt";
    readInstructions(inst);
    pc = 0;
    cycles = 0;
    running = true;
    rgx = std::regex("\\s*([a-zA-Z]+\\.?[a-zA-Z]?)\\s+([a-zA-Z][0-9]|[a-zA-Z]+),?\\s*(-?[a-zA-Z0-9\\(\\)]*),?\\s*([a-zA-Z]*[0-9]*)?");
}

Pipeline::Pipeline(std::string inst, std::string data, std::string config, std::string out) {
    readInstructions(inst);
    pc = 0;
    cycles = 0;
    running = true;
    rgx = std::regex("\\s*([a-zA-Z]+\\.?[a-zA-Z]?)\\s+([a-zA-Z][0-9]|[a-zA-Z]+),?\\s*(-?[a-zA-Z0-9\\(\\)]*),?\\s*([a-zA-Z]*[0-9]*)?");
}

Pipeline::~Pipeline() {
    // output.close();
}

void Pipeline::readInstructions(std::string file) {
    std::ifstream in(file);
    if(!in) {
        perror("Unable to open instruction file");
        std::exit(EXIT_FAILURE);
    }

    std::string line;
    std::string::size_type n;
    int count = 0;
    while(std::getline(in, line)) {
        instructions.push_back(line);
        if((n=line.find(":")) != std::string::npos) {
            std::string br = line.substr(0, n);
            branches[br] = count;
        }
        count++;
    }
    if(in.bad()) {
        perror("Error reading instruction file line");
        std::exit(EXIT_FAILURE);
    }
    in.close();

    //Testing
    std::cout << "Program:\n";
    for(int i = 0; i < (int)instructions.size(); i++) {
        std::cout << instructions[i] << "\n";
    }
    std::cout << "\nBranches:\n";
    for(const auto &pair : branches) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }
}

void Pipeline::run() {
    resetStageMem(ifStage);
    resetStageMem(idStage);
    resetStageMem(exStage);
    resetStageMem(memStage);
    resetStageMem(wbStage);
    ifid.wait = false;
    ifid.insBuf.in = NOINST;
    parse = false;
    while(running) {
        cycles++;
        Fetch();
        Issue();
        Read();
        Exec();
        Write();
        pc++;
        if(pc >= (int)instructions.size()) {
            running = false;
        }
    }
}

//If there's not an instruction waiting, read in next instruction
//Check if the buffer is empty, move instruction out if so
//    If not empty, instruction in ID stage,
void Pipeline::Fetch() {
    //If there's not an instruction sitting here already
    if(ifStage.in == NOINST) {
        std::string line = instructions[pc];
        //strip branch label if exists
        std::string::size_type n;
        if((n=line.find(":")) != std::string::npos) {
            line = line.substr(n+1, std::string::npos);
        }
        line = trim(line);
        if(line == "HLT") { //Special Case for Halt
            ifStage.in = HLT;
        }
        else {  //Everything Else
            std::smatch match;
            if(std::regex_search(line, match, rgx)) {
                ifStage.in = instToEnum(match[1].str());
                // printf("Match 1: %s, %d\n", match[1].str().c_str(), (int)ifStage.in);
                for(int i = 2; i < (int)match.size(); i++) {    //Skip 0 (whole match)
                    if(!match[i].str().empty()) {
                        ifStage.args.push_back(match[i].str());
                    }
                    // printf("Arg %d: %s\n", i-1, match[i].str().c_str());
                }
            }
            else {
                printf("Line %s: No Match\n", line.c_str());
            }
        }
        ifStage.IFin = cycles;
    }
    else {  //If we're waiting, mark it as a structural hazard
        ifStage.struc = 'Y';
    }

    //Move to the buffer.
    if(ifid.insBuf.in == NOINST) {
        ifStage.IFout = cycles;
        ifid.insBuf = ifStage;
        resetStageMem(ifStage);
        ifid.wait = true;
    }
}

//Decode Instruction and check for structural hazards
//If there's an instruction waiting in the buffer, read it in and wait
//If an instruction has been read in and hasn't been parsed, do it
//Otherwise, just wait for the id/ex buffer to be free'd
void Pipeline::Issue() {
    if(ifid.wait) {
        ifid.wait = false;
        idStage = ifid.insBuf;
        idStage.IDin = cycles+1;
        //Clear Buffer
        ifid.insBuf.in = NOINST;
    }

    if(idStage.in != NOINST) {
        if(parse) {
            //Parse instruction and if next buffer is free push it
            //TODO: Deal with branching
            //Loop through each argument
                //If it's a register, set appropriate reg value
                //If it's an immediate value, set it
                //If it's an offset

            parse = false;
        }
        if(ifex.insBuf.in == NOINST) {
            idStage.IDout = cycles;
            idex.insBuf = idStage;
            resetStageMem(idStage);
            idex.wait = true;
        }
    }
}

//Wait until no data hazards, then read operands
void Pipeline::Read() {
    //TODO: Print info to test stages
    printf("Inst %s: ", enumToInst(idStage.in).c_str());
    for(int i = 0; i < (int)idStage.args.size(); i++) {
        printf("%s, ", idStage.args[i].c_str());
    }
    printf("\nID in: %d\nID out: %d\n\n", idStage.IFin, idStage.IFout);
}

//Operate on operands
void Pipeline::Exec() {

}

//Write results
void Pipeline::Write() {

}

void Pipeline::resetStageMem(Instruction_t &ins) {
    ins.IFin = ins.IFout = ins.IDin = ins.IDout = ins.EXin = ins.EXout = ins.MEMin = ins.MEMout = ins.WBin = ins.WBout = -1;
    ins.in = NOINST;
    ins.args.clear();
    ins.regDest = ins.regSource1 = ins.regSource2 = -1;
    ins.im = ins.memLoc = -1;
    ins.raw = ins.waw = ins.struc = 'N';
}

Ins Pipeline::instToEnum(std::string ins) {
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
    else {
        return NOINST;
    }
}

std::string Pipeline::enumToInst(Ins ins) {
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
    else {
        return "HLT";
    }
}
