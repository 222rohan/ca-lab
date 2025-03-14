#include<bits/stdc++.h>
using namespace std;

#define FILENAME "test.txt"

typedef struct {
    string opcode;
    string dest;
    string src1;
    string src2;
} instruction;
vector<instruction> instructions;

typedef struct {
    string add;
    string mul;
    string fadd;
    string fmul;
    string ld;
    string sd;
    string lu;
} instruction_packet;

map<string, int> register_read;
map<string, int> register_write;
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

void add_instruction_to_packet(instruction_packet &packet, instruction instr){
    string opcode = instr.opcode;
    string final_instr = opcode + " " + instr.dest + " " + instr.src1 + " " + instr.src2;
    if(opcode == "ADD"){
        packet.add = final_instr;
    }
    else if(opcode == "MUL"){
        packet.mul = final_instr;
    }
    else if(opcode == "FADD"){
        packet.fadd = final_instr;
    }
    else if(opcode == "FMUL"){
        packet.fmul = final_instr;
    }
    else if(opcode == "LD"){
        packet.ld = final_instr;
    }
    else if(opcode == "SD"){
        packet.sd = final_instr;
    }
    else if(opcode == "AND" || opcode == "OR" || opcode == "XOR" || opcode == "NOT"){
        packet.lu = final_instr;
    }
}

void clear_registers_and_units(instruction_packet &packet){
    //empty the register_read and register_write maps
    register_read.clear();
    register_write.clear();
    //empty the functional units
    for(auto &fu : functional_units){
        fu.second = 0;
    }

    //empty the packet
    packet.add = "";
    packet.mul = "";
    packet.fadd = "";
    packet.fmul = "";
    packet.ld = "";
    packet.sd = "";
    packet.lu = "";
}

void print_packet(instruction_packet &packet, int max_time){
    //add NOP if a functional unit doesnt have any instruction
    if(packet.add == ""){
        packet.add = "NOP";
    }
    if(packet.mul == ""){
        packet.mul = "NOP";
    }
    if(packet.fadd == ""){
        packet.fadd = "NOP";
    }
    if(packet.fmul == ""){
        packet.fmul = "NOP";
    }
    if(packet.ld == ""){
        packet.ld = "NOP";
    }
    if(packet.sd == ""){
        packet.sd = "NOP";
    }
    if(packet.lu == ""){
        packet.lu = "NOP";
    }
    //print the packet
    printf("{ %s, %s, %s, %s, %s, %s, %s }\t%d\t%d\n", packet.add.c_str(), packet.mul.c_str(), packet.fadd.c_str(), packet.fmul.c_str(), packet.ld.c_str(), packet.sd.c_str(), packet.lu.c_str(), clock_cycle, clock_cycle + max_time);
    //increment the clock cycle
    clock_cycle += max_time + 1;

    //clear the packet
    clear_registers_and_units(packet);
}

void schedule_packets(){
    instruction_packet packet;
    int n = instructions.size();
    int max_time = 0;
    for(int i = 0; i < n; i++){
        instruction instr = instructions[i];
        string opcode = instr.opcode;
        string dest = instr.dest;
        string src1 = instr.src1;
        string src2 = instr.src2;
        int is_src2 = 0;
        if(src2 != ""){
            is_src2 = 1;
        }

        if(opcode == "AND" || opcode == "OR" || opcode == "XOR" || opcode == "NOT"){
            opcode = "LU";
        }

        if(functional_units[opcode] == 0){
            // cout << opcode << endl;

            //RAW
            if(register_write[src1] == 1 || (is_src2 && register_write[src2] == 1)){
                // cout << "RAW" << endl;
                print_packet(packet, max_time);
                max_time = 0;
                i--;
                continue;
            }
            //WAW
            if(register_write[dest] == 1){
                // cout << "WAW" << endl;
                print_packet(packet, max_time);
                max_time = 0;
                i--;
                continue;
            }
            //WAR
            if(register_read[dest] == 1){
                // cout << "WAR" << endl;
                print_packet(packet, max_time);
                max_time = 0;
                i--;
                continue;
            }

            //no hazards
            // cout << "no hazards" << endl;

            functional_units[opcode] = 1;
            register_write[dest] = 1;
            register_read[src1] = 1;
            if(is_src2){
                register_read[src2] = 1;
            }

            add_instruction_to_packet(packet, instr);
            if(exec_time[opcode] > max_time){
                max_time = exec_time[opcode];
            }
        }
        else{
            //structural hazard
            // cout << "structural hazard" << endl;
            print_packet(packet, max_time);
            max_time = 0;
            i--;
        }
    }
    print_packet(packet, max_time);
}

int main() {
    ifstream file(FILENAME);

    if(!file.is_open()) {
        cout << "File not found!" << endl;
        return 0;
    }

    instruction_parser(file);

    schedule_packets();

    return 0;
}