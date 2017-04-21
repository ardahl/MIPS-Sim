#include "pipeline.hpp"

int main(int argc, char** argv) {
    if(argc != 5) {
        printf("Usage: ./pipeline <instruction_input> <data_input> <config_input> <output>\n");
        return 0;
    }
    std::string inst = std::string(argv[1]);
    std::string data = std::string(argv[2]);
    std::string config = std::string(argv[3]);
    std::string output = std::string(argv[4]);
    // std::string data(""), config(""), output("");

    Pipeline pipe(inst, data, config, output);
    pipe.run();

    return 0;
}
