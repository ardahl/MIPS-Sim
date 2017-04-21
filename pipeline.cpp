#include "pipeline.hpp"
#include <cstdlib>      //exit
#include <cstdio>       //printf

Pipeline::Pipeline() {
    pc = 0;
    cycles = 0;
    running = false;
    rgx = std::regex("\\s*([a-zA-Z]+\\.?[a-zA-Z]?)\\s+([a-zA-Z][0-9]|[a-zA-Z]+),?\\s*(-?[a-zA-Z0-9\\(\\)]*),?\\s*([a-zA-Z]*[0-9]*)?");
}

Pipeline::Pipeline(std::string inst, std::string data, std::string config, std::string out) {
    mem = new Memory(inst, data);
    branches = mem.getBranches();
    pc = 0;
    cycles = 0;
    running = true;
    rgx = std::regex("\\s*([a-zA-Z]+\\.?[a-zA-Z]?)\\s+([a-zA-Z][0-9]|[a-zA-Z]+),?\\s*(-?[a-zA-Z0-9\\(\\)]*),?\\s*([a-zA-Z]*[0-9]*)?");

    sb = Scoreboard(config);

    log.open(out);
    if(!log) {
        perror("Unable to open log file");
        std::exit(EXIT_FAILURE);
    }
}

Pipeline::~Pipeline() {
    // output.close();
}

void Pipeline::run() {
    ifStage = NULL;
    stall = false;
    isStage = NULL;
    parsed = false;
    rdStage = NULL;
    exStage = NULL;
    exIndex = -1;
    exCycles = false;
    wbStage = NULL;
    ifis1.insBuf = NULL;
    ifis2.insBuf = NULL;
    ifisC = 0;
    isrd1.insBuf = NULL;
    isrd2.insBuf = NULL;
    isrdC = 0;
    rdex1.insBuf = NULL;
    rdex2.insBuf = NULL;
    rdexC = 0;
    exwb1.insBuf = NULL;
    exwb2.insBuf = NULL;
    exwbC = 0;
    while(running) {
        cycles++;
        Fetch();
        Issue();
        Read();
        Exec();
        Write();
        printCycle(log);
        sb.printSB(log);
        if(!stall) {
            pc++;
        }
        if(cycles == 35) {
            running = false;
        }
    }
}

//If there's not an instruction waiting, read in next instruction
//Check if the buffer is empty, move instruction out if so
//    If not empty, instruction in ID stage,
void Pipeline::Fetch() {
    //If there's not an instruction sitting here already
    if(ifStage == NULL) {
        if(mem.availInstruction(pc)) { //make sure we don't read past the end
            std::string line = mem.getInstruction(pc);
            //strip branch label if exists
            std::string::size_type n;
            if((n=line.find(":")) != std::string::npos) {
                line = line.substr(n+1, std::string::npos);
            }
            line = trim(line);
            ifStage = new Instruction_t;
            initInstMemory(ifStage);
            ifStage->line = line;
            ifStage->IFin = cycles;
            std::smatch match;
            //Parse just the instruction for tracking purposes
            if(std::regex_search(line, match, rgx)) {
                ifStage->in = instToEnum(match[1].str());
            }
            fetched.push_back(ifStage);
            stall = false;
        }
    }
    else {  //If we're waiting with an instruction, mark it as a structural hazard
        stall = true;
        ifStage->struc = 'Y';
    }

    //Move to the buffer.
    if(ifis1.insBuf == NULL && ifStage != NULL) {
        ifStage->IFout = cycles;
        ifis1.insBuf = ifStage;
        ifStage = NULL;
        if(ifisC == 0) {
            ifisC = 1;
        }
    }
}

