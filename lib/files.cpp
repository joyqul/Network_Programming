#include <iostream>
#include <strings.h>
#include "files.h"
#include "readline.h"

long get_file_len(std::ifstream* fp) {
    fp->seekg(0, fp->end);
    long len = fp->tellg();
    fp->seekg(0, fp->beg);
    return len;
}

void generate_file_description(std::ifstream* fp, char* buf, std::string filename, int& redundent, long& packet_num) {
    long filesize = get_file_len(fp);
    redundent = filesize%(BUF_SIZE-0);
    packet_num = filesize/(BUF_SIZE-0) + 1;
    bzero(buf, BUF_SIZE);
    sprintf(buf, "SENDFILE%s\n%d\n%ld\n", filename.c_str(), redundent, packet_num); /* redundent */
}

void get_file_description(std::string& message, std::string& filename, int& redundent, long& filesize) {
    int i;
    for (i = 8; i < message.size(); ++i) {
        if (message[i] == '\0') break;
        if (message[i] == '\r') break;
        if (message[i] == '\n') break;
        filename += message[i];
    }

    ++i;
    redundent = 0;
    for (; i < message.size(); ++i) {
        if (message[i] == '\0') break;
        if (message[i] == '\r') break;
        if (message[i] == '\n') break;
        redundent = redundent*10 + message[i] - '0';
    }

    ++i;
    filesize = 0;
    for (; i < message.size(); ++i) {
        if (message[i] == '\0') break;
        if (message[i] == '\r') break;
        if (message[i] == '\n') break;
        filesize = filesize*10 + message[i] - '0';
    }
    //std::cout << filename << " " << redundent << " " << filesize << std::endl;
}
