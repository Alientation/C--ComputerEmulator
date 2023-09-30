#include "AlienCPU6502.h"
#include <iostream>
#include <iomanip>

const std::string AlienCPU6502::VERSION = "0.0.1";

AlienCPU6502::AlienCPU6502() {
    // null out instructions
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        instructions[i] = _0000NullInstruction;
    }

    instructions[INS_LDA_IM] = _00A9_LoadAccumulator_Immediate;
}

void AlienCPU6502::Reset() {
    // reset all registers
    pc = PC_INIT;
    sp = SP_INIT;

    a = A_INIT;
    x = X_INIT;
    y = Y_INIT;

    // reset flags
    p = P_INIT;

    // reset cycle counters
    nextInterruptCheck = INTERRUPT_CHECK_INTERVAL;
    cycles = 0;

    // prepare ram
    ram.Initialize();
}

void AlienCPU6502::Start(u64 maxCycles) {
    // Reset the CPU, all registers, ram etc
    Reset();

    std::cout << "Starting Alien CPU v" << VERSION << std::endl;
    
    // Fetch, Decode, Execute Cycle loop
    for (;;) {
        // Halt execution because max cycles has been reached
        if (cycles >= maxCycles) {
            std::cout << std::endl << "Max cycles reached" << std::endl;
            break;
        }

        std::cout << ".";

        // Reads in the next instruction of variable length (1-2 bytes for this cpu)
        u16 nextInstruction = 0x0000;
        Byte nextInstructionByte = 0;
        u8 instructionBytesLength = 0;
        
        // while the length of the instruction in bytes is less than the max instruction bytes length
        // stop when the current read instruction is valid
        while (instructionBytesLength < MAX_INSTRUCTION_BYTES_LENGTH && !ValidInstruction(nextInstruction)) {
            nextInstructionByte = FetchNextByte(); // get the next byte in memory

            nextInstruction = nextInstruction << 8; // shift the current instruction 8 bits to the left
            nextInstruction = nextInstruction | nextInstructionByte; // add the read instruction byte to the current instruction
            instructionBytesLength++; //increment byte count
        }

        // Executes the instruction even if it is invalid
        // ExecuteInstruction(nextInstruction);


        // Check for Interrupts
        if (nextInterruptCheck == 0) {

        }
    }

    std::cout << "Stopping Alien CPU v" << VERSION << std::endl;
}

// Executes the instruction if it is valid, otherwise throws an exception
void AlienCPU6502::ExecuteInstruction(u16 instruction) {
    if (!ValidInstruction(instruction)) {
        std::stringstream stream;
        stream << "Error: Invalid instruction 0x" << std::hex << instruction << std::endl;
        
        throw std::invalid_argument(stream.str());
    }

    instructions[instruction](*this); // calls the function associated with the instruction
}

// Checks if the instruction is a valid instruction. Must be within max instructions and must not be a null instruction
bool AlienCPU6502::ValidInstruction(u16 instruction) {
    return instruction < INSTRUCTION_COUNT && &instructions[instruction] != &instructions[0];
}

// Gets the next byte in memory and increments PC and cycles
Byte AlienCPU6502::FetchNextByte() {
    Byte data = ReadByte(pc); // gets byte at the program pointer (PC)
    pc++;
    cycles++;
    return data;
}

// Gets the next 4 bytes in memory
Word AlienCPU6502::FetchNextWord() {
    Word data = FetchNextByte(); // byte 1

    data = (data << 8) | FetchNextByte(); // byte 2
    data = (data << 8) | FetchNextByte(); // byte 3
    data = (data << 8) | FetchNextByte(); // byte 4

    return data;
}

// Reads the byte at the specified address in memory if valid, otherwise throws an exception
Byte AlienCPU6502::ReadByte(Word address) {
    if (address >= ram.MEMORY_SIZE) {
        // display the hex address of the out of bounds memory access
        std::stringstream stream;
        stream << "Error: Out of bounds memory access at address 0x" << std::hex << address <<
             ". Requires 1 byte" << std::endl;
        
        throw std::invalid_argument(stream.str());
    }
    return ram.Data[address];
}

// Reads the next 4 bytes at the specified address in memory if valid, otherwise throws an exception
Word AlienCPU6502::ReadWord(Word address) {
    if (address >= ram.MEMORY_SIZE - 4) {
        // display the hex address of the out of bounds memory access
        std::stringstream stream;
        stream << "Error: Out of bounds memory access at address 0x" << std::hex << address << 
            ". Requires 4 bytes." << std::endl;
        
        throw std::invalid_argument(stream.str());
    }

    Word data = ReadByte(address); // byte 1

    data = (data << 8) | ReadByte(address + 1); // byte 2
    data = (data << 8) | ReadByte(address + 2); // byte 3
    data = (data << 8) | ReadByte(address + 3); // byte 4

    return data;
}

// Write the byte to the specified address in memory if valid, otherwise throws an exception
void AlienCPU6502::WriteByte(Word address, Byte value) {
    if (address >= ram.MEMORY_SIZE) {
        // display the hex address of the out of bounds memory access
        std::stringstream stream;
        stream << "Error: Out of bounds memory access at address 0x" << std::hex << address <<
             ". Requires 1 byte" << std::endl;
        
        throw std::invalid_argument(stream.str());
    }

    // write byte 1 to ram
    ram.Data[address] = value;
}

// Write the next 4 bytes to the specified address in memory if valid, otherwise throws an exception
void AlienCPU6502::WriteWord(Word address, Word value) {
    if (address >= ram.MEMORY_SIZE - 4) {
        // display the hex address of the out of bounds memory access
        std::stringstream stream;
        stream << "Error: Out of bounds memory access at address 0x" << std::hex << address << 
            ". Requires 4 bytes." << std::endl;
        
        throw std::invalid_argument(stream.str());
    }

    // write bytes 1 to 4 to ram
    WriteByte(address, (value >> 24) & 0xFF);
    WriteByte(address + 1, (value >> 16) & 0xFF);
    WriteByte(address + 2, (value >> 8) & 0xFF);
    WriteByte(address + 3, value & 0xFF);
}


// =======================STACK============================
//
//

//
void AlienCPU6502::SPtoAddress(Byte page) {

}

//
void AlienCPU6502::PushWordToStack(Word value) {

}

//
Word AlienCPU6502::PopWordFromStack() {
    return NULL_ADDRESS;
}

//
void AlienCPU6502::PushByteToStack(Byte value) {

}

//
Byte AlienCPU6502::PopByteFromStack() {
    return NULL_ADDRESS >> 24;
}



// ====================INSTRUCTIONS======================
//
//

// Null Instruction, throws error if called
void AlienCPU6502::_0000NullInstruction() {
    std::stringstream stream;
    stream << "Error: Null Instruction" << std::endl;
    
    throw std::invalid_argument(stream.str());
}

// Load Accumulator Immediate Instruction (LDA_IM) into register A
// Loads the next byte into register A
void AlienCPU6502::_00A9_LoadAccumulator_Immediate() {
    
}