//Decode Instruction and check for structural hazards
//If there's an instruction waiting in the buffer, read it in and wait
//If an instruction has been read in and hasn't been parsed, do it
//Otherwise, just wait for the id/ex buffer to be free'd
//function issue(op, dst, src1, src2)
//   wait until (!Busy[FU] AND !Result[dst]); // FU can be any functional unit that can execute operation op
//   Busy[FU] ← Yes;
//   Op[FU] ← op;
//   Fi[FU] ← dst;
//   Fj[FU] ← src1;
//   Fk[FU] ← src2;
//   Qj[FU] ← Result[src1];
//   Qk[FU] ← Result[src2];
//   Rj[FU] ← Qj[FU] == 0;
//   Rk[FU] ← Qk[FU] == 0;
//   Result[dst] ← FU;
void Pipeline::Issue() {
    if(ifisC == 2 && isStage == NULL) {
        isStage = ifis2.insBuf;
        isStage->ISin = cycles;
        ifis2.insBuf = NULL;
        if(ifis1.insBuf == NULL) {
            ifisC = 0;
        }
        else {
            ifisC = 1;
        }
    }
    if(isStage != NULL) {
        //Parse instruction and if next buffer is free push it
        //TODO: Deal with branching
        //Loop through each argument
            //If it's a register, set appropriate reg value
            //If it's an immediate value, set it
            //If it's an offset
        if(!parsed) {
            std::string line = isStage->line;
            if(line == "HLT") { //Special Case for Halt
                isStage->in = HLT;
            }
            else {  //Everything Else
                std::smatch match;
                if(std::regex_search(line, match, rgx)) {
                    isStage->in = instToEnum(match[1].str());
                    for(int i = 2; i < (int)match.size(); i++) {    //Skip 0 (whole match) and 1 (instruction)
                        if(!match[i].str().empty()) {
                            isStage->args.push_back(match[i].str());
                        }
                    }
                }
                else {
                    printf("Line %s: No Match\n", line.c_str());
                }
            }
            //Parse Arguments to actual values
            parseInstruction(isStage);
            parsed = true;
        }
        //Set Scoreboard values
        issue_t t = sb.attemptIssue(isStage);
        isStage->index = t;
        if(t != ISS_FAILED) {
            if(isrd1.insBuf == NULL) {
                isStage->ISout = cycles;
                isrd1.insBuf = isStage;
                isStage = NULL;
                parsed = false;
                if(isrdC == 0) {
                    isrdC = 1;
                }
            }
        }
        else {
            isStage->struc = 'Y';
        }
    }
    if(ifisC == 1 && isStage == NULL) {
        ifisC = 2;
        ifis2 = ifis1;
        ifis1.insBuf = NULL;
    }
}

//Wait until no data hazards, then read operands
//function read_operands(FU)
//   wait until (Rj[FU] AND Rk[FU]);
//   Rj[FU] ← No;
//   Rk[FU] ← No;
void Pipeline::Read() {
    if(isrdC == 2 && rdStage == NULL) {
        rdStage = isrd2.insBuf;
        rdStage->RDin = cycles;
        isrd2.insBuf = NULL;
        if(isrd1.insBuf == NULL) {
            isrdC = 0;
        }
        else {
            isrdC = 1;
        }
    }
    if(rdStage != NULL) {
        if(sb.canRead(rdStage)) {
            //read in from register files
            switch(rdStage->in) {
                case DADD:
                case DSUB:
                case AND:
                case OR:
                case BNE:
                case BEQ:
                    rdStage->R3 = intRegisters[rdStage->regSource2];
                case LW:
                case SW:
                case LD:
                case SD:
                case DADDI:
                case DSUBI:
                case ANDI:
                case ORI:
                    rdStage->R2 = intRegisters[rdStage->regSource1];
                    break;
                case ADDD:
                case SUBD:
                case MULD:
                case DIVD:
                    rdStage->F2 = fpRegisters[rdStage->regSource1];
                    rdStage->F3 = fpRegisters[rdStage->regSource2];
                    break;
                default:
                    break;
            }
            rdStage->RDout = cycles;
            rdex1.insBuf = rdStage;
            rdStage = NULL;
            if(rdexC == 0) {
                rdexC = 1;
            }
        }
    }
    if(isrdC == 1 && rdStage == NULL) {
        isrdC = 2;
        isrd2 = isrd1;
        isrd1.insBuf = NULL;
    }
}

//Operate on operands
//function execute(FU)
   // Execute whatever FU must do
