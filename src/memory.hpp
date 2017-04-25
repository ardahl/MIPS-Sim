#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include "defines.hpp"
#include <unordered_map>

//TODO: data cache

//An instruction is considered a word
#define CYCLES_PER_WORD 3

class Memory {
public:
    Memory();
    Memory(std::string inst, std::string data, std::string config);

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
    std::string lastInst;
    int memDelay;
    int iBlockNum, iBlockSize;

    //I-Cache
    //Cache tag: pc / block size
    //Cache block index: cache tag % number of blocks
    //Each array is of length number of blocks
        //Don't need to store the actual data since it's just an index into an array
    bool *iValid;
    int *iTag;

    //D-Cache

    void readInstructions(std::string inst);
    void readData(std::string data);
    void readCache(std::string config);
};

#endif
