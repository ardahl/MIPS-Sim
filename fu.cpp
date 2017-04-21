#include "fu.hpp"

/**
Data
**/
void DataUnit::execute(Instruction_t* instruction) {
    if(validOp(instruction->in)) {
        inst = instruction;
        count = cycles;
        //Calculate offset for memory
        instruction->memLoc = instruction->R2 + instruction->memLoc;
    }
}

bool DataUnit::validOp(Ins in) {
    switch(in) {
        case LD:
        case SD:
        case LW:
        case SW:
            return true;
        default:
            return false;
    }
}

/**
Integer
**/
void IntegerUnit::execute(Instruction_t* instruction) {
    if(validOp(instruction->in)) {
        inst = instruction;
        count = cycles;
        //If load immediate, do nothing
        //If LUI, multiply immediate value by 2^16
        if(instruction->in == LUI) {
            instruction->im = instruction->im << 16;
        }
        else if(instruction->in != LI) {//Else, perform selected operation
            switch(instruction->in) {
                case DADD:
                    instruction->R1 = instruction->R2 + instruction->R3;
                    break;
                case DADDI:
                    instruction->R1 = instruction->R2 + instruction->im;
                    break;
                case DSUB:
                    instruction->R1 = instruction->R2 - instruction->im;
                    break;
                case DSUBI:
                    instruction->R1 = instruction->R2 - instruction->im;
                    break;
                case AND:
                    instruction->R1 = instruction->R2 & instruction->im;
                    break;
                case ANDI:
                    instruction->R1 = instruction->R2 & instruction->im;
                    break;
                case OR:
                    instruction->R1 = instruction->R2 | instruction->im;
                    break;
                case ORI:
                    instruction->R1 = instruction->R2 | instruction->im;
                    break;
                default:
                    break;
            }
        }
    }
}

bool IntegerUnit::validOp(Ins in) {
    switch(in) {
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
            return true;
        default:
            return false;
    }
}

/**
FP Add
**/
void FPAdder::execute(Instruction_t* instruction) {
    if(validOp(instruction->in)) {
        inst = instruction;
        count = cycles;
        //Perform add/sub
        if(instruction->in == ADDD) {
            instruction->F1 = instruction->F2 + instruction->F3;
        }
        else {
            instruction->F1 = instruction->F2 - instruction->F3;
        }
    }
}

bool FPAdder::validOp(Ins in) {
    switch(in) {
        case ADDD:
        case SUBD:
            return true;
        default:
            return false;
    }
}

/**
FP Multiply
**/
void FPMult::execute(Instruction_t* instruction) {
    if(validOp(instruction->in)) {
        inst = instruction;
        count = cycles;
        //Do Multiply
        instruction->F1 = instruction->F2 * instruction->F3;
    }
}

bool FPMult::validOp(Ins in) {
    switch(in) {
        case MULD:
            return true;
        default:
            return false;
    }
}

/**
FP Divide
**/
void FPDiv::execute(Instruction_t* instruction) {
    if(validOp(instruction->in)) {
        inst = instruction;
        count = cycles;
        //Do Division
        instruction->F1 = instruction->F2 / instruction->F3;
    }
}

bool FPDiv::validOp(Ins in) {
    switch(in) {
        case DIVD:
            return true;
        default:
            return false;
    }
}
