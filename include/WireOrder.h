#pragma once
#include <cstdint>
#include <Order.h>

// Ensure no padding
#pragma pack(push, 1)
struct WireOrder {
    uint64_t order_id;     
    uint64_t timestamp_ns; 
    uint64_t price; 
    uint32_t quantity;     
    char symbol[8];      
    Side side;            
    OrderType type;   
    
    void print() {
        std::cout << 
        " Order Id: " << order_id << 
        " timestamp: " << timestamp_ns << 
        " Symbol: " << symbol <<
        " Price: " << price <<
        " Quantity: " << quantity << 
        " Side: "<< static_cast<int>(side) <<
        " Type: " << static_cast<int>(type) <<
        std::endl;
    }
};
#pragma pack(pop)

static_assert(sizeof(WireOrder) == 38, "WireOrder must be exactly 38 bytes");
