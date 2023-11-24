#include "Build.h"
#include "StringUtil.h"
#include "Logger.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>

/**
 * Constructs a build process from the specified arguments.
 * 
 * @param compilerArgs the arguments to construct the build process from
 */
Process::Process(std::string compilerArgs) {
	log(LOG, std::stringstream() << "Building Process: args(" << compilerArgs << ")\n" 
			<< "Current Working Directory: " << std::filesystem::current_path().string());

	// split command args by whitespace unless surrounded by quotes
	std::vector<std::string> argsList;
	bool isEscaped = false;
	bool isQuoted = false;
	std::string curArg = "";
	for (int i = 0; i < compilerArgs.length(); i++) {
		char c = compilerArgs[i];
		if (c == '\"' && !isEscaped) {
			// this is a quote that is not escaped
			isQuoted = !isQuoted;
		} else if (std::isspace(c) && !isQuoted) {
			// only add argument if it's not empty
			if (curArg.length() > 0) {
				argsList.push_back(curArg);
				curArg = "";
			}
		} else {
			// check if escape character
			if (c == '\\') {
				isEscaped = !isEscaped;
			} else {
				isEscaped = false;
			}
			curArg += c;
		}
	}

	// check if there are any dangling quotes or escape characters
	if (isQuoted) {
		log(ERROR, std::stringstream() << "Process::Process() - Missing end quotes: " << compilerArgs);
	} else if (isEscaped) {
		log(ERROR, std::stringstream() << "Process::Process() - Dangling escape character: " << compilerArgs);
	}

	// add the last argument if it's not empty
	if (curArg.length() > 0) {
		argsList.push_back(curArg);
	}

	log(DEBUG, std::stringstream() << "Process::Process() - argsList.size(): " << argsList.size());
	for (int i = 0; i < argsList.size(); i++) {
		log(DEBUG, std::stringstream() << "Process::Process() - argsList[" << i << "]: " << argsList[i]);
	}

	// parse arguments
	for (int i = 0; i < argsList.size(); i++) {
		log(LOG, std::stringstream() << "arg" << i << ": " << argsList[i]);

		std::string& arg = argsList[i];
		if (arg[0] == '-') {
			// this is a flag
			if (flags.find(arg) == flags.end()) {
				log(ERROR, std::stringstream() << "Process::Process() - Invalid flag: " << arg);
			}

			(this->*flags[arg])(argsList, i);
		} else {
			// this should be a file
			File* file = new File(arg);

			// check the extension
			if (file->getExtension() != SOURCE_EXTENSION) {
				log(ERROR, std::stringstream() << "Process::Process() - Invalid file extension: " << file->getExtension());
			}

			sourceFiles.push_back(file);
		}
	}
}

/**
 * Destructs a build process.
 */
Process::~Process() {

}


/**
 * Builds the executable from the compile arguments
 */
void Process::build() {

}

/**
 * Only preprocesses any source files
 */
void Process::preprocess() {

}

/**
 * Only assembles any processed files
 */
void Process::assemble() {

}

/**
 * Only links any object files
 */
void Process::link() {

}


/**
 * Prints out the version of the assembler
 * 
 * USAGE: -v, -version
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_version(std::vector<std::string>& args, int& index) {

}

/**
 * Compiles the source code files to object files and stops
 * 
 * USAGE: -c, -compile
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_compile(std::vector<std::string>& args, int& index) {

}

/**
 * Sets the output file
 * 
 * USAGE: -o, -output [filename]
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_output(std::vector<std::string>& args, int& index) {

}

/**
 * Sets the optimization level
 * 
 * USAGE: -o, -optimize [level]
 * 
 * Optimization Levels
 * 0 - no optimization
 * 1 - basic optimization
 * 2 - advanced optimization
 * 3 - full optimization
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_optimize(std::vector<std::string>& args, int& index) {

}

/**
 * Sets the highest optimization level
 * 
 * USAGE: -O, -oall
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_optimizeAll(std::vector<std::string>& args, int& index) {

}

/**
 * Adds debug information to the executable
 * 
 * USAGE: -g, -debug
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_debug(std::vector<std::string>& args, int& index) {

}

/**
 * Turns on warning messages
 * 
 * USAGE: -w, -warning [type]
 * 
 * Warning Types
 * error - turns warnings into errors
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_warn(std::vector<std::string>& args, int& index) {

}

/**
 * Turns on all warning messages
 * 
 * USAGE: -W, -wall
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_warnAll(std::vector<std::string>& args, int& index) {

}

/**
 * Adds directory to the list of system directories to search for included files
 * 
 * USAGE: -I, -inc, -include [directory path]
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_include(std::vector<std::string>& args, int& index) {

}

/**
 * Adds shared library to be linked with the compiled object files
 * 
 * USAGE: -l, -lib, -library [library name]
 * 
 * Specifically, it links to the shared library [library name].so
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_library(std::vector<std::string>& args, int& index) {

}

/**
 * Adds directory to the list of directories to search for shared libraries
 * 
 * USAGE: -L, -libdir, -librarydir [directory path]
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_libraryDirectory(std::vector<std::string>& args, int& index) {

}

/**
 * Adds a preprocessor flag
 * 
 * USAGE: -D [flag name?=value]
 * 
 * @param args the arguments passed to the build process
 * @param index the index of the flag in the arguments list
 */
void Process::_preprocessorFlag(std::vector<std::string>& args, int& index) {
	
}