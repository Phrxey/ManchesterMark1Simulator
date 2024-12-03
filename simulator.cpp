#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cassert>

using namespace std;

//constant defining the size of memory
const int rowMax = 32;
const int columnMax = 64;

//global variables for tracking processor state
int step = 0;           //current execution step
int address = 0;        //target address or opperand 
int IAF = 0;            //IAF: Immediate Addressing Flag
int opCode = 8;         //operation code
int opNum = 0;          //operand value

//processor struct containing memory, control reisters, accumulator
struct Processor{
    int memory[rowMax][columnMax];
    int CI;
    int PI[rowMax];
    int accumulator[rowMax];
};

void plus(int array1[], int array2[], int result[]);
void loadNeg(int array[]);
void substract(int array1[], int array2[], int result[]);
int powerOfTwo(int n);
int TC2Dec(int array[]);
void extendTo32(int address[], int ins[]);
void JMP(Processor &pro);
void JRP(Processor &pro);
void LDN(Processor &pro);
void STO(Processor &pro);
void LDP(Processor &pro);
void SUB(Processor &pro);
void CMP(Processor &pro);
void STP(Processor &pro);
int getOperand(const int input[]);
int getOpNum(const int input[]);
int getOpcode(const int input[]);
void initialize(Processor &pro);
void readInMemory(Processor &pro);
void fetch(Processor &pro);
void decode(const Processor &pro);
void execute(Processor &pro);
void printOut(Processor &pro);
void displayMemory(const Processor &pro);
void finalOutput(Processor &pro);

//funtion to add 2 binary arrays
void plus(int array1[], int array2[], int result[]){
    int carry = 0;
    for(int i = 0; i < 32; i++){
        int sum = array1[i] + array2[i] + carry;
        result[i] = sum % 2;
        carry = sum / 2;
    }
}

//function to get the two's complement of a binary array
void loadNeg(int array[]){
    int invert[32];
    int one[32] = {0};
    one[0] = 1;
    for(int i = 0; i < 32; i++){
        invert[i] = (array[i] == 0 ? 1 : 0);
    }
    ::plus(invert,one,array);   //add 1
}

//function to substract with addition method
void substract(int array1[], int array2[], int result[]){
    loadNeg(array2);
    ::plus(array1,array2,result);
}

int powerOfTwo(int n){
    if(n <= 0){
        return 1;
    }else{
        return 2 * powerOfTwo(n - 1);
    }
}

//function to directly handle two's complement conersion to decimal
int TC2Dec(int array[]){
    long long output = 0;
    for(int i = 0; i < 31; i++){
        output += array[i] * powerOfTwo(i);
    }
    output -= array[31] * powerOfTwo(31);
    return output;
}

void JMP(Processor &pro){
    pro.CI = address;
}

void JRP(Processor &pro){
    int val[32] = {0};
    if(IAF == 0){
        for(int i = 0; i < 32; i++){
            val[i] = pro.memory[i][address];
        }
        pro.CI += TC2Dec(val);
    }else{
        pro.CI += address;
    }
}

void LDN(Processor &pro){
    int val[32] = {0};
    if(IAF == 1){
        extendTo32(pro.PI, val);
        loadNeg(val);
        for(int i = 0; i <= 31; i++){
            pro.accumulator[i] = val[i];                }
    }else{
        for(int row = 0; row <= 31; row++){
            val[row] = pro.memory[row][address];
        }
        loadNeg(val);
        for(int i = 0; i <= 31; i++){
            pro.accumulator[i] = val[i];
        }
    }
}

void STO(Processor &pro){
    for(int i = 0; i <= 31; i++){
        pro.memory[i][address] = pro.accumulator[i];
    }
}

void LDP(Processor &pro){
    int val[32];
    if(IAF == 1){
        extendTo32(pro.PI, val);
        for(int i = 0; i <= 31; i++){
            pro.accumulator[i] = val[i];
        }
    }else{
        for(int row = 0; row <= 31; row++){
            val[row] = pro.memory[row][address];
        }
        for(int i = 0; i <= 31; i++){
            pro.accumulator[i] = val[i];
        }
    }
}

void SUB(Processor &pro){
    int val[32];
    if(IAF == 0){
        for(int row = 0; row <= 31; row++){
                val[row] = pro.memory[row][address];
        }
        int temp[32];
        substract(pro.accumulator,val,temp);
        for(int i = 0; i < 32; i++){
            pro.accumulator[i] = temp[i];
        }
    }else{
        extendTo32(pro.PI, val);
        int temp[32];
        substract(pro.accumulator,val,temp);
        for(int i = 0; i < 32; i++){
            pro.accumulator[i] = temp[i];
        }
    }
}

void CMP(Processor &pro){
    if(pro.accumulator[31] == 1){
        pro.CI++;
    }
}

void STP(Processor &pro){
    pro.CI = 64;
}

//function to extract the address from the instruction array
int getOperand(const int input[]){
    int output = 0;
    for(int i = 0; i < 12; i++){
        output += input[i] * powerOfTwo(i);
    }
    output -= input[12] * powerOfTwo(31);
    return output;
}

//function to extract the operation value from the instruction array
int getOpNum(const int input[]){
    int opNum = 0;
    for(int i = 0; i <= 31; i++){
        opNum += input[i] * powerOfTwo(i);
    }
    return opNum;
}

//function to extract the operation code from the instruction array
int getOpcode(const int input[]){
    int opCode = 0;
    for(int i = 13; i < 16; i++){
        opCode += input[i] * powerOfTwo(i - 13);
    }
    return opCode;
}
/*
void traverse(int arr[]){
    for(int i = 0; i < 16; i++){
        int temp =  arr[i];
        arr[i] = arr[31 - i];
        arr[31 - i] = temp;
    }
}*/

