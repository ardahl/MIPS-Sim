#include "memory.hpp"

Memory::Memory() {

}

Memory::Memory(std::string inst, std::string data) {
    readInstructions(inst);
    readData(data);
}

bool Memory::availInstruction(int pc) {
    return pc < instructions.size();
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
        ss << c;
        bitnum++;
        if(bitnum == 8) {
            std::bitset<8> bits(ss.str());
            ss.str(std::string());
            unsigned long tmp = bits.to_ulong();
            memory[count] = static_cast<byte_t>(tmp);
            count++;
            bitnum = 0;
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
}

bool Memory::write(int address, float data) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[DOUBLE_WORD_SIZE];
    memcpy(buf, &data, DOUBLE_WORD_SIZE);
    for(int i = 0; i < DOUBLE_WORD_SIZE; i++) {
        memory[i] = buf[index+i];
    }
}

bool Memory::read(int address, int &val) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[WORD_SIZE];
    for(int i = 0; i < WORD_SIZE; i++) {
        buf[i] = memory[index+i];
    }
    memcpy(&val, buf, WORD_SIZE);
}

bool Memory::readDouble(int address, double &val) {
    int index = address - DATA_BEGIN;
    if(index < 0 || index >= MEM_SIZE) {
        return false;
    }
    byte_t *buf = new byte_t[DOUBLE_WORD_SIZE];
    for(int i = 0; i < DOUBLE_WORD_SIZE; i++) {
        buf[i] = memory[index+i];
    }
    //mash bits together in double
    memcpy(&val, buf, DOUBLE_WORD_SIZE);
}

std::string Memory::getInstruction(int pc) {
    return instructions[pc];
}
