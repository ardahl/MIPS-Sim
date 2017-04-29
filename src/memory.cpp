#include "memory.hpp"
#include <bitset>
#include <sstream>
#include <cstring>
#include <cctype>

Memory::Memory() {

}

Memory::Memory(std::string inst, std::string data, std::string config) {
    readInstructions(inst);
    readData(data);
    readCache(config);
    memDelay = -1;
    iUsed = false;
    dUsed = false;
    //I-Cache
    iValid = new bool[iBlockNum];
    iTag = new int[iBlockNum];
    for(int i = 0; i < iBlockNum; i++) {
        iValid[i] = false;
    }

    dValid = new bool*[SET_NUMBER];
    dTag = new int*[SET_NUMBER];
    oldestIndex = new int[SET_NUMBER];
    for(int i = 0; i < SET_NUMBER; i++) {
        dValid[i] = new bool[BLOCKS_PER_SET];
        dTag[i] = new int[BLOCKS_PER_SET];
        for(int j = 0; j < BLOCKS_PER_SET; j++) {
            dValid[i][j] = false;
        }
        oldestIndex[i] = 0;
    }
}

Memory::~Memory() {
    delete[] iValid;
    delete[] iTag;
    delete[] oldestIndex;
    for(int i = 0; i < SET_NUMBER; i++) {
        delete[] dValid[i];
        delete[] dTag[i];
    }
    delete[] dValid;
    delete[] dTag;
}

bool Memory::cacheBusy() {
    return memDelay > 0;
}

//Keep internal counter for caching
bool Memory::availInstruction(int pc) {
    if(pc >= (int)instructions.size()) {
        return false;
    }
    if(!iUsed) {   //New instruction to check for cache
        int btag = pc / iBlockSize;
        int bindex = btag % iBlockNum;
        //Check if pc is in cache
        if(iValid[bindex] && iTag[bindex] == btag) {
            //Cache hit
            return true;
        }
        else {  //Cache miss, set counter and replace block
            if(dUsed) {
                return false;
            }
            memDelay = iBlockSize*CYCLES_PER_WORD-1;
            iValid[bindex] = true;
            iTag[bindex] = btag;
            iUsed = true;
            return false;
        }
    }
    else if(iUsed && memDelay > 0) { //Instruction was a cache miss, so we're counting down
        memDelay--;
        return false;
    }
    else if(iUsed && memDelay == 0){
        memDelay = -1;
        iUsed = false;
        return true;
    }
    return false;
}

void Memory::readInstructions(std::string inst) {
    std::ifstream in(inst);
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

void Memory::readData(std::string data) {
    std::ifstream in(data);
    if(!in) {
        perror("Unable to open data file");
        std::exit(EXIT_FAILURE);
    }

    char c;
    int count = 0;
    int bitnum = 0;
    std::stringstream ss;
    while(in.get(c)) {
        if(std::isspace(c)) {
            continue;
        }
        ss << c;
        bitnum++;
        if(bitnum == 8) {
            std::bitset<8> b(ss.str());
            ss.str(std::string());
            unsigned long tmp = b.to_ulong();
            memory[count] = static_cast<byte_t>(tmp);
            unsigned long tmp2 = static_cast<unsigned long>(memory[count]);
            std::bitset<8> b2(tmp2);
            count++;
            bitnum = 0;
        }
    }
}

void Memory::readCache(std::string config) {
    std::ifstream in(config);
    if(!in) {
        perror("[MEM] Unable to open instruction file");
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    int n;
    while(std::getline(in, line)) {
        if((n=ci_find_substr(line, std::string("I-Cache"))) != -1) {
            std::string num = line.substr(n+8, line.find_first_of(",")-n-8);
            std::string num2 = line.substr(line.find_first_of(",")+1);
            iBlockNum = std::stoi(num);
            iBlockSize = std::stoi(num2);
        }
    }
}

//Write-back strategy
//If the write address is not in cache, add a delay simulating kicking out an entry and writing that back to memory
int Memory::write(int address, int data) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return -1;
    }
    int delay = 0;
    //Check if in cache
    //If so, just read value and return 0 delays
    //Otherwise, add to cache, read value and return delay cycles
    //transform index to words
    index = index / WORD_SIZE;
    int blockAdd = index/WORDS_PER_BLOCK;
    int setInd = blockAdd % SET_NUMBER;
    bool inCache = false;
    for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
        if(dValid[setInd][i] && dTag[setInd][i] == blockAdd) {
            inCache = true;
            break;
        }
    }
    if(!inCache) {
        if(iUsed) {
            return -2;
        }
        //find open spot in set
        int bind = -1;
        for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
            if(!dValid[setInd][i]) {
                bind = i;
                break;
            }
        }
        //If still -1, then replace oldest item in set
        if(bind >= 0) {
            dValid[setInd][bind] = true;
            dTag[setInd][bind] = blockAdd;
        }
        else {
            dValid[setInd][oldestIndex[setInd]] = true;
            dTag[setInd][oldestIndex[setInd]] = blockAdd;
            oldestIndex[setInd]++;
            if(oldestIndex[setInd] >= BLOCKS_PER_SET) {
                oldestIndex[setInd] = 0;
            }
        }
        delay += WORDS_PER_BLOCK*CYCLES_PER_WORD;
        dUsed = true;
        memDelay = delay;
    }

    byte_t *buf = new byte_t[WORD_SIZE];
    memcpy(buf, &data, WORD_SIZE);
    for(int i = 0; i < WORD_SIZE; i++) {
        memory[i] = buf[index+i];
    }
    return delay;
}

