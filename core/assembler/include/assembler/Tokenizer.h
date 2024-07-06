#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "util/File.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <set>


// TODO create a macro that will generate the token spec
class Tokenizer {
    public:
        enum Type {
            UNKNOWN,

			LABEL,
            TEXT, WHITESPACE_SPACE, WHITESPACE_TAB, WHITESPACE_NEWLINE, WHITESPACE,
            COMMENT_SINGLE_LINE, COMMENT_MULTI_LINE, BACK_SLASH, FORWARD_SLASH,

            // PREPROCESSOR DIRECTIVES
            PREPROCESSOR_INCLUDE,
            PREPROCESSOR_MACRO, PREPROCESSOR_MACRET, PREPROCESSOR_MACEND, PREPROCESSOR_INVOKE,
            PREPROCESSOR_DEFINE, PREPROCESSOR_UNDEF,
            PREPROCESSOR_IFDEF, PREPROCESSOR_IFNDEF,
            PREPROCESSOR_IFEQU, PREPROCESSOR_IFNEQU, PREPROCESSOR_IFLESS, PREPROCESSOR_IFMORE,
            PREPROCESSOR_ELSE, PREPROCESSOR_ELSEDEF, PREPROCESSOR_ELSENDEF,
            PREPROCESSOR_ELSEEQU, PREPROCESSOR_ELSENEQU, PREPROCESSOR_ELSELESS, PREPROCESSOR_ELSEMORE,
            PREPROCESSOR_ENDIF,

            // VARIABLE TYPES
            VARIABLE_TYPE_BYTE, VARIABLE_TYPE_DBYTE, VARIABLE_TYPE_WORD, VARIABLE_TYPE_DWORD,
            VARIABLE_TYPE_CHAR, VARIABLE_TYPE_STRING, VARIABLE_TYPE_FLOAT, VARIABLE_TYPE_DOUBLE,
            VARIABLE_TYPE_BOOLEAN,

            // ASSEMBLER DIRECTIVES
            ASSEMBLER_GLOBAL, ASSEMBLER_EXTERN,
            ASSEMBLER_EQU,
            ASSEMBLER_ORG,
            ASSEMBLER_SCOPE, ASSEMBLER_SCEND,
            ASSEMBLER_DB_LOW_ENDIAN,
            ASSEMBLER_DDB_LOW_ENDIAN, ASSEMBLER_DDB_HIGH_ENDIAN,
            ASSEMBLER_DW_LOW_ENDIAN, ASSEMBLER_DW_HIGH_ENDIAN,
            ASSEMBLER_DDW_LOW_ENDIAN, ASSEMBLER_DDW_HIGH_ENDIAN,
            ASSEMBLER_ASCII, ASSEMBLER_ASCIZ,
            ASSEMBLER_ADVANCE, ASSEMBLER_FILL, ASSEMBLER_SPACE,
            ASSEMBLER_CHECKPC,
            ASSEMBLER_ALIGN,
			ASSEMBLER_SECTION,
            ASSEMBLER_BSS, ASSEMBLER_BSS_ABSOLUTE,
            ASSEMBLER_DATA, ASSEMBLER_DATA_ABSOLUTE,
            ASSEMBLER_TEXT, ASSEMBLER_TEXT_ABSOLUTE,
            ASSEMBLER_STOP,

			// relocation
			RELOCATION_EMU32_O_LO12, RELOCATION_EMU32_ADRP_HI20,
			RELOCATION_EMU32_MOV_LO19, RELOCATION_EMU32_MOV_HI13,

			// registers
			REGISTER_X0, REGISTER_X1,
			REGISTER_X2, REGISTER_X3,
			REGISTER_X4, REGISTER_X5,
			REGISTER_X6, REGISTER_X7,
			REGISTER_X8, REGISTER_X9,
			REGISTER_X10, REGISTER_X11,
			REGISTER_X12, REGISTER_X13,
			REGISTER_X14, REGISTER_X15,
			REGISTER_X16, REGISTER_X17,
			REGISTER_X18, REGISTER_X19,
			REGISTER_X20, REGISTER_X21,
			REGISTER_X22, REGISTER_X23,
			REGISTER_X24, REGISTER_X25,
			REGISTER_X26, REGISTER_X27,
			REGISTER_X28, REGISTER_X29,
			REGISTER_SP, REGISTER_XZR,

