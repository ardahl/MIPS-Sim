#include "fu.hpp"

FunctionalUnit::~FunctionalUnit() {
    mem = NULL;
    inst = NULL;
}

/**
Data
**/
void DataUnit::execute(Instruction_t* instruction) {
    if(validOp(instruction->in)) {
        inst = instruction;
        // if(inst->in == LD || inst->in == SD) {
        //     count = fpcycles;
        // }
        // else {
        //     count = cycles;
        // }
        count = cycles;
        //Read value from memory?
        inst->memLoc = inst->R2 + inst->memLoc;
        if(inst->in == LW) {
            int val = -1;
            int delay;
            if((delay=mem->read(inst->memLoc, val)) >= 0) {
                inst->R1 = val;
                if(delay > 0) {
                    count = delay;
                }
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("LW failed: %d\n", inst->memLoc);
            }
        }
        else if(inst->in == LD) {
            double val = -1;
            word = 1;
            int delay;
            if((delay=mem->readDouble(inst->memLoc, val, word)) >= 0) {
                inst->R1 = val;
                // if(delay > 0) {
                //     count = delay;
                // }
                count += delay;
                word++;
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("LD failed: %d\n", inst->memLoc);
                printf("%d\n", inst->R2);
            }
        }
        else if(inst->in == SW) {
            int delay;
            if((delay=mem->write(inst->memLoc, inst->R1)) >= 0) {
                if(delay > 0) {
                    count = delay;
                }
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("SW failed: %d\n", inst->memLoc);
            }
        }
        else if(inst->in == SD) {
            int delay;
            word = 1;
            if((delay=mem->write(inst->memLoc, inst->F1, word)) >= 0) {
                // if(delay > 0) {
                //     count = delay;
                // }
                count += delay;
                word++;
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("SD failed: %d\n", inst->memLoc);
            }
        }
    }
}

bool DataUnit::running() {
    //If the bus was busy, we need to keep retrying until it's not, then just count back
    if(count == BUSY) {
        // if(inst->in == LD || inst->in == SD) {
        //     count = fpcycles;
        // }
        // else {
        //     count = cycles;
        // }
        count = cycles;
        if(inst->in == LW) {
            int val = -1;
            int delay;
            if((delay=mem->read(inst->memLoc, val)) >= 0) {
                inst->R1 = val;
                if(delay > 0) {
                    count = delay;
                }
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("LW failed: %d\n", inst->memLoc);
            }
        }
        else if(inst->in == LD) {
            double val = -1;
            int delay;
            if((delay=mem->readDouble(inst->memLoc, val, word)) >= 0) {
                if(word == 1) {
                    inst->R1 = val;
                }
                // if(delay > 0) {
                //     count = delay;
                // }
                count += delay;
                word++;
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("LD failed: %d\n", inst->memLoc);
                printf("%d\n", inst->R2);
            }
        }
        else if(inst->in == SW) {
            int delay;
            if((delay=mem->write(inst->memLoc, inst->R1)) >= 0) {
                if(delay > 0) {
                    count = delay;
                }
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("SW failed: %d\n", inst->memLoc);
            }
        }
        else if(inst->in == SD) {
            int delay;
            if((delay=mem->write(inst->memLoc, inst->F1, word)) >= 0) {
                // if(delay > 0) {
                //     count = delay;
                // }
                count += delay;
                word++;
            }
            else if(delay == -2) {
                count = BUSY;
            }
            else {
                printf("SD failed: %d\n", inst->memLoc);
            }
        }
    }
    else {
        count--;
    }
    //If count is 0 (memory finished) and instruction was double load/store, try second word
    if(count == 0) {
        if(word == 3) {
            word = 0;
            mem->finishData();
        }
        else if(word == 2) {
            count = cycles;
            if(inst->in == LD) {
                double val = -1;
                int delay;
                if((delay=mem->readDouble(inst->memLoc, val, word)) >= 0) {
                    // if(delay > 0) {
                    //     count = delay;
                    // }
                    count += delay;
                    word++;
                }
                else if(delay == -2) {
                    count = BUSY;
                }
                else {
                    printf("LD failed: %d\n", inst->memLoc);
                    printf("%d\n", inst->R2);
                }
            }
            else if(inst->in == SD) {
                int delay;
                if((delay=mem->write(inst->memLoc, inst->F1, word)) >= 0) {
                    // if(delay > 0) {
                    //     count = delay;
                    // }
                    count += delay;
                    word++;
                }
                else if(delay == -2) {
                    count = BUSY;
                }
                else {
                    printf("SD failed: %d\n", inst->memLoc);
                }
            }
        }
        else {
            mem->finishData();
        }
    }
    return count;
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
        if(inst->in == LI) {
            inst->R1 = inst->im;
        }
        else if(inst->in == LUI) {
            inst->im = inst->im << 16;
            inst->R1 = inst->im;
        }
        else {//Else, perform selected operation
            switch(instruction->in) {
                case DADD:
                    inst->R1 = inst->R2 + inst->R3;
                    break;
                case DADDI:
                    inst->R1 = inst->R2 + inst->im;
                    break;
                case DSUB:
                    inst->R1 = inst->R2 - inst->R3;
                    break;
                case DSUBI:
                    inst->R1 = inst->R2 - inst->im;
                    break;
                case AND:
                    inst->R1 = inst->R2 & inst->R3;
                    break;
                case ANDI:
                    inst->R1 = inst->R2 & inst->im;
                    break;
                case OR:
                    inst->R1 = inst->R2 | inst->R3;
                    break;
                case ORI:
                    inst->R1 = inst->R2 | inst->im;
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
        if(inst->in == ADDD) {
            inst->F1 = inst->F2 + inst->F3;
        }
        else {
            inst->F1 = inst->F2 - inst->F3;
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
        inst->F1 = inst->F2 * inst->F3;
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
        inst->F1 = inst->F2 / inst->F3;
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
