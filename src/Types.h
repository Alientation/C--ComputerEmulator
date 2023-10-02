#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <iomanip>
#include <bitset>

using Byte = unsigned char;

// Standard size of a unit of data
using Word = unsigned int; // 4 bytes

// int sizes
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long; 

// static constants
static const u8 BITS_IN_BYTE = 8;
static const u8 BITS_IN_WORD = 32;

static const char VALUE_PREFIX = '#';
static const char BINARY_PREFIX = '%'; // could also be 0b
static const char HEXADECIMAL_PREFIX = '$'; // could also be 0x

// help to print out hexadecimal numbers with zero padding
static std::string stringifyHex(u64 hex, int width = 16) {
    std::stringstream stream;
    stream << HEXADECIMAL_PREFIX << std::setfill('0') << std::setw(width) << std::hex << hex;
    std::string result(stream.str());
    return result;
}

static std::string stringifyHex(u32 hex) {
    return stringifyHex(hex,8);
}

static std::string stringifyHex(u16 hex) {
    return stringifyHex(hex,4);
}

static std::string stringifyHex(u8 hex) {
    return stringifyHex(hex,2);
}


// help to print out binary numbers with zero padding
static std::string stringifyBin(u64 bin) {
    std::stringstream stream;
    stream << BINARY_PREFIX << std::bitset<64>(bin);
    std::string result(stream.str());
    return result;
}

static std::string stringifyBin(u32 bin) {
    std::stringstream stream;
    stream << BINARY_PREFIX << std::bitset<32>(bin);
    std::string result(stream.str());
    return result;
}

static std::string stringifyBin(u16 bin) {
    std::stringstream stream;
    stream << BINARY_PREFIX << std::bitset<16>(bin);
    std::string result(stream.str());
    return result;
}

static std::string stringifyBin(u8 bin) {
    std::stringstream stream;
    stream << BINARY_PREFIX << std::bitset<8>(bin);
    std::string result(stream.str());
    return result;
}

#endif // TYPES_H