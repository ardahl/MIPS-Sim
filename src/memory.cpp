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
    //I-Cache
    iValid = new bool[iBlockNum];
    iTag = new int[iBlockNum];
    for(int i = 0; i < iBlockNum; i++) {
        iValid[i] = false;
    }
}

//Keep internal counter for caching
bool Memory::availInstruction(int pc) {
    if(memDelay == -1) {   //New instruction to check for cache
        int btag = pc / iBlockSize;
        int bindex = btag % iBlockNum;
        //Check if pc is in cache
        if(iValid[bindex] && iTag[bindex] == btag) {
            //Cache hit
            return true;
        }
        else {  //Cache miss, set counter and replace block
            memDelay = iBlockSize*CYCLES_PER_WORD;
            iValid[bindex] = true;
            iTag[bindex] = btag;
            return false;
        }
    }
    else if(memDelay > 0) { //Instruction was a cache miss, so we're counting down
        memDelay--;
        return false;
    }
    else {
        memDelay = -1;
        return pc < (int)instructions.size();
    }
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
    while(std::getline(in, line)) {
        if((n=ci_find_substr(line, std::string("I-Cache"))) != -1) {
            std::string num = line.substr(n+8, line.find_first_of(",")-n-8);
            std::string num2 = line.substr(line.find_first_of(",")+1);
            iBlockNum = std::stoi(num);
            iBlockSize = std::stoi(num2);
        }
    }
}

bool Memory::write(int address, int data) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[WORD_SIZE];
    memcpy(buf, &data, WORD_SIZE);
    for(int i = 0; i < WORD_SIZE; i++) {
        memory[i] = buf[index+i];
    }
    return true;
}

bool Memory::write(int address, double data) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[DOUBLE_WORD_SIZE];
    memcpy(buf, &data, DOUBLE_WORD_SIZE);
    for(int i = 0; i < DOUBLE_WORD_SIZE; i++) {
        memory[i] = buf[index+i];
    }
    return true;
}

bool Memory::read(int address, int &val) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[WORD_SIZE];
    for(int i = 0; i < WORD_SIZE; i++) {
        //Copy bytes in reverse order because of endianess
        buf[WORD_SIZE-i-1] = memory[index+i];
    }
    memcpy(&val, buf, WORD_SIZE);
    return true;
}

bool Memory::readDouble(int address, double &val) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[DOUBLE_WORD_SIZE];
    for(int i = 0; i < DOUBLE_WORD_SIZE; i++) {
        //Copy bytes in reverse order because of endianess
        buf[DOUBLE_WORD_SIZE-i-1] = memory[index+i];
    }
    //mash bits together in double
    memcpy(&val, buf, DOUBLE_WORD_SIZE);
    return true;
}

std::string Memory::getInstruction(int pc) {
    return instructions[pc];
}
