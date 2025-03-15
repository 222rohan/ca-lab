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
    int instr_index;
    int time_to_finish;
    int busy;
    int idle; //instruction is ready to be executed, but waiting for result
    string op;
    string Vj;
    string Vk;
    string Qj;
    string Qk;

} reservation_station;
map<string, vector<reservation_station>> reservation_stations;

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
    // reservation_station add1, add2, mul1, mul2, fadd1, fadd2, fadd3, fadd4, fmul1, fmul2, fmul3, fmul4, ld1, ld2, sd1, sd2, lu1, lu2;
    // vector<reservation_station> temp;
    // temp.push_back(add1);
    // temp.push_back(add2);
    // reservation_stations["ADD"] = temp;
    // temp.clear();

    // temp.push_back(mul1);
    // temp.push_back(mul2);
    // reservation_stations["MUL"] = temp;
    // temp.clear();

    // temp.push_back(fadd1);
    // temp.push_back(fadd2);
    // temp.push_back(fadd3);
    // temp.push_back(fadd4);
    // reservation_stations["FADD"] = temp;
    // temp.clear();

    // temp.push_back(fmul1);
    // temp.push_back(fmul2);
    // temp.push_back(fmul3);
    // temp.push_back(fmul4);
    // reservation_stations["FMUL"] = temp;
    // temp.clear();

    // temp.push_back(ld1);
    // temp.push_back(ld2);
    // reservation_stations["LD"] = temp;
    // temp.clear();

    // temp.push_back(sd1);
    // temp.push_back(sd2);
    // reservation_stations["SD"] = temp;
    // temp.clear();

    // temp.push_back(lu1);
    // temp.push_back(lu2);
    // reservation_stations["LU"] = temp;

    //initialize all reservation stations, and set their variables
    reservation_station rs;
    rs.busy = 0;
    rs.idle = 0;
    rs.Vj = "";
    rs.Vk = "";
    rs.Qj = "";
    rs.Qk = "";
    rs.time_to_finish = 0;
    rs.instr_index = -1;
    
    vector<reservation_station> temp;
    
    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["ADD"] = temp;
    temp.clear();

    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["MUL"] = temp;
    temp.clear();

    temp.push_back(rs);
    temp.push_back(rs);
    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["FADD"] = temp;
    temp.clear();

    temp.push_back(rs);
    temp.push_back(rs);
    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["FMUL"] = temp;
    temp.clear();

    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["LD"] = temp;
    temp.clear();

    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["SD"] = temp;
    temp.clear();

    temp.push_back(rs);
    temp.push_back(rs);
    reservation_stations["LU"] = temp;
    temp.clear();



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

void execute_instruction(reservation_station &station, string opcode){
    station.idle = 0;
    cout << "Executing instruction: " << station.instr_index + 1 << endl;
    int time_to_finish = exec_time[opcode];
    station.time_to_finish = clock_cycle + time_to_finish;
    return;
}

bool program_complete(bool completed[], int num_instr){
    for(int i = 0; i < num_instr; i++){
        if(!completed[i]){
            return false;
        }
    }
    return true;
}

void check_for_completion(bool *completed) {
    //check if any instruction has completed execution
    for(auto station: reservation_stations) {
        for(auto rs: station.second){
            if(rs.busy && rs.time_to_finish == clock_cycle){
                rs.busy = 0;
                rs.Vj = "";
                rs.Vk = "";
                rs.Qj = "";
                rs.Qk = "";
                rs.time_to_finish = -1;
                instructions[rs.instr_index].exec_comp = clock_cycle;
                instructions[rs.instr_index].write_res = clock_cycle + 1;
                rs.idle = 0;
                cout << "Setting register " << register_result_status[instructions[rs.instr_index].dest] << " as free" << endl;
                register_result_status[instructions[rs.instr_index].dest] = "";
                cout << "Instruction " << rs.instr_index + 1 << " completed" << endl;
                completed[rs.instr_index] = true;

                //cout << "Instruction " << instructions[rs.instr_index].opcode << " " << instructions[rs.instr_index].dest << " " << instructions[rs.instr_index].src1 << " " << instructions[rs.instr_index].src2 << " completed at clock cycle " << clock_cycle << endl;
            }
        }
    }

    //go through all reservation stations, and check for dependencies in result
    for(auto &station: reservation_stations) {
        for(auto &rs: station.second){
            if(rs.busy){
                bool special = false;
                if(rs.Qj != "" && register_result_status[rs.Qj] == ""){
                    rs.Vj = rs.Qj;
                    rs.Qj = "";
                    special = true;
                }
                if(rs.Qk != "" && register_result_status[rs.Qk] == ""){
                    rs.Vk = rs.Qk;
                    rs.Qk = "";
                    special = true;
                }
                if(special && rs.Vj != "" && rs.Vk != ""){
                    cout << "idleset completetion finction " << rs.instr_index << endl;
                    rs.idle = 1;
                }
            }
        }
    }
}