int Memory::write(int address, double data, int word) {
    int index = (address+(word-1)*WORD_SIZE) - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return -1;
    }
    int delay = 0;
    //Check if in cache
    //If so, just read value and return 0 delays
    //Otherwise, add to cache, read value and return delay cycles
    //transform index to words
    index = index / WORD_SIZE;
    int blockAdd = index/WORDS_PER_BLOCK;
    int setInd = blockAdd % SET_NUMBER;
    bool inCache = false;
    for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
        if(dValid[setInd][i] && dTag[setInd][i] == blockAdd) {
            inCache = true;
            break;
        }
    }
    if(!inCache) {
        if(iUsed) {
            return -2;
        }
        //find open spot in set
        int bind = -1;
        for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
            if(!dValid[setInd][i]) {
                bind = i;
                break;
            }
        }
        //If still -1, then replace oldest item in set
        if(bind >= 0) {
            dValid[setInd][bind] = true;
            dTag[setInd][bind] = blockAdd;
        }
        else {
            dValid[setInd][oldestIndex[setInd]] = true;
            dTag[setInd][oldestIndex[setInd]] = blockAdd;
            oldestIndex[setInd]++;
            if(oldestIndex[setInd] >= BLOCKS_PER_SET) {
                oldestIndex[setInd] = 0;
            }
        }
        delay += WORDS_PER_BLOCK*CYCLES_PER_WORD;
        dUsed = true;
        memDelay += delay;
    }

    if(word == 1) {
        byte_t *buf = new byte_t[DOUBLE_WORD_SIZE];
        memcpy(buf, &data, DOUBLE_WORD_SIZE);
        for(int i = 0; i < DOUBLE_WORD_SIZE; i++) {
            memory[i] = buf[index+i];
        }
    }
    return delay;
}

