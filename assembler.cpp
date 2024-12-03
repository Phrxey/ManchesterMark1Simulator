#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>

//###########################################################
#define MAX_MACHINE_CODE 32
#define MAX_LINES 64
#define MAX_LENGTH 256
#define MAX_SYMBOLS 64

// Opcode table
extern std::map<std::string, std::string> opcodeTable;

// Symbol table structure
struct Symbol {
    std::string name;
    int address;
    std::string type;
};

// Declaration of assembler function functions
void initOpcodeTable(); // Initialize the opcode table
int convertToInt(const std::string& str); //String to integer conversion 
std::string decToBinary(int num, int width); // Decimal to binary conversion


// Assembler-implemented function declarations 
std::vector<std::string> readAssemblyCode(); // Read the assembly code in the file, clearing comments and blank lines 
std::vector<Symbol> getSymbolTable(const std::vector<std::string>& lines); //Get the symbol table in assembly code and store it in vector
int assembleToMachineCode(const std::vector<Symbol>& symbolTable, const std::vector<std::string>& assemblyLines); //Print the machine code into a file

//###########################################################
using namespace std;

// the opcode mapping table
map<string, string> opcodeTable;
int assemblyLineCount = 0;
int symbolCount = 0;

// Initialize the opcode mapping table
void initOpcodeTable() {
    opcodeTable["JMP"] = "000";
    opcodeTable["VAR"] = "000"; 
    opcodeTable["JRP"] = "100";
    opcodeTable["LDN"] = "010";
    opcodeTable["STO"] = "110";
    opcodeTable["LDP"] = "001"; // Automatic loading of positive and negative values 
    opcodeTable["SUB"] = "101";
    opcodeTable["CMP"] = "011";
    opcodeTable["STP"] = "111";
}

//String to integer conversion
int convertToInt(const string& str) {
    stringstream ss(str); // Create a stream of strings
    int number = 0;
    ss >> number; // Convert strings to integers

    // Check if the conversion was successful
    if (ss.fail() || !ss.eof()) {
        throw invalid_argument("Invalid number format");
    }
    return number;
}

// Convert a decimal number to a binary array, completing the length with 0
string decToBinary(int num, int width) {
    string binary;

    if (num < 0) {
        // Convert negative numbers to complementary code
        num = -num;
        // Get the binary representation of the absolute value
        while (num > 0) {
            binary.insert(binary.begin(),(num % 2 == 0) ? '0' : '1');
            num /= 2;
        }
        
        // If the number of binary bits is less than the specified width, prefix it with '0'.
    	while (binary.length() < width) {
        	binary.insert(binary.begin(),'0');
    	}
    	
        // inverts the binary.
        for (int i = 0; i < binary.length(); ++i) {
            binary[i] = (binary[i] == '0') ? '1' : '0';
        }
        // add 1
        bool carry = true;
        for (int i = (binary.length()-1); i > 0; --i) {
            if (carry) {
                if (binary[i] == '0') {
                    binary[i] = '1';
                    carry = false;
                } else {
                    binary[i] = '0';
                }
            }
        }
    } else {
        // For positive numbers, convert directly to binary
        while (num > 0) {
            binary.insert(binary.begin(),(num % 2 == 0) ? '0' : '1');
            num /= 2;
        }
        // If the number of binary bits is less than the specified width, prefix it with '0'.
    	while (binary.length() < width) {
        	binary.insert(binary.begin(),'0');
    	}
    }

    // Since binary is constructed from low to high, invert the string
    reverse(binary.begin(), binary.end());

    return binary;
}

// Read assembly code and store in vector
vector<string> readAssemblyCode() {
    ifstream file("assemblyCode.txt");
    if (!file.is_open()) {
    	cerr << "Failed to open assembly code file" << endl; // Failed to open assembly code file
    	assemblyLineCount = -1; 
        return vector<string>();
    }

    vector<string> lines; // Store assembly code lines 
    string line;

    // Read the file line by line
    while (getline(file, line)) {
        // Ignore lines that start with ';' or blank lines
        if (line.empty() || line[0] == ';') {
            continue;
        }
        // Remove empty line
        size_t NotSpace = line.find_last_not_of(" \n\r\t");
        if (NotSpace == string::npos) {
            continue;
        }

        // find the position of the semicolon and truncate the string
        size_t semicolonPos = line.find(';');
        if (semicolonPos != string::npos) {
            line = line.substr(0, semicolonPos);
        } else {
        	cerr << "Error in line " << assemblyLineCount << ": " << line << endl;
        	cerr << "Mising a semicolon!" << endl;
        	assemblyLineCount = -1;
        	return vector<string>();
		}

        // Remove extra spaces or line breaks at the end of the line
        size_t lastNotOf = line.find_last_not_of(" \n\r\t");
        if (lastNotOf != string::npos) {
            line = line.substr(0, lastNotOf + 1);
        }

        // if row is not empty, add to vector
        if (!line.empty()) {
            lines.push_back(line);
            assemblyLineCount++; 
        }
    }
    
    file.close();

    // Assembly code without comments and blank lines
    cout << "Assembly code without comments and blank lines:" << endl;
    cout << endl;
    for (size_t i = 0; i < lines.size(); ++i) {
        cout << lines[i] << endl;
    }
    cout << "===============================================" << endl;

    return lines;
}