void Pipeline::Exec() {
    if(rdexC == 2 && exStage == NULL) {
        exStage = rdex2.insBuf;
        exStage->EXin = cycles;
        rdex2.insBuf = NULL;
        if(rdex1.insBuf == NULL) {
            rdexC = 0;
        }
        else {
            rdexC = 1;
        }
        exCycles = false;
    }
    //Loop Through each functional unit and perform the operation if available
    if(exStage != NULL) {
        if(!exCycles) {
            exIndex = sb.execute(exStage);
            exCycles = true;
        }
        if(!sb.running(exIndex)) {
            exStage->EXout = cycles;
            exwb1.insBuf = exStage;
            exStage = NULL;
            if(exwbC == 0) {
                exwbC = 1;
            }
        }
    }
    if(rdexC == 1 && exStage == NULL) {
        rdexC = 2;
        rdex2 = rdex1;
        rdex1.insBuf = NULL;
    }
}

//Write results
//function write_back(FU)
//   wait until (∀f {(Fj[f]≠Fi[FU] OR Rj[f]=No) AND (Fk[f]≠Fi[FU] OR Rk[f]=No)})
//   foreach f do
//       if Qj[f]=FU then Rj[f] ← Yes;
//       if Qk[f]=FU then Rk[f] ← Yes;
//   Result[Fi[FU]] ← 0; // 0 means no FU generates the register's result
//   Busy[FU] ← No;
void Pipeline::Write() {
    //Write result to register or memory
    if(exwbC == 2 && wbStage == NULL) {
        wbStage = exwb2.insBuf;
        wbStage->WBin = cycles;
        exwb2.insBuf = NULL;
        if(exwb1.insBuf == NULL) {
            exwbC = 0;
        }
        else {
            exwbC = 1;
        }
    }
    //Loop Through each functional unit and perform the operation if available
    if(wbStage != NULL) {
        if(sb.canWrite(wbStage)) {
            //Write results to register

            if(wbStage->in == HLT) {
                running = false;
            }
            wbStage->WBout = cycles;
            wbStage = NULL;
        }
    }
    if(exwbC == 1 && wbStage == NULL) {
        exwbC = 2;
        exwb2 = exwb1;
        exwb1.insBuf = NULL;
    }
}

void Pipeline::parseInstruction(Instruction_t *inst) {
    Ins in = inst->in;
    std::vector<std::string> args = inst->args;
    int dest = 0, imm = 0, pos = 0, offset = 0, s1 = 0, s2 = 0;
    std::string label;
    switch(in) {
        //Dest and Imm
        case LI:
        case LUI:
            dest = std::stoi(args[0].substr(1));
            imm = std::stoi(args[1]);
            // printf("Ins: %s, Dest: %d, Imm: %d\n", enumToInst(in).c_str(), dest, imm);
            inst->regDest = dest;
            inst->im = imm;
            break;
        //Dest and Reg Offset
        case LW:
        case SW:
        case LD:
        case SD:
            dest = std::stoi(args[0].substr(1));
            pos = args[1].find_first_of("(");
            offset = std::stoi(args[1].substr(0, pos));
            s1 = std::stoi(args[1].substr(pos+2, args[1].size()-pos-1));
            // printf("Ins: %s, Dest: %d, Offset: %d, Reg: %d\n", enumToInst(in).c_str(), dest, offset, s);
            inst->regDest = dest;
            inst->memLoc = offset;
            inst->regSource1 = s1;
            break;
        //Dest and 2 Source
        case DADD:
        case DSUB:
        case AND:
        case OR:
        case ADDD:
        case SUBD:
        case MULD:
        case DIVD:
            dest = std::stoi(args[0].substr(1));
            s1 = std::stoi(args[1].substr(1));
            s2 = std::stoi(args[2].substr(1));
            // printf("Ins: %s, Dest: %d, S1: %d, S2: %d\n", enumToInst(in).c_str(), dest, s1, s2);
            inst->regDest = dest;
            inst->regSource1 = s1;
            inst->regSource2 = s2;
            break;
        //Dest, Source, Imm
        case DADDI:
        case DSUBI:
        case ANDI:
        case ORI:
            dest = std::stoi(args[0].substr(1));
            s1 = std::stoi(args[1].substr(1));
            imm = std::stoi(args[2]);
            // printf("Ins: %s, Dest: %d, S1: %d, Imm: %d\n", enumToInst(in).c_str(), dest, s1, imm);
            inst->regDest = dest;
            inst->regSource1 = s1;
            inst->im = imm;
            break;
        //label
        case J:
            label = args[0];
            // printf("Ins: %s, Label: %s\n", enumToInst(in).c_str(), label.c_str());
            inst->label = label;
            break;
        //2 Reg and label
        case BNE:
        case BEQ:
            s1 = std::stoi(args[0].substr(1));
            s2 = std::stoi(args[1].substr(1));
            label = args[2];
            // printf("Ins: %s, S1: %d, S2: %d, Label: %s\n", enumToInst(in).c_str(), s1, s2, label.c_str());
            inst->regSource1 = s1;
            inst->regSource2 = s2;
            inst->label = label;
            break;
        default:
            // printf("Ins: %s\n", enumToInst(in).c_str());
            break;
    }
}

