#include "scoreboard.hpp"

Scoreboard::Scoreboard() {

}

Scoreboard::Scoreboard(std::string configUnits, Memory *m) {
    numData = 1;
    numInt = 1;
    usedData = 0;
    usedInt = 0;
    //Parse Number of units
    numAdd = numMult = numDiv = 1;
    usedAdd = usedMult = usedDiv = 0;
    std::ifstream in(configUnits);
    if(!in) {
        perror("Unable to open instruction file");
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    int n, addDelay, multDelay, divDelay;
    addDelay = multDelay = divDelay = 1;
    while(std::getline(in, line)) {
        if((n=ci_find_substr(line, std::string("FP Adder"))) != -1) {
            std::string num = line.substr(n+9, line.find_first_of(",")-n-9);
            // printf("%s: %s\n", line.c_str(), num.c_str());
            std::string num2 = line.substr(line.find_first_of(",")+1);
            numAdd = std::stoi(num);
            addDelay = std::stoi(num2);
        }
        if((n=ci_find_substr(line, std::string("FP Multiplier"))) != -1) {
            std::string num = line.substr(n+14, line.find_first_of(",")-n-14);
            // printf("%s: %s\n", line.c_str(), num.c_str());
            std::string num2 = line.substr(line.find_first_of(",")+1);
            numMult = std::stoi(num);
            multDelay = std::stoi(num2);
        }
        if((n=ci_find_substr(line, std::string("FP Divider"))) != -1) {
            std::string num = line.substr(n+11, line.find_first_of(",")-n-11);
            // printf("%s: %s\n", line.c_str(), num.c_str());
            std::string num2 = line.substr(line.find_first_of(",")+1);
            numDiv = std::stoi(num);
            divDelay = std::stoi(num2);
        }
    }

    //init arrays
    total = numData + numInt + numAdd + numMult + numDiv;
    busy = new bool[total];
    op = new Instruction_t*[total];
    Fi = new int[total];
    Fj = new int[total];
    Fk = new int[total];
    Qj = new std::string[total];
    Qk = new std::string[total];
    Rj = new bool[total];
    Rk = new bool[total];

    //Init units
    FU = new FunctionalUnit*[total];
    FU[0] = new DataUnit(m, 1);
    FU[1] = new IntegerUnit(m, 1);
    for(int i = 0; i < numAdd; i++) {
        int index = i+2;
        FU[index] = new FPAdder(m, addDelay);
    }
    for(int i = 0; i < numMult; i++) {
        int index = i+numAdd+2;
        FU[index] = new FPMult(m, multDelay);
    }
    for(int i = 0; i < numDiv; i++) {
        int index = i+numAdd+numMult+2;
        FU[index] = new FPDiv(m, divDelay);
    }

    //Initialize empty scoreboard
    for(int i = 0; i < total; i++) {
        busy[i] = false;
        op[i] = NULL;
        Fi[i] = -1;
        Fj[i] = -1;
        Fk[i] = -1;
        Qj[i] = "";
        Qk[i] = "";
        Rj[i] = false;
        Rk[i] = false;
    }

    for(int i = 0; i < INT_REG; i++) {
        regInt[i] = "";
    }
    for(int i = 0; i < FLOAT_REG; i++) {
        regFloat[i] = "";
    }
}

//Returns false if it can't be issues, true if the issue succeeds
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
issue_t Scoreboard::attemptIssue(Instruction_t *instruct) {
    Ins ins = instruct->in;
    std::string unit = instructionFU(ins);
    if(ins == HLT || ins == J || ins == BNE || ins == BEQ) {
        return true;
    }
    int dest = instruct->regDest;
    if(WAW(unit, dest)) {
        // printf("Failed: WAR %s\n", instruct->line.c_str());
        // printf("RegInt[%d]: %s\n", dest, regInt[dest].c_str());
        return ISS_FAILED;
        instruct->waw = 'Y';
    }

    if(unit == "DATAI") {
        if(busy[0]) {
            return ISS_FAILED;
        }
        else {
            busy[0] = true;
            op[0] = instruct;
            Fi[0] = dest;
            Fj[0] = instruct->regSource1;
            Fk[0] = -1;
            Qj[0] = regInt[instruct->regSource1];
            Qk[0] = "";
            Rj[0] = Qj[0].empty();
            Rk[0] = true;
            regInt[dest] = unit+std::to_string(0);
            return 0;
        }
    }
    if(unit == "DATAF") {
        if(busy[0]) {
            return ISS_FAILED;
        }
        else {
            busy[0] = true;
            op[0] = instruct;
            Fi[0] = dest;
            Fj[0] = instruct->regSource1;
            Fk[0] = -1;
            Qj[0] = regFloat[instruct->regSource1];
            Qk[0] = "";
            Rj[0] = Qj[0].empty();
            Rk[0] = true;
            regFloat[dest] = unit+std::to_string(0);
            return 0;
        }
    }
    else if(unit == "INT") {
        if(busy[1]) {
            return ISS_FAILED;
        }
        else {
            //Issue
            busy[1] = true;
            op[1] = instruct;
            Fi[1] = dest;
            if(ins == LI || ins == LUI) {
                Fj[1] = -1;
                Qj[1] = "";
            }
            else {
                Fj[1] = instruct->regSource1;
                Qj[1] = regInt[instruct->regSource1];
            }
            if(ins == LI || ins == LUI || ins == DADDI || ins == DSUBI || ins == ANDI || ins == ORI) {
                Fk[1] = -1;
                Qk[1] = "";
            }
            else {
                Fk[1] = instruct->regSource2;
                Qk[1] = regInt[instruct->regSource2];
            }
            Rj[1] = Qj[1].empty();
            Rk[1] = Qk[1].empty();
            regInt[dest] = unit+std::to_string(0);
            return 1;
        }
    }
    else if(unit == "ADD") {
        if(usedAdd == numAdd) {
            return ISS_FAILED;
        }
        else {
            int index = getIndex(unit, usedAdd);
            busy[index] = true;
            op[index] = instruct;
            Fi[index] = dest;
            Fj[index] = instruct->regSource1;
            Fk[index] = instruct->regSource2;
            Qj[index] = regInt[instruct->regSource1];
            Qk[index] = regInt[instruct->regSource2];
            Rj[index] = Qj[index].empty();
            Rk[index] = Qk[index].empty();
            regFloat[dest] = unit+std::to_string(usedAdd);
            usedAdd++;
            return index;
        }
    }
    else if(unit == "MUL") {
        if(usedMult == numMult) {
            return ISS_FAILED;
        }
        else {
            int index = getIndex(unit, usedMult);
            busy[index] = true;
            op[index] = instruct;
            Fi[index] = dest;
            Fj[index] = instruct->regSource1;
            Fk[index] = instruct->regSource2;
            Qj[index] = regFloat[instruct->regSource1];
            Qk[index] = regFloat[instruct->regSource2];
            Rj[index] = Qj[index].empty();
            Rk[index] = Qk[index].empty();
            regFloat[dest] = unit+std::to_string(usedMult);
            usedMult++;
            return index;
        }
    }
    else if(unit == "DIV") {
        if(usedDiv == numDiv) {
            return ISS_FAILED;
        }
        else {
            int index = getIndex(unit, usedDiv);
            busy[index] = true;
            op[index] = instruct;
            Fi[index] = dest;
            Fj[index] = instruct->regSource1;
            Fk[index] = instruct->regSource2;
            Qj[index] = regFloat[instruct->regSource1];
            Qk[index] = regFloat[instruct->regSource2];
            Rj[index] = Qj[index].empty();
            Rk[index] = Qk[index].empty();
            regFloat[dest] = unit+std::to_string(usedDiv);
            usedDiv++;
            return index;
        }
    }

    return ISS_FAILED;
}

bool Scoreboard::WAW(std::string type, int dest) {
    if(type == "INT" || type == "DATAI") {
        if(regInt[dest].empty()) {
            return false;
        }
    }
    else {
        if(regFloat[dest].empty()) {
            return false;
        }
    }
    return true;
}

//Read Functions
bool Scoreboard::canRead(Instruction_t *instruct) {
    issue_t index = instruct->index;
    if(instruct->in == BNE || instruct->in == BEQ || instruct->in == J || instruct->in == HLT) {
        return true;
    }
    if(Rj[index] && Rk[index]) {
        Rj[index] = false;
        Rk[index] = false;
        return true;
    }
    return false;
}

//Write Functions
bool Scoreboard::canWrite(Instruction_t *instruct) {
    issue_t index = instruct->index;
    if(instruct->in == BNE || instruct->in == BEQ || instruct->in == J || instruct->in == HLT) {
        return true;
    }
    //   wait until (∀f {(Fj[f]≠Fi[FU] OR Rj[f]=No) AND (Fk[f]≠Fi[FU] OR Rk[f]=No)})
    char d, s1, s2;
    getRegTypes(index, d, s1, s2);
    for(int f = 0; f < total; f++) {
        if(f == index || !busy[f]) {
            continue;
        }
        char fd, fs1, fs2;
        getRegTypes(f, fd, fs1, fs2);
        //Fuck me for making this "1" line
        if(!(
            (((d != 'n' || fs1 != 'n') || (Fj[f] != Fi[index] && fs1 == d)) || !Rj[f]) &&
            (((d != 'n' || fs2 != 'n') || (Fk[f] != Fi[index] && fs2 == d)) || !Rk[f])
            )) {

            return false;
        }
    }

    //foreach f do
    //       if Qj[f]=FU then Rj[f] ← Yes;
    //       if Qk[f]=FU then Rk[f] ← Yes;
    for(int f = 0; f < total; f++) {
        if(f == index) {
            continue;
        }
        if(Qj[f] == indexToUnit(index)) {
            Rj[f] = true;
        }
        if(Qk[f] == indexToUnit(index)) {
            Rk[f] = true;
        }
    }

    //   Result[Fi[FU]] ← 0; // 0 means no FU generates the register's result
    //   Busy[FU] ← No;
    if(d == 'i') {
        if(Fi[index] != -1) {
            regInt[Fi[index]] = "";
        }
    }
    else if(d == 'f') {
        if(Fi[index] != -1) {
            regFloat[Fi[index]] = "";
        }
    }
    busy[index] = false;

    std::string type = instructionFU(op[index]->in);
    if(type == "ADD") {
        usedAdd--;
    }
    else if(type == "MUL") {
        usedMult--;
    }
    else if(type == "DIV") {
        usedDiv--;
    }
    return true;
}

//Execution Functions
int Scoreboard::execute(Instruction_t* inst) {
    //figure out which unit it's being executed on
    if(inst->in == BNE || inst->in == BEQ || inst->in == J || inst->in == HLT) {
        return -1;
    }
    int index = 0;
    for(int i = 0; i < total; i++) {
        if(busy[i] && op[i] == inst) { //Both point to the same object (same instruction)
            index = i;
            break;
        }
    }
    if(index == total) {
        printf("Bad Index\n");
        return -1;
    }
    //Run instruction through unit
    FU[index]->execute(inst);
    //Return FU index
    return index;
}

bool Scoreboard::running(int index) {
    if(index == -1) {
        return false;
    }
    return FU[index]->running();
}

//Helpers
void Scoreboard::printSB(std::ofstream &f) {
    print(f, "Name", 8);
    print(f, "Busy", 8);
    print(f, "Op", 8);
    print(f, "Fi", 8);
    print(f, "Fj", 8);
    print(f, "Fk", 8);
    print(f, "Qj", 8);
    print(f, "Qk", 8);
    print(f, "Rj", 8);
    print(f, "Rk", 8);
    f << std::endl;
    for(int i = 0; i < total; i++) {
        //Name
        std::string name, fi, fj, fk, oper, qj, qk, rj, rk, timer;
        char d, s1, s2;
        if(busy[i]) {
            getRegTypes(i, d, s1, s2);

            if(i == 0) {
                name = "DATA";
            }
            else if(i == 1) {
                name = "INT";
            }
            else if(i > 1 && i < 2+numAdd) {
                name = "ADD" + std::to_string(i-2);
            }
            else if(i >= 2+numAdd && i < 2+numAdd+numMult) {
                name = "MULT" + std::to_string(i-2-numAdd);
            }
            else {
                name = "DIV" + std::to_string(i-2-numAdd-numMult);
            }
            oper = enumToInst(op[i]->in);
            if(Fi[i] != -1) {
                if(d == 'i') {
                    fi = "R";
                }
                else if(d == 'f') {
                    fi = "F";
                }
                fi = fi + std::to_string(Fi[i]);
            }
            if(Fj[i] != -1) {
                if(s1 == 'i') {
                    fj = "R";
                }
                else if(s1 == 'f') {
                    fj = "F";
                }
                fj = fj + std::to_string(Fj[i]);
            }
            if(Fk[i] != -1) {
                if(s2 == 'i') {
                    fk = "R";
                }
                else if(s2 == 'f') {
                    fk = "F";
                }
                fk = fk + std::to_string(Fk[i]);
            }
            qj = Qj[i];
            qk = Qk[i];
            rj = std::to_string(Rj[i]);
            rk = std::to_string(Rk[i]);
            timer = std::to_string(FU[i]->count);
        }
        print(f, name, 8);
        print(f, busy[i], 8);
        print(f, oper, 8);
        print(f, fi, 8);
        print(f, fj, 8);
        print(f, fk, 8);
        print(f, qj, 8);
        print(f, qk, 8);
        print(f, rj, 8);
        print(f, rk, 8);
        print(f, timer, 8);
        f << std::endl;
    }
    f << std::endl;
    for(int i = 0; i < INT_REG; i++) {
        print(f, "R"+std::to_string(i), 5);
    }
    f << std::endl;
    for(int i = 0; i < INT_REG; i++) {
        print(f, regInt[i], 5);
    }
    f << std::endl << std::endl;

    for(int i = 0; i < FLOAT_REG; i++) {
        print(f, "F"+std::to_string(i), 5);
    }
    f << std::endl;
    for(int i = 0; i < FLOAT_REG; i++) {
        print(f, regFloat[i], 5);
    }
    f << std::endl << std::endl;
}

void Scoreboard::getRegTypes(int index, char &d, char &s1, char &s2) {
    switch(op[index]->in) {
        case LW:
        case SW:
        case DADDI:
        case DSUBI:
        case ANDI:
        case ORI:
        case BNE:
        case BEQ:
        case HLT:
            d = 'i';
            s1 = 'i';
            s2 = 'n';
            break;
        case LD:
        case SD:
            d = 'f';
            s1 = 'i';
            s2 = 'n';
            break;
        case LI:
        case LUI:
            d = 'i';
            s1 = 'n';
            s2 = 'n';
            break;
        case DADD:
        case DSUB:
        case AND:
        case OR:
            d = 'i';
            s1 = 'i';
            s2 = 'i';
            break;
        case ADDD:
        case SUBD:
        case MULD:
        case DIVD:
            d = 'f';
            s1 = 'f';
            s2 = 'f';
            break;
        case J:
        default:
            d = 'n';
            s1 = 'n';
            s2 = 'n';
            break;
    }
}

std::string Scoreboard::indexToUnit(int index) {
    if(!busy[index]) {
        return "";
    }
    std::string instr = instructionFU(op[index]->in);
    if(index <= 1) {
        return instr + "0";
    }
    else if(index < 2+numAdd) {
        return instr + std::to_string(index-2);
    }
    else if(index < 2+numAdd+numMult) {
        return instr + std::to_string(index-2-numAdd);
    }
    else {
        return instr + std::to_string(index-2-numAdd-numMult);
    }
}

std::string Scoreboard::instructionFU(Ins i) {
    switch(i) {
        case LW:
        case SW:
            return "DATAI";
        case LD:
        case SD:
            return "DATAF";
        case LI:
        case LUI:
        case DADD:
        case DADDI:
        case DSUB:
        case DSUBI:
        case AND:
        case ANDI:
        case OR:
        case ORI:
        case J:
        case BNE:
        case BEQ:
        case HLT:
            return "INT";
        case ADDD:
        case SUBD:
            return "ADD";
        case MULD:
            return "MUL";
        case DIVD:
            return "DIV";
        default:
            return "NONE";
    }
}

//Returns the general index for the ind'th FU
int Scoreboard::getIndex(std::string fu, int ind) {
    //indexing goes Integer Unit, FP Adders, FP Multipliers, FP Dividers
    int index = -1;
    if(fu.find("DATA") != std::string::npos) {
        index = 0;
    }
    if(fu == "INT") {
        index = 1;
    }
    else if(fu == "ADD") {
        index = 2 + ind;
    }
    else if(fu == "MUL") {
        index = 2 + numAdd + ind;
    }
    else if(fu == "DIV") {
        index = 2 + numAdd + numMult + ind;
    }
    return index;
}
