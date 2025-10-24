#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <inttypes.h>
#include <MessageParser.h>
#include <LatencyTracker.h>


void LatencyTracker::analyzeLatencies(uint64_t (&timestampArr)[MessageParser::MAX_SAMPLES], uint64_t count) {
    if (count == 0) {
        std::cout << "No latency data recorded.\n";
        return;
    }

    // Convert only the valid samples into a vector for analysis
    std::vector<uint64_t> latencies(timestampArr, timestampArr + count);

    auto min = *std::min_element(latencies.begin(), latencies.end());
    auto max = *std::max_element(latencies.begin(), latencies.end());
    double avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    std::vector<uint64_t> sorted = latencies;
    std::sort(sorted.begin(), sorted.end());

    uint64_t median = sorted[sorted.size() / 2];
    uint64_t p99 = sorted[static_cast<size_t>(sorted.size() * 0.99)];
    uint64_t p999 = sorted[static_cast<size_t>(sorted.size() * 0.999)];

    std::cout << "Count: " << latencies.size() << "\n";
    std::cout << "Min: " << min << " ns\n";
    std::cout << "Median: " << median << " ns\n";
    std::cout << "Avg: " << avg << " ns\n";
    std::cout << "99th percentile: " << p99 << " ns\n";
    std::cout << "99.9th percentile: " << p999 << " ns\n";
    std::cout << "Max: " << max << " ns\n";
}
