#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include "defines.hpp"
#include <unordered_map>

class Memory {
public:
    Memory();
    Memory(std::string inst, std::string data);

    bool write(int address, int data);
    bool write(int address, double data);
    bool read(int address, int &val);
    bool readDouble(int address, double &val);
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
