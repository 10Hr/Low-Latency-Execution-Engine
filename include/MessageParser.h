#pragma once 

#include <Order.h>
#include <optional>
#include <vector>

class MessageParser {
    
    public:

    static const size_t MAX_SAMPLES = 1'000'000;
    
    std::optional<Order> parse(const uint8_t* data, size_t size);
    std::vector<uint8_t> serialize(const Order& order);
    void recordLatency(uint64_t (&timestampArr)[MAX_SAMPLES], uint64_t latency);
    uint64_t getIndex();
    static uint64_t (&getTimestampList())[MAX_SAMPLES];
    size_t getMaxSamples();

    private:
        // Byte-order helpers
        uint64_t hton64(uint64_t value);
        uint64_t ntoh64(uint64_t value);
        uint64_t doubleToUint64(double value);
        double uint64ToDouble(uint64_t value);

        // Validation helpers
        bool validateSymbol(const char* symbol);
        bool validatePrice(double price);
        bool validateQuantity(uint32_t qty);

        // Timestamp
        uint64_t captureTimestamp();

};