			// instructions
			INSTRUCTION_HLT,
			INSTRUCTION_ADD, INSTRUCTION_SUB, INSTRUCTION_RSB,
			INSTRUCTION_ADC, INSTRUCTION_SBC, INSTRUCTION_RSC,
			INSTRUCTION_MUL, INSTRUCTION_UMULL, INSTRUCTION_SMULL,
			INSTRUCTION_VABS_F32, INSTRUCTION_VNEG_F32, INSTRUCTION_VSQRT_F32,
			INSTRUCTION_VADD_F32, INSTRUCTION_VSUB_F32, INSTRUCTION_VDIV_F32,
			INSTRUCTION_VMUL_F32, INSTRUCTION_VCMP_F32, INSTRUCTION_VSEL_F32,
			INSTRUCTION_VCINT_U32_F32, INSTRUCTION_VCINT_S32_F32,
			INSTRUCTION_VCFLO_U32_F32, INSTRUCTION_VCFLO_S32_F32,
			INSTRUCTION_VMOV_F32,
			INSTRUCTION_AND, INSTRUCTION_ORR, INSTRUCTION_EOR, INSTRUCTION_BIC,
			INSTRUCTION_LSL, INSTRUCTION_LSR, INSTRUCTION_ASR, INSTRUCTION_ROR,
			INSTRUCTION_CMP, INSTRUCTION_CMN, INSTRUCTION_TST, INSTRUCTION_TEQ,
			INSTRUCTION_MOV, INSTRUCTION_MVN,
			INSTRUCTION_LDR, INSTRUCTION_STR, INSTRUCTION_SWP,
			INSTRUCTION_LDRB, INSTRUCTION_STRB, INSTRUCTION_SWPB,
			INSTRUCTION_LDRH, INSTRUCTION_STRH, INSTRUCTION_SWPH,
			INSTRUCTION_B, INSTRUCTION_BL, INSTRUCTION_BX, INSTRUCTION_BLX, INSTRUCTION_SWI,

			// conditions for branch instructions
			CONDITION_EQ, CONDITION_NE,
			CONDITION_CS, CONDITION_HS,
			CONDITION_CC, CONDITION_LO,
			CONDITION_MI, CONDITION_PL,
			CONDITION_VS, CONDITION_VC,
			CONDITION_HI, CONDITION_LS,
			CONDITION_GE, CONDITION_LT, CONDITION_GT, CONDITION_LE,
			CONDITION_AL, CONDITION_NV,

			// expressions
            NUMBER_SIGN,
			LITERAL_FLOAT_32,
            LITERAL_NUMBER_BINARY, LITERAL_NUMBER_OCTAL, LITERAL_NUMBER_DECIMAL, LITERAL_NUMBER_HEXADECIMAL,
            LITERAL_CHAR, LITERAL_STRING,

            SYMBOL,
            COLON, COMMA, PERIOD, SEMICOLON,
            OPEN_PARANTHESIS, CLOSE_PARANTHESIS, OPEN_BRACKET, CLOSE_BRACKET, OPEN_BRACE, CLOSE_BRACE,

            OPERATOR_ADDITION, OPERATOR_SUBTRACTION, OPERATOR_MULTIPLICATION, OPERATOR_DIVISION, OPERATOR_MODULUS,
            OPERATOR_BITWISE_LEFT_SHIFT, OPERATOR_BITWISE_RIGHT_SHIFT,
            OPERATOR_BITWISE_XOR, OPERATOR_BITWISE_AND, OPERATOR_BITWISE_OR, OPERATOR_BITWISE_COMPLEMENT,
            OPERATOR_LOGICAL_NOT, OPERATOR_LOGICAL_EQUAL, OPERATOR_LOGICAL_NOT_EQUAL,
            OPERATOR_LOGICAL_LESS_THAN, OPERATOR_LOGICAL_GREATER_THAN,
            OPERATOR_LOGICAL_LESS_THAN_OR_EQUAL, OPERATOR_LOGICAL_GREATER_THAN_OR_EQUAL,
            OPERATOR_LOGICAL_OR, OPERATOR_LOGICAL_AND,
        };

        static const std::unordered_map<Type, std::string> TYPE_TO_NAME_MAP;

		static const std::unordered_map<Type,std::string> VARIABLE_TYPE_TO_NAME_MAP;

        static const std::set<Type> WHITESPACES;

        static const std::set<Type> COMMENTS;

        static const std::set<Type> PREPROCESSOR_DIRECTIVES;

        static const std::set<Type> VARIABLE_TYPES;

        static const std::set<Type> ASSEMBLER_DIRECTIVES;

		static const std::set<Type> RELOCATIONS;

		static const std::set<Type> REGISTERS;

		static const std::set<Type> INSTRUCTIONS;

		static const std::set<Type> CONDITIONS;

        static const std::set<Type> LITERAL_NUMBERS;

        static const std::set<Type> LITERAL_VALUES;

        static const std::set<Type> OPERATORS;

        static const std::vector<std::pair<std::string, Type>> TOKEN_SPEC;

        /**
		 * Base source code character set
		 *
		 * a-z A-Z 0-9 _ { } [ ] ( ) < > % : ; . , ? * + - / ^ & | ~ ! = " ' \ # @ $
		 */
		struct Token {

			Type type;
			std::string value;

			Token(Type type, std::string value) {
				this->type = type;
				this->value = value;
			}

			std::string to_string() {
				if (type == WHITESPACE_SPACE || type == WHITESPACE_TAB || type == WHITESPACE_NEWLINE) {
					std::string toString = TYPE_TO_NAME_MAP.at(type) + ":";
					for (auto i = 0; i < value.length(); i++) {
						toString += " " + std::to_string(value[i]);
					}
					return toString;
				} else if (type == COMMENT_SINGLE_LINE || type == COMMENT_MULTI_LINE) {
                    return TYPE_TO_NAME_MAP.at(type);
                }

				return TYPE_TO_NAME_MAP.at(type) + ": " + value;
			}
		};


        static std::vector<Token>& tokenize(File* srcFile);
		static std::vector<Token>& tokenize(std::string source_code);
};

#endif