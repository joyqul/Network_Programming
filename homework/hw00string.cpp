#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;
string instr, target;
char token;

void ReverseString() {
    reverse(target.begin(), target.end());
    cout << target << endl;
}

void SplitString() {
    istringstream iss(target);
    string split;
    int num = 0;
    while (getline(iss, split, token)) {
        if (split == "") continue;
        if (num) cout << " ";
        cout << split;
        ++num;
    }
    cout << endl;
}

int main (int argc, char* argv[]) {
    // Check argument number
    if (argc != 3) {
        cout << "usage: ./main [input file] [split token]" << endl;
        return 0;
    }

    // Open file
    fstream fp;
    fp.open(argv[1], ios::in);

    // Check if file cannot be openned
    if (!fp) {
        cout << "Fail to open file: " << argv[1] << endl;
        return 0;
    }

    // Read the file
    token = argv[2][0];
    while (fp >> instr) {
        if (instr == "reverse") {
            fp >> target;
            ReverseString();
        }
        else if (instr == "split") {
            fp >> target;
            SplitString();
        }
        else if (instr == "exit") {
            fp.close();
            return 0;
        }
    }
    fp.close();

    // Read the std::in
    while (cin >> instr, instr != "exit") {
        if (instr == "reverse") {
            cin >> target;
            ReverseString();
        }
        else if (instr == "split") {
            cin >> target;
            SplitString();
        }
        else if (instr == "exit") {
            return 0;
        }
    }

    return 0;
}
