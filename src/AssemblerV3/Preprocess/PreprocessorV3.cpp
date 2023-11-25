#include "PreprocessorV3.h"

#include "Logger.h"
#include <regex>


/**
 * Constructs a preprocessor object with the given file.
 * 
 * @param file the file to preprocess
 */
Preprocessor::Preprocessor(Process* process, File* file) {
	this->process = process;
	this->inputFile = file;
	this->outputFile = new File(file->getFileName(), PROCESSED_EXTENSION, file->getFileDirectory());

	if (!process->isValidSourceFile(inputFile)) {
		log(ERROR, std::stringstream() << "Preprocessor::Preprocessor() - Invalid source file: " << inputFile->getExtension());
		return;
	}

	state = State::UNPROCESSED;
	writer = new FileWriter(outputFile);

	tokenize();
}

/**
 * Tokenizes the input file. This is an internal function.
 */
void Preprocessor::tokenize() {
	log(DEBUG, std::stringstream() << "Preprocessor::Preprocessor() - Tokenizing file: " << inputFile->getFileName());

	if (state != State::UNPROCESSED) {
		log(ERROR, std::stringstream() << "Preprocessor::Preprocessor() - Preprocessor is not in the UNPROCESSED state");
		return;
	}

	FileReader* reader = new FileReader(inputFile);

	// tokenizes the input file
	std::string tok = "";
	bool isQuoted = false;
	char quotedChar = '"';

	// read the file
	while (reader->hasNextByte()) {
		char byte = reader->readByte();

		// handle quotes
		if (!isQuoted && (byte == '"' || byte == '<')) {
			isQuoted = true;
			quotedChar = byte;
		} else if (isQuoted && byte == quotedChar) {
			isQuoted = false;
		}

		if (std::isspace(byte) && !isQuoted) {
			// whitespace character not surrounded by quotes
			tokens.push_back(Token(Token::Type::TEXT, tok));
			tokens.push_back(Token(Token::Type::WHITESPACE, std::string(1, byte)));
			tok = "";
		} else if (!reader->hasNextByte()) {
			tok += byte;
			tokens.push_back(Token(Token::Type::TEXT, tok));
		} else {
			tok += byte;
		}
	}

	// unclosed quotes
	if (isQuoted) {
		log(ERROR, std::stringstream() << "Preprocessor::Preprocessor() - Unclosed quotes: " << quotedChar);
		return;
	}

	// print out tokens
	log(DEBUG, std::stringstream() << "Preprocessor::Preprocessor() - Tokenized file: " << inputFile->getFileName());
	for (int i = 0; i < tokens.size(); i++) {
		Token& token = tokens[i];

		if (token.type == token.WHITESPACE) {
			log(DEBUG, std::stringstream() << "Preprocessor::Preprocessor() - Token[" << i << "]: " << std::to_string(token.value[0]) << " (" << token.type << ")");
		} else {
			log(DEBUG, std::stringstream() << "Preprocessor::Preprocessor() - Token[" << i << "]: " << token.value << " (" << token.type << ")");
		}
	}

	reader->close();
}


/**
 * Destructs a preprocessor object
 */
Preprocessor::~Preprocessor() {
	inputFile->~File();
	outputFile->~File();
	writer->~FileWriter();
}

/**
 * Preprocesses the file
 */
void Preprocessor::preprocess() {
	log(DEBUG, std::stringstream() << "Preprocessor::preprocess() - Preprocessing file: " << inputFile->getFileName());

	if (state != State::UNPROCESSED) {
		log(ERROR, std::stringstream() << "Preprocessor::preprocess() - Preprocessor is not in the UNPROCESSED state");
		return;
	}
	state = State::PROCESSING;

	// parses the tokens
	for (int i = 0; i < tokens.size(); i++) {
		Token& token = tokens[i];
		
		if (token.type == token.WHITESPACE) {
			writer->writeString(token.value);
		} else if (token.type == token.TEXT) {
			if (token.value[0] == '#' && directives.find(token.value) != directives.end()) {
				// this is a preprocessor directive
				(this->*directives[token.value])(i);
			} else {
				// this is not a preprocessor directive
				writer->writeString(token.value);
			}
		} else {
			log(ERROR, std::stringstream() << "Preprocessor::preprocess() - Invalid token type: " << token.type);
		}
	}

	state = State::PROCESSED_SUCCESS;
	writer->close();

	log(DEBUG, std::stringstream() << "Preprocessor::preprocess() - Preprocessed file: " << inputFile->getFileName());
}


/**
 * Skips tokens that match the given regex.
 * 
 * @param regex the regex to match
 * @param tokenI the index of the current token
 */
void Preprocessor::skipTokens(int& tokenI, std::string regex) {
	while (tokenI < tokens.size() && std::regex_match(tokens[tokenI].value, std::regex(regex))) {
		tokenI++;
	}
}

/**
 * Expects the next token to exist
 * 
 * @param tokenI the index of the expected token
 * @param errorMsg the error message to throw if the token does not exist
 */
void Preprocessor::expectToken(int& tokenI, std::string errorMsg) {
	if (tokenI >= tokens.size()) {
		log(ERROR, std::stringstream() << errorMsg);
	}
}


