#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <inttypes.h>
#include <MessageParser.h>

class LatencyTracker {
    
    public:
        void analyzeLatencies(uint64_t (&timestampArr)[MessageParser::MAX_SAMPLES], uint64_t count);

};