//function to sign-extend a 13 bit address to 32 bit
void extendTo32(int address[], int ins[]){
    int posOrNeg = address[31]; //the sign bit of the address
    for(int i = 0; i < 13; i++){
        ins[i] = address[i];    //copy the 13 bits
    }
    for(int i = 13; i < 32; i++){
        ins[i] = posOrNeg;      //fill the remaining bits with the sign bit
    }
}

//initialize the processor's memory and registers
void initialize(Processor &pro){
    //initialize memory to 0
    for(int column = 0; column < columnMax; column++){
        for(int row = 0; row < rowMax; row++){
            pro.memory[row][column] = 0;
        }
    }
    pro.CI = 0;
    for(int row = 0; row < rowMax; row++){
        pro.PI[row] = 0;
        pro.accumulator[row] = 0;
    }
}

//function to load machine code into the processor's memory
void readInMemory(Processor &pro){
    std::ifstream inFile("MachineCode.txt");

    assert(inFile);

    std::string line;
    int column = 0;
    while(getline(inFile,line) && column < columnMax){
        for(int row = 0; row <= 31 && row < line.size(); row++){
            pro.memory[row][column] = line[row] - '0';
        }
        column++;
    }
}

void fetch(Processor &pro){
    for(int row = 0; row <= 31; row++){
        pro.PI[row] = pro.memory[row][pro.CI];
    }
}

void decode(const Processor &pro){
    address = getOperand(pro.PI);
    opCode = getOpcode(pro.PI);
    opNum = getOpNum(pro.PI);
    IAF = pro.PI[16];
}

void execute(Processor &pro){
    int val[32] = {0};
    switch(opCode){
        case 0:
            JMP(pro);
            break;
        case 1:
            JRP(pro);
            break;
        case 2:
            LDN(pro);
            break;
        case 3:
            STO(pro);
            break;
        case 4:
            LDP(pro);
            break;
        case 5:
            SUB(pro);
            break;
        case 6:
            CMP(pro);
            break;
        case 7:
            STP(pro);
            break;
    }
}

void printOut(Processor &pro){
    std::cout << "STEP: " << step << std::endl;
    std::cout << "Target Address: " << address << "\tOp Code: " <<  opCode << " IAF: " << IAF << endl;
switch(opCode) {
    case 0:
        std::cout << "[Instruction: JMP] Jump to address: " << address << std::endl;
        break;
    case 1:
        std::cout << "[Instruction: JRP] Jump to the next " << address << " line of address" << std::endl;
        break;
    case 2:
        std::cout << "[Instruction: LDN] Negatively load value from";
        if (IAF == 1) {
            std::cout << "immediate value: " << address << std::endl;
        } else {
            std::cout << "memory address: " << address << std::endl;
        }
        break;
    case 3:
        std::cout << "[Instruction: STO] Store accumulator content into memory address " << address << std::endl;
        break;
    case 4:
        std::cout << "[Instruction: LDP] Positively load value from";
        if (IAF == 1) {
            std::cout << "immediate value: " << address << std::endl;
        } else {
            std::cout << "memory address: " << address << std::endl;
        }
    case 5:
        std::cout << "[Instruction: SUB] Subtract value at memory address: " << address 
                  << " from the accumulator" << std::endl;
        break;
    case 6:
        std::cout << "[Instruction: CMP] Compare accumulator; skipping next instruction if accumulator < 0" << std::endl;
        break;
    case 7:
        std::cout << "[Instruction: STP] Stop execution" << std::endl;
        break;
    default:
        std::cout << "Empty Operator Code: " << opCode << std::endl;
        break;
}
    std::cout << "CI: " << pro.CI << "  PI: ";
    for(int i = 0; i <= 31; i++){
        std::cout << pro.PI[i];
    }
    std::cout << std::endl;
    std::cout << "Accumulator(after caculation)(Twos Complement): ";
    for(int i = 0; i <= 31; i++){
        std::cout << pro.accumulator[i];
    }
    std::cout << std::endl;
    std::cout <<  "Decimal: " << TC2Dec(pro.accumulator) <<std::endl;
    std::cout << std::endl;
}

void displayMemory(const Processor &pro){
    bool equalTo7 = false;
    for(int column = 0; column < columnMax; column++){
        if(!equalTo7){
            for(int row = 0; row < rowMax; row++){
                if(row == 0){
                    std::cout << "Column: " << column << "\tAddress: ";
                }else if(row == 13){
                    std::cout << " Op Code: ";
                }else if(row == 16){
                    std::cout << " IAF: ";
                }else if(row == 17){
                    std::cout << " ";
                }
            std::cout << pro.memory[row][column];
            }
        }else{
            for(int row = 0; row < rowMax; row++){
                if(row == 0){
                    std::cout << "Column: " << column << "\tValue: ";
                }
                std::cout << pro.memory[row][column];
            }
        }
        std::cout << std::endl;
        int opCode = 0;
        for(int i = 13; i < 16; i++){
            opCode += pro.memory[i][column] * powerOfTwo(i - 13);
        }
        if(opCode == 7){
            equalTo7 = true;
        }
    }
}

void finalOutput(Processor &pro){
    std::cout << "Final Output: " << TC2Dec(pro.accumulator) << endl;
}

int main(){
    Processor processor;
    initialize(processor);
    readInMemory(processor);
    displayMemory(processor);
    std::cout << "INITIAL STATUS: " << std::endl;
    printOut(processor);
    while(processor.CI < columnMax){
        if(opCode != 0 && opCode != 1){
            processor.CI++;
        }
        sleep(1);
        step++;
        fetch(processor);
        decode(processor);
        execute(processor);
        printOut(processor);
    }
    displayMemory(processor);
    finalOutput(processor);
}
