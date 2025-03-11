#include<bits/stdc++.h>

using namespace std;

#define FILENAME "test.txt"

typedef struct {
    string opcode;
    string dest;
    string src1;
    string src2;
} instruction;

typedef struct {
    instruction iadd;
    instruction imul;
    instruction fadd;
    instruction fmul;
    instruction load;
    instruction store;
    instruction lu;
} instruction_packet;

map<string, int> register_file;
// map<string, int> functional_units = {
//     {"ADD", 0},
//     {"MUL", 0},
//     {"FADD", 0},
//     {"FMUL", 0},
//     {"LD", 0},
//     {"SD", 0},
//     {"LU", 0}
// };

int is_valid_opcode(string opcode) {
    if(opcode == "ADD" || opcode == "MUL" || opcode == "FADD" || opcode == "FMUL" || opcode == "LD" || opcode == "SD" || opcode == "AND" || opcode == "OR" || opcode == "XOR" || opcode == "NOT") {
        return 1;
    }
    return 0;
}

/*

    Function to parse through all instructions
    and group them into packets

*/
void instruction_parser( ifstream &file ) {

    string line;
    instruction_packet packet;

    //go through each line
    while(getline(file, line)) {
        //cout << line << endl;

        string opcode, dest, src1, src2 = "";
        
        //split the line based on space, then remove the commas
        istringstream iss(line);
        vector<string> tokens;
        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(tokens));
        //remove the commas from the tokens
        for(int i = 0; i < tokens.size(); i++) {
            tokens[i].erase(remove(tokens[i].begin(), tokens[i].end(), ','), tokens[i].end());
        }

        //check if the opcode is valid
        if(!is_valid_opcode(tokens[0])) {
            cout << "Invalid opcode: " << tokens[0] << endl;
            return;
        }

        

        opcode = tokens[0];
        dest = tokens[1];
        src1 = tokens[2];

        //check if there is a second source
        if(tokens.size() > 3) {
            src2 = tokens[3];
        }

        cout << opcode << ":" << dest << ":" << src1 << ":" << src2 << endl;

    }

    file.close();
}

int main() {
    ifstream file(FILENAME);

    if(!file.is_open()) {
        cout << "File not found!" << endl;
        return 0;
    }

    instruction_parser(file);

    return 0;
}