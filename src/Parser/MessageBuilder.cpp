#include <Order.h>
#include <WireOrder.h>
#include <MessageBuilder.h>
#include <cstring>

WireOrder MessageBuilder::makeTestOrder(
    uint64_t order_id,
    uint64_t timestamp,
    double price,
    uint32_t quantity,
    const char* sym) 
{
    WireOrder w{};
    w.order_id = order_id;
    w.timestamp_ns = timestamp;
    w.price = price;
    w.quantity = quantity;
    std::strncpy(w.symbol, sym, 8);
    w.symbol[7] = '\0';
    w.side = Side::Buy;
    w.type = OrderType::Market;
    return w;
}

Order MessageBuilder::makeTestOrder(
    uint64_t order_id,
    uint64_t timestamp,
    double price,
    uint32_t quantity,
    const char* sym,
    Side side,
    OrderType orderType) 
{
    Order o{};
    o.order_id = order_id;
    o.timestamp_ns = timestamp;
    o.price = price;
    o.quantity = quantity;
    std::strncpy(o.symbol, sym, 8);
    o.symbol[7] = '\0';
    o.side = side;
    o.type = orderType;
    return o;
}