void execute_idle_instructions(){
    for(auto &station: reservation_stations){
        for(auto &rs: station.second){
            if(rs.idle && rs.Vj != "" && rs.Vk != ""){
                cout << "idelset instrno. " << rs.instr_index << endl;
                execute_instruction(rs, rs.op);
            }
        }
    }
}

void schedule_instructions(){
    //inorder issue
    int num_instr = instructions.size();
    bool completed[num_instr];
    int issued_instruction = 0;
    for(int i = 0; i < num_instr; i++){
        completed[i] = false;
    }
    
    while(!program_complete(completed, num_instr) && clock_cycle < 100){

        cout << "Clock cycle: " << clock_cycle << endl;
        
        for(auto i: completed){
            cout << i << " ";
        }
        cout << endl;

        //check for reservation stations with instructions ready to be executed (no dependencies)
        execute_idle_instructions();

        //issue the instruction
        if(!completed[issued_instruction]){

            instruction instr = instructions[issued_instruction];

            cout << "Issuing instruction no(" << issued_instruction+1 << ")" << instr.opcode << ":" << instr.dest << ":" << instr.src1 << ":" << instr.src2 << endl;

            string opcode = instr.opcode;

            if(opcode == "AND" || opcode == "OR" || opcode == "XOR" || opcode == "NOT"){
                opcode = "LU";
            }

            for(auto &station: reservation_stations[opcode]){
                if(!station.busy) {
                    instructions[issued_instruction].issue = clock_cycle;
                    station.busy = 1;
                    station.idle = 0;
                    station.op = instr.opcode;
                    
                    //set destination register as busy for dependency check
                    register_result_status[instr.dest] = instr.dest;                    
                   //register result status is empty for all cases for wahteevr reason pls fix
                   //something to do with map maybe?

                   cout << "checking for dependencies" << " SRC1:" << register_result_status[instr.src1] << " SRC2:" << register_result_status[instr.src2] << endl;


                    if(register_result_status[instr.src1] != ""){
                        station.Qj = register_result_status[instr.src1];
                    }
                    else{
                        station.Vj = instr.src1;
                    }

                    if(register_result_status[instr.src2] != ""){
                        station.Qk = register_result_status[instr.src2];
                    }
                    else{
                        station.Vk = instr.src2;
                    }

                    if(station.Vj != "" && station.Vk != ""){
                        //execute the instruction
                        execute_instruction(station, opcode);
                    }

                    station.instr_index = issued_instruction;
                    issued_instruction++;
                    break;
                }
            }
            
        }

        //check if any instruction has completed execution
        check_for_completion(completed);

        clock_cycle++;

    }
}

void print_stats(){
    cout << "Instruction\tIssue\tExec\tWrite Res" << endl;
    for(auto instr: instructions){
        cout << instr.opcode << " " << instr.dest << " " << instr.src1 << " " << instr.src2 << "\t" << instr.issue << "\t" << instr.exec_comp << "\t" << instr.write_res << endl;
    }
}

int main() {
    ifstream file(FILENAME);

    if(!file.is_open()) {
        cout << "File not found!" << endl;
        return 0;
    }

    instruction_parser(file);

    init_reservation_stations();

    schedule_instructions();

    print_stats();

    return 0;
}