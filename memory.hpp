#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include "defines.hpp"
#include <unordered_map>

class Memory {
public:
    Memory();
    Memory(std::string inst, std::string data);

    bool write(int address, int data);
    bool write(int address, float data);
    bool read(int address, byte_t* buf);
    bool readDouble(int address, byte_t* buf);
    bool availInstruction(int pc);
    std::string getInstruction(int pc);
    std::unordered_map<std::string, int> getBranches() { return branches; }

private:
    std::vector<std::string> instructions;
    byte_t memory[MEM_SIZE];
    std::unordered_map<std::string, int> branches;

    void readInstructions(std::string inst);
    void readData(std::string data);
};

#endif