//check if in chache
//if so, just read the value and use the regular counter
//Otherwise, read in value and set counter to cycles + 4word*3cycles
int Memory::read(int address, int &val) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return -1;
    }
    int delay = 0;
    //Check if in cache
    //If so, just read value and return 0 delays
    //Otherwise, add to cache, read value and return delay cycles
    //transform index to words
    index = index / WORD_SIZE;
    int blockAdd = index/WORDS_PER_BLOCK;
    int setInd = blockAdd % SET_NUMBER;
    bool inCache = false;
    for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
        if(dValid[setInd][i] && dTag[setInd][i] == blockAdd) {
            inCache = true;
            break;
        }
    }
    if(!inCache) {
        //Instruction cache is using mem bus
        if(iUsed) {
            return -2;
        }
        //find open spot in set
        int bind = -1;
        for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
            if(!dValid[setInd][i]) {
                bind = i;
                break;
            }
        }
        //If still -1, then replace oldest item in set
        if(bind >= 0) {
            dValid[setInd][bind] = true;
            dTag[setInd][bind] = blockAdd;
        }
        else {
            dValid[setInd][oldestIndex[setInd]] = true;
            dTag[setInd][oldestIndex[setInd]] = blockAdd;
            oldestIndex[setInd]++;
            if(oldestIndex[setInd] >= BLOCKS_PER_SET) {
                oldestIndex[setInd] = 0;
            }
        }
        delay += WORDS_PER_BLOCK*CYCLES_PER_WORD;
        dUsed = true;
        memDelay = delay;
    }
    byte_t *buf = new byte_t[WORD_SIZE];
    for(int i = 0; i < WORD_SIZE; i++) {
        //Copy bytes in reverse order because of endianess
        buf[WORD_SIZE-i-1] = memory[index+i];
    }
    memcpy(&val, buf, WORD_SIZE);
    return delay;
}

int Memory::readDouble(int address, double &val, int word) {
    int index = (address+(word-1)*WORD_SIZE) - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return -1;
    }
    int delay = 0;
    //Check if in cache
    //If so, just read value and return 0 delays
    //Otherwise, add to cache, read value and return delay cycles
    //transform index to words
    index = index / WORD_SIZE;
    int blockAdd = index/WORDS_PER_BLOCK;
    int setInd = blockAdd % SET_NUMBER;
    bool inCache = false;
    for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
        if(dValid[setInd][i] && dTag[setInd][i] == blockAdd) {
            inCache = true;
            break;
        }
    }
    if(!inCache) {
        if(iUsed) {
            return -2;
        }
        //find open spot in set
        int bind = -1;
        for(int i = 0; i < BLOCK_NUM/SET_NUMBER; i++) {
            if(!dValid[setInd][i]) {
                bind = i;
                break;
            }
        }
        //If still -1, then replace oldest item in set
        if(bind >= 0) {
            dValid[setInd][bind] = true;
            dTag[setInd][bind] = blockAdd;
        }
        else {
            dValid[setInd][oldestIndex[setInd]] = true;
            dTag[setInd][oldestIndex[setInd]] = blockAdd;
            oldestIndex[setInd]++;
            if(oldestIndex[setInd] >= BLOCKS_PER_SET) {
                oldestIndex[setInd] = 0;
            }
        }
        delay += WORDS_PER_BLOCK*CYCLES_PER_WORD;
        dUsed = true;
        memDelay += delay;
    }

    if(word == 1) {
        byte_t *buf = new byte_t[DOUBLE_WORD_SIZE];
        for(int i = 0; i < DOUBLE_WORD_SIZE; i++) {
            //Copy bytes in reverse order because of endianess
            buf[DOUBLE_WORD_SIZE-i-1] = memory[index+i];
        }
        //mash bits together in double
        memcpy(&val, buf, DOUBLE_WORD_SIZE);
    }
    return delay;
}

void Memory::finishData() {
    //FU processing includes cache miss delay so just mark it as done
    dUsed = false;
    memDelay = -1;
}

std::string Memory::getInstruction(int pc) {
    return instructions[pc];
}

void Memory::printCache(std::ofstream &f) {
    print(f, "Inst Cache\n", 10);
    for(int i = 0; i < iBlockNum; i++) {
        std::string msg = "\t" + std::to_string(iValid[i]);
        if(iValid[i]) {
            msg += " " + std::to_string(iTag[i]);
        }
        print(f, msg, 10);
        f << std::endl;
    }
    print(f, "Data Cache\n", 10);
    for(int i = 0; i < SET_NUMBER; i++) {
        print(f, "Set " + std::to_string(i), 10);
        f << std::endl;
        for(int j = 0; j < BLOCKS_PER_SET; j++) {
            std::string msg = "\t" + std::to_string(dValid[i][j]);
            if(dValid[i][j]) {
                msg += " " + std::to_string(dTag[i][j]);
            }
            print(f, msg, 10);
            f << std::endl;
        }
    }
}
