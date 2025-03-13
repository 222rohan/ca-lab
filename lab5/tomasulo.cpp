#include<bits/stdc++.h>
using namespace std;

#define FILENAME "test.txt"

typedef struct {
    string opcode;
    string dest;
    string src1;
    string src2;
    int issue;
    int exec_comp;
    int write_res;
    string res_station;
} instruction;
vector<instruction> instructions;

typedef struct {
    int busy;
    string op;
    string Vj;
    string Vk;
    string Qj;
    string Qk;
} reservation_station;
map<string, vector<reservation_station>> res_stations;

map<string, string> register_result_status;
map<string, int> functional_units = {
    {"ADD", 0},
    {"MUL", 0},
    {"FADD", 0},
    {"FMUL", 0},
    {"LD", 0},
    {"SD", 0},
    {"LU", 0}
};
map<string, int> exec_time = {
    {"ADD", 6},
    {"MUL", 12},
    {"FADD", 18},
    {"FMUL", 30},
    {"LD", 1},
    {"SD", 1},
    {"LU", 1}
};

int clock_cycle = 1;

int is_valid_opcode(string opcode) {
    if(opcode == "ADD" || opcode == "MUL" || opcode == "FADD" || opcode == "FMUL" || opcode == "LD" || opcode == "SD" || opcode == "AND" || opcode == "OR" || opcode == "XOR" || opcode == "NOT") {
        return 1;
    }
    return 0;
}

void init_reservation_stations(){
    reservation_station add1, add2, mul1, mul2, fadd1, fadd2, fadd3, fadd4, fmul1, fmul2, fmul3, fmul4, ld1, ld2, sd1, sd2, lu1, lu2;
    vector<reservation_station> temp;
    temp.push_back(add1);
    temp.push_back(add2);
    res_stations["ADD"] = temp;
    temp.clear();

    temp.push_back(mul1);
    temp.push_back(mul2);
    res_stations["MUL"] = temp;
    temp.clear();

    temp.push_back(fadd1);
    temp.push_back(fadd2);
    temp.push_back(fadd3);
    temp.push_back(fadd4);
    res_stations["FADD"] = temp;
    temp.clear();

    temp.push_back(fmul1);
    temp.push_back(fmul2);
    temp.push_back(fmul3);
    temp.push_back(fmul4);
    res_stations["FMUL"] = temp;
    temp.clear();

    temp.push_back(ld1);
    temp.push_back(ld2);
    res_stations["LD"] = temp;
    temp.clear();

    temp.push_back(sd1);
    temp.push_back(sd2);
    res_stations["SD"] = temp;
    temp.clear();

    temp.push_back(lu1);
    temp.push_back(lu2);
    res_stations["LU"] = temp;
    
    return;
}

/*

    Function to parse through all instructions
    and group them into packets

*/
void instruction_parser( ifstream &file ) {

    string line;

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

        //cout << opcode << ":" << dest << ":" << src1 << ":" << src2 << endl;
        instruction temp;
        temp.opcode = opcode;
        temp.dest = dest;
        temp.src1 = src1;
        temp.src2 = src2;

        instructions.push_back(temp);

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

    init_reservation_stations();

    return 0;
}