vector<Symbol> getSymbolTable(const vector<string>& lines) {
    int addressCounter = 0;
    vector<Symbol> symbolTable; // Create vectors to store symbol tables

    for (size_t i = 0; i < lines.size(); ++i) {
        string line = lines[i];  // Get the current line

        // Find out if there is a label in the current line
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string labelName = line.substr(0, colonPos);  // Capture symbal names

            // Create a symbol object and add it to the symbol table
            Symbol symbol;
            symbol.name = labelName;
            symbol.address = addressCounter;
            symbol.type = "Label"; // Default to label first and override later. 
            symbolTable.push_back(symbol);
            symbolCount++;
            
            // Skip the space after the colon
            size_t start = line.find_first_not_of(" ", colonPos + 1);
            if (start != string::npos) {
                // Update line, remove label section
                line = line.substr(start);
            }
        } else {
            addressCounter++;
            continue;
        }

        // If this line is declaring a variable
        
        if (line.substr(0, 3) == "VAR") {
            // Change the symbol type to ¡°Variable¡±.
            symbolTable[symbolTable.size() - 1].type = "Variable";
        }

        addressCounter++;
    }

	cout << "Symbol Table:" << endl;
	cout << endl;
    cout << "Name \t\t Addres \t Type\n";
    for (size_t i = 0; i < symbolTable.size(); ++i) {
        cout << symbolTable[i].name << " \t\t " << symbolTable[i].address << " \t\t " << symbolTable[i].type << std::endl;
    }
    cout << "===============================================" << endl;

    return symbolTable;
}

int assembleToMachineCode(const vector<Symbol>& symbolTable, const vector<string>& assemblyLines) {
	
	int operandNumber = 0;
	string opcode, operand;
	
    ofstream outFile("MachineCode.txt"); // Output to output.txt
    if (!outFile.is_open()) {
        cerr << "Error: Unable to open the output file!" << endl;
        return -1;
    }

    for (size_t i = 0; i < assemblyLines.size(); ++i) {
        string line = assemblyLines[i];
        size_t colonPos = line.find(':'); // Clear the symbol identifier from the front of the string
        if (colonPos != string::npos) {
            line = line.substr(colonPos+1);
        }
        size_t start = line.find_first_not_of(" \t"); // Clear spaces at the beginning of the string  
    	if (start == string::npos) {
        	continue; 
    	} else {
			line = line.substr(start) ;
		}
		
        size_t spacePos = line.find(' '); // Separate opcodes and operands
        if (spacePos != string::npos) {
            opcode = line.substr(0, spacePos);
            operand = line.substr(spacePos + 1);
        } else {
            opcode = line;
            operand = "";
        }

		if (opcode == "VAR") {
			try {
            	operandNumber = convertToInt(operand); // Convert strings to integers
        	} catch (const invalid_argument&) {
        		cerr << "Error in line " << i+1 << ": " << assemblyLines[i] << endl;
            	cerr << "Variable error:" << operand << endl;
           		return -1;
        	}
        	outFile << decToBinary(operandNumber,32) << endl;
        	continue;
		}

        // Get the value of the opcode
        if (opcodeTable.find(opcode) == opcodeTable.end()) {
            cerr << "Error: Unknown opcode:" << opcode << endl;
            return -1;
        }
        string opcodeBinary = opcodeTable[opcode];
        
        //Get the value of the operand
        int operandNumber = 0;
        bool immediateAddressing = false;
        // 1. If it starts with a '#', parse the number after it
    	if (operand[0] == '#') {
        	try {
            	operandNumber = convertToInt(operand.substr(1)); // Convert the part after '#' to an integer
            	immediateAddressing = true;
        	} catch (const invalid_argument&) {
        		cerr << "Error: No correct address after # :" << operand << endl;
           		return -1;
        	} 
    	} else if (isdigit(operand[0])){ // 2. if it's a pure number, return it directly
    		try {
            	operandNumber = convertToInt(operand); // Convert strings to integers
        	} catch (const invalid_argument&) {
            	cerr << "Error: invalid address:" << operand << endl;
           		return -1;
        	}
		} else if (!operand.empty()){
			// look up the address of the operand in the symbol table
            bool found = false;
            for (size_t j = 0; j < symbolTable.size(); ++j) {
		    	if (symbolTable[j].name == operand) {
		        	operandNumber = symbolTable[j].address;
		        	found = true;
		        	break;
		    	}
			}
            if (!found) {
                cerr << "Error: Unknown operand:" << operand << endl;
                return -1;
            }
		} 
		// If the address is too large, return it directly 
    	if (immediateAddressing == false && operandNumber > MAX_MACHINE_CODE) {
            cerr << operand << "oversize address" << endl;
           	return -1;
        }
        
        outFile << decToBinary(operandNumber,13);
        outFile << opcodeBinary;
        if (immediateAddressing){
        	outFile << '1' << decToBinary(0,15) << endl;
		} else {
			outFile << decToBinary(0,16) << endl;
		}
    }

    outFile.close(); 
    return 0;
}

int main() {

    initOpcodeTable();

    vector<string> assemblyLines = readAssemblyCode();
    if (assemblyLines.empty()) {
    	return -1;
	}
	
    vector<Symbol> symbolTable = getSymbolTable(assemblyLines);
    if(!assembleToMachineCode(symbolTable, assemblyLines)){
    	cout << "Assembly code successfully converted to machine code" << endl;
	}
    return 0;
}