void Pipeline::printCycle(std::ofstream &f) {
    //Loop through every fetched instruction and print out the info
    f << "Cycle " << cycles << "\n";
    print(f, "Ins", 8);
    print(f, "Fetch", 8);
    print(f, "Issue", 8);
    print(f, "Read", 8);
    print(f, "Exec", 8);
    print(f, "Write", 8);
    print(f, "RAW", 7);
    print(f, "WAW", 7);
    print(f, "Struct", 7);
    f << std::endl;
    for(int i = 0; i < (int)fetched.size(); i++) {
        Instruction_t *struc = fetched[i];
        print(f, enumToInst(struc->in), 8);
        std::string fetch, issue, read, exe, write;
        if(struc->IFin != -1) {
            fetch = std::to_string(struc->IFin) + "," + std::to_string(struc->IFout);
        }
        if(struc->ISin != -1) {
            issue = std::to_string(struc->ISin) + "," + std::to_string(struc->ISout);
        }
        if(struc->RDin != -1) {
            read = std::to_string(struc->RDin) + "," + std::to_string(struc->RDout);
        }
        if(struc->EXin != -1) {
            exe = std::to_string(struc->EXin) + "," + std::to_string(struc->EXout);
        }
        if(struc->WBin != -1) {
            write = std::to_string(struc->WBin) + "," + std::to_string(struc->WBout);
        }
        print(f, fetch, 8);
        print(f, issue, 8);
        print(f, read, 8);
        print(f, exe, 8);
        print(f, write, 8);
        print(f, struc->raw, 7);
        print(f, struc->waw, 7);
        print(f, struc->struc, 7);
        f << std::endl;
    }
    f << std::endl;
    //Print buffers
    f << "IFIS: ";
    std::string ifis, isrd, rdex, exwb;
    if(ifis1.insBuf != NULL) {
        ifis += enumToInst(ifis1.insBuf->in);
    }
    ifis += ",";
    if(ifis2.insBuf != NULL) {
        ifis += enumToInst(ifis2.insBuf->in);
    }
    print(f, ifis, 8);
    f << std::endl << "ISRD: ";
    if(isrd1.insBuf != NULL) {
        isrd += enumToInst(isrd1.insBuf->in);
    }
    isrd += ",";
    if(isrd2.insBuf != NULL) {
        isrd += enumToInst(isrd2.insBuf->in);
    }
    print(f, isrd, 8);
    f << std::endl << "RDEX: ";
    if(rdex1.insBuf != NULL) {
        rdex += enumToInst(rdex1.insBuf->in);
    }
    rdex += ",";
    if(rdex2.insBuf != NULL) {
        rdex += enumToInst(rdex2.insBuf->in);
    }
    print(f, rdex, 8);
    f << std::endl << "EXWB: ";
    if(exwb1.insBuf != NULL) {
        exwb += enumToInst(exwb1.insBuf->in);
    }
    exwb += ",";
    if(exwb2.insBuf != NULL) {
        exwb += enumToInst(exwb2.insBuf->in);
    }
    print(f, exwb, 8);
    f << std::endl << std::endl;
}

void Pipeline::initInstMemory(Instruction_t *ins) {
    ins->IFin = ins->IFout = ins->ISin = ins->ISout = ins->RDin = ins->RDout = ins->EXin = ins->EXout = ins->WBin = ins->WBout = -1;
    ins->in = UNKNOWN;
    ins->args.clear();
    ins->regDest = ins->regSource1 = ins->regSource2 = -1;
    ins->im = ins->memLoc = -1;
    ins->raw = ins->waw = ins->struc = 'N';
}
