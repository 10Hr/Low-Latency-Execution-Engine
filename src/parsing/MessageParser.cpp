#include <MessageParser.h>
#include <WireOrder.h>
#include <optional>
#include <vector>
#include <bit>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <inttypes.h>
#include <x86intrin.h>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#define HAVE_HTONLL
#else
#include <arpa/inet.h>
#endif

bool checkHTONLL() {
#ifdef HAVE_HTONLL
return true;
#else
std::cout << "This platform doesn't support htonll()/ntohll\n" << endl;
return false;
#endif
}

uint64_t timestamps_RDTSC[MessageParser::MAX_SAMPLES];
static uint64_t s_idx;

//Record latency in circular buffer
void MessageParser::recordLatency(uint64_t (&timestampArr)[MessageParser::MAX_SAMPLES], uint64_t latency) {
    timestampArr[s_idx % MessageParser::MAX_SAMPLES] = latency;
    ++s_idx;
}

uint64_t MessageParser::getIndex() {
    return s_idx;
}

uint64_t (&MessageParser::getTimestampList())[MessageParser::MAX_SAMPLES] {
    return timestamps_RDTSC;
}

size_t MessageParser::getMaxSamples() {
    return MessageParser::MAX_SAMPLES;
}

std::optional<Order> MessageParser::parse(const uint8_t* data, size_t size) {
    checkHTONLL();

    uint64_t start = __rdtsc();

    if (size < sizeof(WireOrder)) return std::nullopt;

    WireOrder w{};
    std::memcpy(&w, data, sizeof(WireOrder)); 

    Order o{};
    o.order_id     = ntoh64(w.order_id);
    o.timestamp_ns = ntoh64(w.timestamp_ns);
    o.price        = uint64ToDouble(ntoh64(w.price));
    o.quantity     = ntohl(w.quantity);
    std::memcpy(o.symbol, w.symbol, sizeof(w.symbol));
    o.side = static_cast<Side>(w.side);
    o.type = static_cast<OrderType>(w.type);

    if (!validateSymbol(o.symbol) || !validatePrice(o.price) || !validateQuantity(o.quantity))
        return std::nullopt;

    uint64_t end = __rdtsc();
    recordLatency(timestamps_RDTSC, end - start);

    return o;
}

std::vector<uint8_t> MessageParser::serialize(const Order& order) {
    checkHTONLL();

    // 1. Create a WireOrder and fill fields
    WireOrder w{};
    w.order_id     = hton64(order.order_id);
    w.timestamp_ns = hton64(order.timestamp_ns);
    w.price        = hton64(doubleToUint64(order.price));
    w.quantity     = htonl(order.quantity);
    std::memcpy(w.symbol, order.symbol, sizeof(w.symbol));
    w.side = static_cast<Side>(order.side);  
    w.type = static_cast<OrderType>(order.type); 

    // 2. Copy WireOrder bytes into a vector
    std::vector<uint8_t> buffer(sizeof(WireOrder));
    std::memcpy(buffer.data(), &w, sizeof(WireOrder));

    return buffer;
}

// Byte-order helpers
uint64_t MessageParser::hton64(uint64_t value) {
    return htonll(value);
}
uint64_t MessageParser::ntoh64(uint64_t value) {
    return ntohll(value);
}

uint64_t MessageParser::doubleToUint64(double value) {
    uint64_t result;
    std::memcpy(&result, &value, sizeof(uint64_t));
    return result;
}

double MessageParser::uint64ToDouble(uint64_t value) {
    double result; 
    std::memcpy(&result, &value, sizeof(double)); 
    return result;
}

// Validation helpers
bool MessageParser::validateSymbol(const char* symbol) {
    for (size_t i = 0; i < 8; ++i) { 
        if (symbol[i] == '\0') 
            break; 
        if (!std::isalnum(symbol[i])) 
            return false; 
    } 
    return true;
}

bool MessageParser::validatePrice(double price) {
    return price > 0.0;
}
bool MessageParser::validateQuantity(uint32_t qty) {
    return qty > 0;
}

// Timestamp
uint64_t MessageParser::captureTimestamp() {
    return __rdtsc();
}
