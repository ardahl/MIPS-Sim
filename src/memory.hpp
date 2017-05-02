#ifndef MEMORY_HPP_
#define MEMORY_HPP_

#include "defines.hpp"
#include <unordered_map>

//TODO: data cache

//An instruction is considered a word
#define CYCLES_PER_WORD 3
#define SET_NUMBER 2
#define BLOCK_NUM 4
#define BLOCKS_PER_SET 2
#define WORDS_PER_BLOCK 4

class Memory {
public:
    Memory();
    Memory(std::string inst, std::string data, std::string config);
    ~Memory();

    int write(int address, int data);
    int write(int address, double data, int word);
    int read(int address, int &val);
    int readDouble(int address, double &val, int word);
    bool availInstruction(int pc);
    std::string getInstruction(int pc);
    std::unordered_map<std::string, int> getBranches() { return branches; }
    bool cacheBusy();
    void finishData();
    void printCache(std::ofstream &f);
    void getNumberAccesses(int &iCache, int &dCache) { iCache = iaccess; dCache = daccess; }
    void getNumberMisses(int &iCache, int &dCache) { iCache = imisses; dCache = dmisses; }

private:
    std::vector<std::string> instructions;
    byte_t memory[MEM_SIZE];
    std::unordered_map<std::string, int> branches;
    std::string lastInst;
    int iaccess, imisses;
    int daccess, dmisses;
    int memDelay;
    int iBlockNum, iBlockSize;

    //I-Cache
    //Cache tag: pc / block size
    //Cache block index: cache tag % number of blocks
    //Each array is of length number of blocks
        //Don't need to store the actual data since it's just an index into an array
    bool *iValid;
    int *iTag;
    bool iUsed;

    //D-Cache
    //first level of pointers is for each set
    //Second level it the set itself
    bool **dValid;
    int **dTag;
    int *oldestIndex;
    bool dUsed;

    void readInstructions(std::string inst);
    void readData(std::string data);
    void readCache(std::string config);
};

#endif
