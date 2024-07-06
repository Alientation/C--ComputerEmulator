#include "assembler/Assembler.h"
#include "util/Logger.h"

#include <string>

/**
 * @brief 					Adds a symbol to the symbol table
 *
 * @param symbol 			symbol string
 * @param value 			value of the symbol if it is defined
 * @param binding_info 		visiblity of the symbol
 * @param section 			section it is defined in. -1 if not defined in a section
 */
void Assembler::add_symbol(std::string symbol, word value, SymbolTableEntry::BindingInfo binding_info, int section) {
	if (string_table.find(symbol) == string_table.end()) {			/* If symbol does not exist yet, create it */
		string_table[symbol] = strings.size();
		strings.push_back(symbol);
		symbol_table[string_table[symbol]] = (SymbolTableEntry) {
			.symbol_name = string_table[symbol],
			.symbol_value = value,
			.binding_info = binding_info,
			.section = section,
		};
	} else {
		SymbolTableEntry &symbol_entry = symbol_table[string_table[symbol]];
		if (symbol_entry.section == -1 && section != -1) {
			symbol_entry.section = section;
			symbol_entry.symbol_value = value;
		} else if (symbol_entry.section != -1 && section != -1) {
			lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::add_symbol() - Multiple definition of symbol "
					<< symbol << " at sections " << strings[section_table[section].section_name] << " and "
					<< strings[section_table[symbol_entry.section].section_name] << ".");
			m_state = State::ASSEMBLER_ERROR;
			return;
		} else if (binding_info == SymbolTableEntry::BindingInfo::GLOBAL
				|| (binding_info == SymbolTableEntry::BindingInfo::LOCAL &&
				symbol_entry.binding_info == SymbolTableEntry::BindingInfo::WEAK)) {
			symbol_entry.binding_info = binding_info;
		}
	}
}

/**
 * @brief
 * @todo					Implement full expression parser
 *
 * @param 					tokenI: Reference to current token index
 * @return 					value of expression
 */
word Assembler::parse_expression(int& tokenI) {
	/* For now, only parse expressions sequentially, without care of precedence */
	word exp_value = 0;
	skipTokens(tokenI, "[ \t]");
	Tokenizer::Token *operator_token = nullptr;
	do {
		Tokenizer::Token token = consume(tokenI);

		word value = 0;
		if (token.type == Tokenizer::LITERAL_NUMBER_DECIMAL) {
			value = std::stoi(token.value);
		} else if (token.type == Tokenizer::LITERAL_NUMBER_HEXADECIMAL) {
			value = std::stoi(token.value, nullptr, 16);
		} else if (token.type == Tokenizer::LITERAL_NUMBER_BINARY) {
			value = std::stoi(token.value, nullptr, 2);
		} else if (token.type == Tokenizer::LITERAL_NUMBER_OCTAL) {
			value = std::stoi(token.value, nullptr, 8);
		}

		if (operator_token != nullptr) {
			switch(operator_token->type) {
				case Tokenizer::OPERATOR_ADDITION:
					exp_value += value;
					break;
				case Tokenizer::OPERATOR_SUBTRACTION:
					exp_value -= value;
					break;
				case Tokenizer::OPERATOR_DIVISION:
					exp_value /= value;
					break;
				case Tokenizer::OPERATOR_MULTIPLICATION:
					exp_value *= value;
					break;
			}
			operator_token = nullptr;
		}
		skipTokens(tokenI, "[ \t]");

		/* Temporary only support 4 operations */
		if (isToken(tokenI, {Tokenizer::OPERATOR_ADDITION, Tokenizer::OPERATOR_DIVISION, Tokenizer::OPERATOR_MULTIPLICATION, Tokenizer::OPERATOR_SUBTRACTION})) {
			operator_token = &consume(tokenI);
		} else {
			break;
		}
	} while(isToken(tokenI, {Tokenizer::WHITESPACE_NEWLINE}));

	return exp_value;
}

/**
 * @brief 					Declares a symbol to be global outside this compilation unit. Must be declared outside any defined sections like .text, .bss, and .data.
 * USAGE:					.global <symbol>
 *
 * @param 					tokenI: reference to current token index
 */
void Assembler::_global(int& tokenI) {
	if (current_section == Section::NONE) {
		lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_global() - Cannot declare symbol as global inside a section. Must be declared outside of .text, .bss, and .data.");
		m_state = State::ASSEMBLER_ERROR;
		return;
	}

	consume(tokenI);
	skipTokens(tokenI, Tokenizer::WHITESPACES);

	std::string symbol = consume(tokenI).value;
	add_symbol(symbol, 0, SymbolTableEntry::BindingInfo::GLOBAL, -1);
}

