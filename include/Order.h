#pragma once

#include <cstdint>
#include <cstddef>
#include <iostream>

enum struct Side : int8_t {
    Buy = 1,
    Sell = -1
};

enum struct OrderType : uint8_t {
    Limit = 0,
    Market = 1,
    Stop = 2
};

struct alignas(64) Order {
    uint64_t order_id;
    uint64_t timestamp_ns;
    char symbol[8];
    double price;
    uint32_t quantity;
    Side side;
    OrderType type;
    uint8_t _padding[20]{};

    Order(
        uint64_t id = 0,
        uint64_t ts = 0,
        const char* sym = "",
        double p = 0.0,
        uint32_t qty = 0,
        Side s = Side::Buy,
        OrderType t = OrderType::Limit) 
        : order_id(id),
        timestamp_ns(ts),
        price(p),
        quantity(qty),
        side(s),
        type(t)
    {
        for (int i = 0; i < 8; i++) {
            symbol[i] = sym[i] ? sym[i] : '\0';
        }
    }

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

static_assert(sizeof(Order) == 64, "Order struct must be 64 bytes!");

