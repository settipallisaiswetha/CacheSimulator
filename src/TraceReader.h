
#ifndef TRACE_READER_H
#define TRACE_READER_H

#include <string>
#include <vector>

struct TraceOperation {
    char type;
    unsigned long long address;
    unsigned long long data;
};

class TraceReader {
public:
    static std::vector<TraceOperation> readTrace(
        const std::string& filename
    );
};

#endif
