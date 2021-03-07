#include<iostream>
#include<fstream>
#include <sstream>
#include<vector>
#include<map>
using namespace std;
struct Instruction
{
    string name;
    string field_1;
    string field_2;
    string field_3;
};
vector <string> words;              //stores the entire file as a vector of strings
map<int,string> register_numbers;  //maps each number in 0-31 to a register
map<string,int> register_values; //stores the value of data stored in each register
bool validFile = true;          //will be false if file is invalid at any point
vector<Instruction>instructs;   //stores instructions as structs
int memory[(1<<20)]={0}; //memory used to store the data
int PC=0;               // PC pointer, points to the next instruction

void map_register_numbers(){
    //maps each register to a unique number between 0-31 inclusive
    register_numbers[0]="$r0";
    register_numbers[1]="$at";
    register_numbers[2]="$v0";
    register_numbers[3]="$v1";
    for(int i=4;i<=7;i++){
        register_numbers[i]="$a"+to_string(i-4);
    }
    for(int i=8;i<=15;i++){
        register_numbers[i]="$t"+to_string(i-8);
    }
    for(int i=16;i<=23;i++){
        register_numbers[i]="$s"+to_string(i-16);
    }
    register_numbers[24]="$t8";
    register_numbers[25]="$t9";
    register_numbers[26]="$k0";
    register_numbers[27]="$k1";
    register_numbers[28]="$gp";
    register_numbers[29]="$sp";
    register_numbers[30]="$s8";
    register_numbers[31]="$ra";
}

void initialise_Registers(){
    //initialises all the registers
    for(int i=0;i<32;i++){
        register_values[register_numbers[i]]=0;
    }
    register_values["$sp"]=2147479548;
}

bool valid_register(string R){
    return register_values.find(R)!=register_values.end();
}

bool is_integer(string s){
    for(int j=0;j<s.length();j++){
        if(isdigit(s[j]) == false){
            return false;
        }
    }
    return true;
}

//handle the case when integer is beyond instruction memory at execution time

void Create_structs(vector<string>words){
    
    for(int i=0;i<words.size();i++){
        
        if(words[i] == "add" || words[i] == "sub" || words[i] == "mul" || words[i] == "slt"){
            //now, there must be three registers ahead
            if(i+3>=words.size()){
                validFile=false;
                return;
            }
            else{
                string R1=words[i+1];
                string R2=words[i+2];
                string R3=words[i+3];
                if(valid_register(R1) && valid_register(R2) && valid_register(R3)){
                    //these are valid register names
                    struct Instruction new_instr;
                    new_instr.name=words[i];
                    
                    new_instr.field_1 = R1;
                    new_instr.field_2 = R2;
                    new_instr.field_3 = R3;
                    
                }
                else{
                    validFile = false;
                    return;
                }
                
            }
        }
        else if (words[i]=="beq" || words[i]=="bne" || words[i]=="addi"){
            if(i+3>=words.size()){
                validFile=false;
                return;
            }
            else{
                string R1=words[i+1];
                string R2=words[i+2];
                string R3=words[i+3];
                if(valid_register(R1) && valid_register(R2) && is_integer(R3)){
                    //these are valid register names
                    struct Instruction new_instr;
                    new_instr.name=words[i];
                    new_instr.field_1 = R1;
                    new_instr.field_2 = R2;
                    new_instr.field_3 = R3;
                }
                else{
                    validFile = false;
                    return;
                }
                         
            }
        }
        else if (words[i]=="j"){
            if(i+1>=words.size()){
                validFile=false;
                return;
            }
            else{
                string R1=words[i+1];
                if(is_integer(R1)){
                    struct Instruction new_instr;
                    new_instr.name=words[i];
                    new_instr.field_1 = R1;
                    new_instr.field_2 = "";
                    new_instr.field_3 = "";
                }
            }
        }
        else if (words[i] == "lw" || words[i] == "sw"){
            //this has the format lw $t0, offset($register_name)
            if(i+2>=words.size()){
                validFile=false;
                return;
            }
            else{
                string R1 = words[i+1];
                string R2 = words[i+2];
                if(valid_register(R1)){
                    string offset="";
                    for(int i=0;i<R2.length();i++){
                        if(R2[i]!='('){
                            offset += R2[i];
                        }
                        else{
                            break;
                        }
                    }
                    if(offset.size()+5!=R2.size()){  //size required for ($reg_name)
                        validFile=false;
                        return;
                    }
                    else{
                        if(R2[offset.size()]=='(' && R2[R2.size()-1] == ')'){
                            //now, the string inside must be a register name
                            string inside = R2.substr(offset.size()+1,R2.size()-offset.size()-2);
                            if(valid_register(inside) && is_integer(offset)){
                                // check if offset divides 4 during execution
                                struct Instruction new_instr;
                                new_instr.name=words[i];
                                new_instr.field_1 = R1;
                                new_instr.field_2 = offset;
                                new_instr.field_3 = inside;
                            }
                            else{
                                validFile=false;
                                return;
                            }
                        }
                        else{
                            validFile=false;
                            return;
                        }
                    }
                }
                else{
                    validFile = false;
                    return;
                }
            }
        }
        else{
            validFile=false;
            return;
        }
    }
}
int main(){
    ifstream file("mips_program.txt");
    string current_line;
    map_register_numbers();
    initialise_Registers();
    while(file){
        getline(file,current_line);
        istringstream ss(current_line);
        string word;
        while(ss >> word){
           words.push_back(word);  //storing all the words in the word vector
        }
    }
    Create_structs(words); //creates structs of instructions from words
    
    if(!validFile){
        cout<<"Invalid MIPS program"<<endl;
        return -1;
    }
    
    /*each instruction occupies 4 bytes. So, we will first of all maintain an array of instructions
     to get the instruction starting at memory address i (in the form of a struct). Rest of the memory is used in RAM*/
    /*so memory stores instructions (as structs here) and data as integers in decimal format*/
    
    return 0;
}

