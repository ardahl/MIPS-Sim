#include "defines.hpp"

//Even though this is called defines.cpp, it's really helpers.cpp. It includes
//various general purpose functions

// Helper functions for trimming whitespace from a string
//Found here: https://gist.github.com/dedeexe/9080526
//Left trim
std::string trim_left(const std::string& str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(str.find_first_not_of(pattern));
}

//Right trim
std::string trim_right(const std::string& str) {
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(0,str.find_last_not_of(pattern) + 1);
}

//Left and Right trim
std::string trim(const std::string& str) {
  return trim_left(trim_right(str));
}
