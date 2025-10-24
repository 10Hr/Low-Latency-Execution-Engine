#pragma once
#include <Order.h>
#include <WireOrder.h>

class MessageBuilder {
    public:
        static WireOrder makeTestOrder(
            uint64_t order_id = 1,
            uint64_t timestamp = 123456789,
            double price = 42.5,
            uint32_t quantity = 100,
            const char* sym = "AAPL"
            );
        static Order makeTestOrder(
            uint64_t order_id = 1,
            uint64_t timestamp = 123456789,
            double price = 42.5,
            uint32_t quantity = 100,
            const char* sym = "AAPL",
            Side side = Side::Buy,
            OrderType orderType = OrderType::Market
        );
};