/**
 * Inserts the file contents into the current file.
 * 
 * USAGE: #include "filepath"|<filepath>
 * 
 * "filepath": looks for files located in the current directory.
 * <filepath>: prioritizes files located in the include directory, if not found, looks in the
 * current directory.
 */
void Preprocessor::_include(int& tokenI) {
	tokenI++;
	skipTokens(tokenI, "[ \t]");
	expectToken(tokenI, "Preprocessor::_include() - Missing include filename");

	std::string filepath = tokens[tokenI].value;

	if (filepath[0] == '"') {
		// local include
		if (filepath.back() != '"') {
			log(ERROR, std::stringstream() << "Preprocessor::_include() - Unclosed quotes around filepath: " << filepath);
			return;
		}

		std::string localFilePath = filepath.substr(1, filepath.find_last_of('"'));
	} else if (filepath[0] == '<') {
		// this is a system include
		if (filepath.back() != '>') {
			log(ERROR, std::stringstream() << "Preprocessor::_include() - Unclosed angle brackets around filepath: " << filepath);
			return;
		}

		std::string systemFilePath = filepath.substr(1, filepath.length() - 2);
	} else {
		log(ERROR, std::stringstream() << "Preprocessor::_include() - Invalid include filepath: " << filepath);
		return;
	}
}

/**
 * Defines a macro symbol with n arguments and optionally a return type.
 * 
 * USAGE: #macro [symbol]([arg1 ?: TYPE, arg2 ?: TYPE,..., argn ?: TYPE]) ?: TYPE
 * 
 * If a return type is specified and the macro definition does not return a value an error is thrown.
 * There cannot be a macro definition within this macro definition.
 * Note that the macro symbol is separate from label symbols and will not be pressent after preprocessing.
 */
void Preprocessor::_macro(int& tokenI) {

}

/**
 * Stops processing the macro and returns the value of the expression.
 * 
 * USAGE: #macret [?expression]
 * 
 * If the macro does not have a return type the macret must return nothing.
 * If the macro has a return type the macret must return a value of that type
 */
void Preprocessor::_macret(int& tokenI) {

}

/**
 * Closes a macro definition.
 * 
 * USAGE: #macend
 * 
 * If a macro is not closed an error is thrown.
 */
void Preprocessor::_macend(int& tokenI) {

}

/**
 * Invokes the macro with the given arguments.
 * 
 * USAGE: #invoke [symbol]([arg1, arg2,..., argn]) [?symbol]
 * 
 * If provided an output symbol, the symbol will be associated with the return value of the macro.
 * If the macro does not return a value but an output symbol is provided, an error is thrown.
 */
void Preprocessor::_invoke(int& tokenI) {

}

/**
 * Associates the symbol with a value
 * 
 * USAGE: #define [symbol] [?value]
 * 
 * Replaces all instances of symbol with the value
 * If value is not specified, the default is 0
 */
void Preprocessor::_define(int& tokenI) {

}

/**
 * Determines whether to include the following text based on whether the symbol is defined
 * 
 * USAGE: #ifdef [symbol]
 * 
 * Must be closed by a #endif
 */
void Preprocessor::_ifdef(int& tokenI) {

}

/**
 * Determines whether to include the following text based on whether the symbol is not defined
 * 
 * USAGE: #ifndef [symbol]
 * 
 * Must be closed by a #endif
 */
void Preprocessor::_ifndef(int& tokenI) {

}

/**
 * Counterpart to #ifdef and #ifndef, only includes the following text if the previous #ifdef or #ifndef
 * was not included
 * 
 * USAGE: #else
 * 
 * Must be preceded by a #ifdef or #ifndef and closed by a #endif
 */
void Preprocessor::_else(int& tokenI) {

}

/**
 * Counterpart to #ifdef and #ifndef, only includes the following text if the previous #ifdef or #ifndef
 * was not included and the symbol is defined
 * 
 * USAGE: #elsedef [symbol]
 * 
 * Must be preceded by a #ifdef or #ifndef and closed by a #endif
 */
void Preprocessor::_elsedef(int& tokenI) {

}

/**
 * Counterpart to #ifdef and #ifndef, only includes the following text if the previous #ifdef or #ifndef
 * was not included and the symbol is not defined
 * 
 * USAGE: #elsendef [symbol]
 * 
 * Must be preceded by a #ifdef or #ifndef and closed by a #endif
 */
void Preprocessor::_elsendef(int& tokenI) {

}

/**
 * Closes a #ifdef, #ifndef, #else, #elsedef, or #elsendef
 * 
 * USAGE: #endif
 * 
 * Must be preceded by a #ifdef, #ifndef, #else, #elsedef, or #elsendef
 */
void Preprocessor::_endif(int& tokenI) {

}

/**
 * Undefines a symbol defined by #define
 * 
 * USAGE: #undefine [symbol]
 * 
 * This will still work if the symbol was never defined previously
 */
void Preprocessor::_undefine(int& tokenI) {

}


/**
 * Returns the state of the preprocessor
 * 
 * @return the state of the preprocessor
 */
Preprocessor::State Preprocessor::getState() {
	return state;
}