/**
 * @brief					Declares a symbol to exist in another compilation unit but not defined here. Symbol's binding info will be marked as weak.
 * 								Must be declared outside any defined sections like .text, .bss, and .data.
 * USAGE:					.extern <symbol>
 *
 * @param 					tokenI: reference to current token index
 */
void Assembler::_extern(int& tokenI) {
	if (current_section == Section::NONE) {
		lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_extern() - Cannot declare symbol as extern inside a section. Must be declared outside of .text, .bss, and .data.");
		m_state = State::ASSEMBLER_ERROR;
		return;
	}

	consume(tokenI);
	skipTokens(tokenI, Tokenizer::WHITESPACES);

	std::string symbol = consume(tokenI).value;
	add_symbol(symbol, 0, SymbolTableEntry::BindingInfo::WEAK, -1);
}

/**
 * @brief 					Moves where the assembler is in a section. Can only move forward, not backward.
 * USAGE:					.org <expression>
 *
 * @param 					tokenI: reference to current token index
 */
void Assembler::_org(int& tokenI) {
	if (current_section == Section::NONE) {
		lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_org() - Not defined inside section. Cannot move section pointer.");
		m_state = State::ASSEMBLER_ERROR;
		return;
	}

	consume(tokenI);
	skipTokens(tokenI, Tokenizer::WHITESPACES);

	word val = parse_expression(tokenI);

	if (val >= 0xffffff) {											/*! Safety exit. Likely unintentional behavior */
		lgr::log(lgr::Logger::LogType::WARN, std::stringstream() << "Assembler::_org() - new value is large and likely unintentional. (" << std::to_string(val) << ")");
		m_state = State::ASSEMBLER_WARNING;
		return;
	}

	switch (current_section) {
		case Section::BSS:
			if (val < bss_section) {
				lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_org() - .org directive cannot move assembler pc backwards. Expected >= "
						<< std::to_string(bss_section) << ". Got " << val << ".");
				m_state = State::ASSEMBLER_ERROR;
				return;
			}
			bss_section = val;
			break;
		case Section::DATA:
			if (val < data_section.size()) {
				lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_org() - .org directive cannot move assembler pc backwards. Expected >= "
						<< std::to_string(data_section.size()) << ". Got " << val << ".");
				m_state = State::ASSEMBLER_ERROR;
				return;
			}
			for (int i = data_section.size(); i < val; i++) {
				data_section.push_back(0);
			}
			break;
		case Section::TEXT:											/* It is likely not very useful to allow .org to move pc in a text section, comparatively to .data and .bss */
			if (val < text_section.size() * 4) {
				lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_org() - .org directive cannot move assembler pc backwards. Expected >= "
						<< std::to_string(text_section.size() * 4) << ". Got " << val << ".");
				m_state = State::ASSEMBLER_ERROR;
				return;
			}

			if (val % 4 != 0) {
				lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_org() - .org directive cannot move assembler pc to a non-word aligned byte in .text section. Expected aligned 4 byte."
						<< " Got " << val << ".");
				m_state = State::ASSEMBLER_ERROR;
				return;
			}

			for (int i = text_section.size() * 4; i < val; i += 4) {
				text_section.push_back(0);
			}
			break;
	}
}

/**
 * @brief 					Defines a local scope. Any symbol defined inside will be marked as local and will not be able to be marked as global.
 * 								Symbols defined here will be postfixed with a special identifier <symbol>:<scope_token_index>
 * USAGE:					.scope
 *
 * @param 					tokenI: Reference to current token index
 */
void Assembler::_scope(int& tokenI) {
	scope_token_indices.push_back(tokenI);
	consume(tokenI);
}

/**
 * @brief 					Ends a local scope.
 * USAGE:					.scend
 *
 * @param 					tokenI: Reference to current token index
 */
void Assembler::_scend(int& tokenI) {
	if (scope_token_indices.empty()) {
		lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_scend() - .scend directive must have a matching .scope directive.");
		m_state = State::ASSEMBLER_ERROR;
		return;
	}

	scope_token_indices.pop_back();
	consume(tokenI);
}

/**
 * @brief 					Moves where the assembler is in a section forward by a certain amount of bytes.
 * USAGE:					.advance <expression>
 *
 * @param 					tokenI: Reference to current token index
 */
