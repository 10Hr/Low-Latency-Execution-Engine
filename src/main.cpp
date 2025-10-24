#include <iostream>
#include <vector>
#include <chrono>
#include <MessageParser.h>
#include <MessageBuilder.h>
#include <WireOrder.h>
#include <LatencyTracker.h>


int main() {
    
    const int NUM_MESSAGES = 2'000'000;
    MessageParser parser;
    LatencyTracker benchmarker;
    std::vector<Order> orders;
    orders.reserve(NUM_MESSAGES);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_MESSAGES; i++) {
        Order o = MessageBuilder::makeTestOrder(
            i,                      // order_id
            1000 + i,               // timestamp
            50.25 + i * 0.01,       // price
            10 + i % 100,           // quantity
            "AAPL",                 // symbol
            Side::Buy,              // side
            OrderType::Market       // ordertype

        );

        std::vector<uint8_t> serialized = parser.serialize(o);

        // auto parsedOrder = parser.parse(reinterpret_cast<const uint8_t*>(&o), sizeof(WireOrder));
        auto parsedOrder = parser.parse(serialized.data(), serialized.size());

        if (!parsedOrder) {
            std::cerr << "Parse failed at message " << i << "\n";
            continue;
        }

        orders.push_back(*parsedOrder);
    }

    

    auto end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << "Parsed " << orders.size() << " messages in " << seconds << " seconds.\n";
    std::cout << "Throughput: " << orders.size() / seconds << " messages/sec\n";

    auto& latencyArr = parser.getTimestampList();
    benchmarker.analyzeLatencies(latencyArr, MessageParser::MAX_SAMPLES);
}
