#include <string>
#include <fstream>
#include "Utilities.h"

std::string read_file(std::string filename) {
	std::ifstream infile(filename);
	std::string result;
	std::string buf;

	while (std::getline(infile, buf)) {
		result += buf + "\r\n";
	}

	return result;
}