
#include "TraceReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<TraceOperation> TraceReader::readTrace(
    const std::string& filename
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Cannot open trace file: " + filename
        );
    }

    std::vector<TraceOperation> operations;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        std::istringstream stream(line);
        TraceOperation op{'R', 0, 0};

        // Ignore empty lines and comments
        std::string firstToken;
        if (!(stream >> firstToken) ||
            firstToken[0] == '#') {
            continue;
        }

        if (firstToken != "R" &&
            firstToken != "W") {
            throw std::runtime_error(
                "Invalid operation at line " +
                std::to_string(lineNumber)
            );
        }

        op.type = firstToken[0];

        if (!(stream >> op.address)) {
            throw std::runtime_error(
                "Invalid address at line " +
                std::to_string(lineNumber)
            );
        }

        if (op.type == 'W' &&
            !(stream >> op.data)) {
            throw std::runtime_error(
                "Missing write data at line " +
                std::to_string(lineNumber)
            );
        }

        std::string extra;
        if (stream >> extra && extra[0] != '#') {
            throw std::runtime_error(
                "Unexpected token at line " +
                std::to_string(lineNumber)
            );
        }

        operations.push_back(op);
    }

    return operations;
}
