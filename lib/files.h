#include <fstream>

void get_file_description(std::string&, std::string&, int&, long&);
void generate_file_description(std::ifstream*, char*, std::string, int&, long&);
long get_file_len(std::ifstream*);