void Assembler::_advance(int& tokenI) {
	if (current_section == Section::NONE) {
		lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_advance() - Not defined inside section. Cannot move section pointer.");
		m_state = State::ASSEMBLER_ERROR;
		return;
	}

	consume(tokenI);
	skipTokens(tokenI, Tokenizer::WHITESPACES);

	word val = parse_expression(tokenI);

	if (val >= 0xffffff) {											/*! Safety exit. Likely unintentional behavior */
		lgr::log(lgr::Logger::LogType::WARN, std::stringstream() << "Assembler::_advance() - offset value is large and likely unintentional. (" << std::to_string(val) << ")");
		m_state = State::ASSEMBLER_WARNING;
		return;
	}

	switch (current_section) {
		case Section::BSS:
			bss_section += val;
			break;
		case Section::DATA:
			for (int i = 0; i < val; i++) {
				data_section.push_back(0);
			}
			break;
		case Section::TEXT:											/* It is likely not very useful to allow .org to move pc in a text section, comparatively to .data and .bss */
			if (val % 4 != 0) {
				lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_advance() - .advance directive cannot move assembler pc to a non-word aligned byte in .text section. Expected aligned 4 byte."
						<< " Got " << val << ".");
				m_state = State::ASSEMBLER_ERROR;
				return;
			}

			for (int i = 0; i < val; i += 4) {
				text_section.push_back(0);
			}
			break;
	}
}

/**
 * @brief 					Aligns where the assembler is in the current section.
 * @note					This is useless unless we can specify in the program header of the object file the alignment of the whole program
 * USAGE:					.align <expression>
 *
 * @param 					tokenI: Reference to the current token index
 */
void Assembler::_align(int& tokenI) {
	if (current_section == Section::NONE) {
		lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_align() - Not defined inside a section. Cannot align section pointer.");
		m_state = State::ASSEMBLER_ERROR;
		return;
	}

	consume(tokenI);
	skipTokens(tokenI, Tokenizer::WHITESPACES);

	word val = parse_expression(tokenI);
	if (val >= 0xffff) {											/*! Safety exit. Likely unintentional behavior */
		lgr::log(lgr::Logger::LogType::WARN, std::stringstream() << "Assembler::_align() - Alignment value is large and likely unintentional. (" << std::to_string(val) << ")");
		m_state = State::ASSEMBLER_WARNING;
		return;
	}

	switch (current_section) {
		case Section::BSS:
			bss_section += (val - (bss_section%val)) % val;
			break;
		case Section::DATA:
			while (data_section.size() % val != 0) {
				data_section.push_back(0);
			}
			break;
		case Section::TEXT:											/*! It is likely not very useful to allow .org to move pc in a text section, comparatively to .data and .bss */
			if (val % 4 != 0) {
				lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_advance() - .advance directive cannot move assembler pc to a non-word aligned byte in .text section. Expected aligned 4 byte."
						<< " Got " << val << ".");
				m_state = State::ASSEMBLER_ERROR;
				return;
			}

			while (data_section.size() * 4 % val != 0) {
				text_section.push_back(0);
			}
			break;
	}
}

/**
 * @brief 					Creates a new section.
 * @warning					Not implemented yet.
 * USAGE:					.section <string>, <flags>
 *
 * @param 					tokenI: Reference to the current token index
 */
void Assembler::_section(int& tokenI) {
	consume(tokenI);
	skipTokens(tokenI, Tokenizer::WHITESPACES);

	expectToken(tokenI, {Tokenizer::LITERAL_STRING}, "Assembler::_section() - .section expects a string argument to follow.");

	lgr::log(lgr::Logger::LogType::ERROR, std::stringstream() << "Assembler::_section() - .section directive is not implemented yet.");
	m_state = State::ASSEMBLER_ERROR;
	return;
}

/**
 * @brief					Creates a new text section.
 * @warning					Currently will simply add on to the previously defined text section if it exists.
 * USAGE:					.text
 *
 * @param 					tokenI: Reference to the current token index
 */
void Assembler::_text(int& tokenI) {
	consume(tokenI);

	current_section = Section::TEXT;
	current_section_index = 0;
}

/**
 * @brief					Creates a new data section.
 * @warning					Currently will simply add on to the previously defined data section if it exists
 * USAGE:					.data
 *
 * @param 					tokenI: Reference to the current token index
 */
void Assembler::_data(int& tokenI) {
	consume(tokenI);

	current_section = Section::DATA;
	current_section_index = 1;
}

/**
 * @brief					Creates a new bss section.
 * @warning					Currently will simply add on to the previously defined bss section if it exists
 * USAGE:					.bss
 *
 * @param 					tokenI: Reference to the current token index
 */
void Assembler::_bss(int& tokenI) {
	consume(tokenI);

	current_section = Section::BSS;
	current_section_index = 2;
}

/**
 * @brief 					Stops assembling
 * USAGE:					.stop
 *
 * @param 					tokenI: Reference to the current token index
 */
void Assembler::_stop(int& tokenI) {
	tokenI = m_tokens.size();
}
