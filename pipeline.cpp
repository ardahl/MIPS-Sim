#include "pipeline.hpp"
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
    std::cout << "\n";
}

void Pipeline::run() {
    ifStage = NULL;
    isStage = NULL;
    rdStage = NULL;
    exStage = NULL;
    wbStage = NULL;
    ifis1.insBuf = NULL;
    ifis2.insBuf = NULL;
    isrd1.insBuf = NULL;
    isrd2.insBuf = NULL;
    rdex1.insBuf = NULL;
    rdex2.insBuf = NULL;
    exwb1.insBuf = NULL;
    exwb2.insBuf = NULL;
    while(running) {
        cycles++;
        Fetch();
        Issue();
        Read();
        Exec();
        Write();
        printCycle();
        pc++;
    }
}

//If there's not an instruction waiting, read in next instruction
//Check if the buffer is empty, move instruction out if so
//    If not empty, instruction in ID stage,
void Pipeline::Fetch() {
    //If there's not an instruction sitting here already
    if(ifStage == NULL) {
        if(pc < (int)instructions.size()) { //make sure we don't read past the end
            std::string line = instructions[pc];
            //strip branch label if exists
            std::string::size_type n;
            if((n=line.find(":")) != std::string::npos) {
                line = line.substr(n+1, std::string::npos);
            }
            line = trim(line);
            ifStage = new Instruction_t;
            initInstMemory(&ifStage);
            ifStage->line = line;
            ifStage->IFin = cycles;
            ifStage->in = UNKNOWN;
            fetched.push_back(ifStage);
        }
    }
    else {  //If we're waiting with an instruction, mark it as a structural hazard
        ifStage->struc = 'Y';
    }

    //Move to the buffer.
    if(ifis1.insBuf == NULL && ifStage != NULL) {
        ifStage->IFout = cycles;
        ifis1.insBuf = ifStage;
        ifStage = NULL;
    }
}

//Decode Instruction and check for structural hazards
//If there's an instruction waiting in the buffer, read it in and wait
//If an instruction has been read in and hasn't been parsed, do it
//Otherwise, just wait for the id/ex buffer to be free'd
void Pipeline::Issue() {
    if(ifis2.insBuf != NULL && ifis2.insBuf->in == UNKNOWN && isStage == NULL) {
        isStage = ifis2.insBuf;
        isStage->ISin = cycles;
        //Clear Buffer
        ifis2.insBuf = NULL;
    }
    if(isStage != NULL && isStage->in != NOINST) {
        //Parse instruction and if next buffer is free push it
        //TODO: Deal with branching
        //Loop through each argument
            //If it's a register, set appropriate reg value
            //If it's an immediate value, set it
            //If it's an offset
        std::string line = isStage->line;
        if(line == "HLT") { //Special Case for Halt
            isStage->in = HLT;
        }
        else {  //Everything Else
            std::smatch match;
            if(std::regex_search(line, match, rgx)) {
                isStage->in = instToEnum(match[1].str());
                for(int i = 2; i < (int)match.size(); i++) {    //Skip 0 (whole match)
                    if(!match[i].str().empty()) {
                        isStage->args.push_back(match[i].str());
                    }
                }
            }
            else {
                printf("Line %s: No Match\n", line.c_str());
            }
        }

        if(isrd1.insBuf == NULL) {
            isStage->ISout = cycles;
            isrd1.insBuf = isStage;
            isStage = NULL;
            ifis2 = ifis1;
            ifis1.insBuf = NULL;
        }
    }
    if(ifis1.insBuf != NULL && ifis1.insBuf->in == UNKNOWN && ifis2.insBuf == NULL) {
        ifis2 = ifis1;
        ifis1.insBuf = NULL;
    }
}

//Wait until no data hazards, then read operands
void Pipeline::Read() {
    if(isrd2.insBuf != NULL && isrd2.insBuf->in != NOINST && rdStage == NULL) {
        rdStage = isrd2.insBuf;
        rdStage->RDin = cycles;
        //Clear Buffer
        isrd2.insBuf = NULL;
    }
    if(rdStage != NULL && rdStage->in != NOINST) {
        rdStage->RDout = cycles;
        if(rdStage->in == HLT) {
            running = false;
        }
        rdStage = NULL;
    }
    // if(isrd1.insBuf.in != NOINST && isrd2.insBuf.in == NOINST) {
    if(isrd1.insBuf != NULL && isrd1.insBuf->in != NOINST && isrd2.insBuf == NULL) {
        isrd2 = isrd1;
        isrd1.insBuf = NULL;
    }
}

//Operate on operands
void Pipeline::Exec() {

}

//Write results
void Pipeline::Write() {

}

void Pipeline::printCycle() {
    //Loop through every fetched instruction and print out the info
    printf("Cycle %d\n", cycles);
    printf("%-8s%-8s%-8s%-8s\n", "Ins", "Fetch", "Issue", "Read");
    for(int i = 0; i < (int)fetched.size(); i++) {
        Instruction_t *struc = fetched[i];
        print(enumToInst(struc->in), 8);
        std::string fetch, issue, read;
        if(struc->IFin != -1) {
            fetch = std::to_string(struc->IFin) + "," + std::to_string(struc->IFout);
        }
        if(struc->ISin != -1) {
            issue = std::to_string(struc->ISin) + "," + std::to_string(struc->ISout);
        }
        if(struc->RDin != -1) {
            read = std::to_string(struc->RDin) + "," + std::to_string(struc->RDout);
        }
        print(fetch, 8);
        print(issue, 8);
        print(read, 8);
        std::cout << "\n";
    }
    printf("\n");
}

void Pipeline::initInstMemory(Instruction_t **instruct) {
    Instruction_t *ins = *instruct;
    ins->IFin = ins->IFout = ins->ISin = ins->ISout = ins->RDin = ins->RDout = ins->EXin = ins->EXout = ins->WBin = ins->WBout = -1;
    ins->in = UNKNOWN;
    ins->args.clear();
    ins->regDest = ins->regSource1 = ins->regSource2 = -1;
    ins->im = ins->memLoc = -1;
    ins->raw = ins->waw = ins->struc = 'N';
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
    else if(ins == UNKNOWN) {
        return "UNKNOWN";
    }
    else {
        return "HLT";
    }
}
