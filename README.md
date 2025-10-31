# Low-Latency Execution Engine

A high-performance C++ message parsing and processing foundation designed for ultra-low-latency trading systems. This project demonstrates efficient binary message parsing with nanosecond-precision latency tracking using RDTSC timestamps.

---

## Project Status

**Current Phase: Phase 1 Complete** — Foundation & Message Processing

This repository contains the completed foundational components for a low-latency trading system:
- Binary message format with separate internal and wire representations
- High-performance message parser with validation
- RDTSC-based latency measurement infrastructure
- Single-threaded proof-of-concept achieving 4M+ messages/second

**What's Working Now:**
- Parse and serialize binary order messages at 4.1M+ messages/second (single-threaded)
- Median parsing latency ~180 nanoseconds per message
- Comprehensive field validation (symbol format, price/quantity bounds)
- Detailed latency histogram generation with percentile analysis

---

## Features

### Message Processing
- **Dual message representations**:
  - `Order` struct: 64-byte cache-aligned internal format with padding
  - `WireOrder` struct: 38-byte packed network format (no padding)
- **Binary wire protocol**: Packed format with big-endian byte ordering
- **Field validation**: 
  - Symbol must be alphanumeric (up to 8 characters)
  - Price must be positive
  - Quantity must be positive
- **Byte-order conversion**: Network-to-host and host-to-network with custom 64-bit helpers
- **Type-safe enums**: `Side` (Buy/Sell) and `OrderType` (Limit/Market/Stop)

### Performance Measurement
- **RDTSC timestamps**: Direct CPU cycle counter for nanosecond-precision timing
- **Circular buffer**: Stores up to 1 million latency samples
- **Automatic latency tracking**: Each parse operation records its duration
- **Statistical analysis**: Min, median, average, P99, P99.9, and max latency
- **Zero overhead**: Latency tracking integrated directly into parse path

### Code Quality
- **C++20 standard**: Modern language features and optimizations
- **Cache-line alignment**: `alignas(64)` ensures `Order` fits in single cache line
- **Static assertions**: Compile-time size verification for both structs
- **Optimized builds**: Compiled with `-O3` and `-march=native` flags

---

## Project Structure

```
Low-Latency-Execution-Engine/
├── CMakeLists.txt              # Top-level CMake configuration
├── include/                    # Public headers
│   ├── Order.h                 # Internal order struct (64 bytes, aligned)
│   ├── WireOrder.h             # Network wire format (38 bytes, packed)
│   ├── MessageParser.h         # Parse/serialize with validation
│   ├── MessageBuilder.h        # Test message creation utilities
│   ├── LatencyTracker.h        # Latency analysis and histograms
│   └── templates/
│       └── spsc_queue/         # (Future) Lock-free queue implementation
├── src/
│   ├── CMakeLists.txt          # Source-level CMake with compiler flags
│   ├── main.cpp                # Benchmark harness (20M message test)
│   ├── parsing/
│   │   ├── MessageParser.cpp   # Binary protocol parser
│   │   └── MessageBuilder.cpp  # Test order generation
│   └── benchmarking/
│       └── LatencyTracker.cpp  # Statistical latency analysis
└── build/                      # Build artifacts (generated)
```

**Key Files:**
- **Order.h**: Cache-aligned internal representation with padding
- **WireOrder.h**: Packed network format with `#pragma pack(1)`
- **MessageParser.cpp**: RDTSC-based timing, byte-order conversion, validation
- **main.cpp**: Single-threaded benchmark measuring 20M message roundtrip
- **LatencyTracker.cpp**: Percentile calculation (P50, P99, P99.9)

### Message Format

**Internal Order Structure** (64 bytes, cache-aligned):
```cpp
struct alignas(64) Order {
    uint64_t order_id;       // Unique order identifier
    uint64_t timestamp_ns;   // Nanosecond timestamp
    char symbol[8];          // Trading symbol (e.g., "AAPL")
    double price;            // Order price
    uint32_t quantity;       // Order quantity
    Side side;               // BUY (1) or SELL (-1)
    OrderType type;          // LIMIT (0), MARKET (1), or STOP (2)
    uint8_t _padding[20];    // Padding to reach 64 bytes
};
```

**Wire Order Structure** (38 bytes, packed):
```cpp
#pragma pack(push, 1)
struct WireOrder {
    uint64_t order_id;     
    uint64_t timestamp_ns; 
    uint64_t price;          // Double transmitted as uint64_t
    uint32_t quantity;     
    char symbol[8];      
    Side side;            
    OrderType type;   
};
#pragma pack(pop)
```

The packed wire format eliminates padding for efficient network transmission, while the internal format is optimized for CPU cache performance.

### Core Components

**1. MessageParser** (`MessageParser.cpp`)
The parser handles bidirectional conversion between wire and internal formats:

- **`parse()`**: Converts wire bytes → validated Order
  - Reads 38-byte `WireOrder` from buffer
  - Converts big-endian fields to host byte order using `ntohll()` and `ntohl()`
  - Converts uint64 representation back to double for price
  - Validates symbol (alphanumeric), price (> 0), quantity (> 0)
  - Records RDTSC timestamp before and after parsing
  - Returns `std::optional<Order>` (nullopt on validation failure)

- **`serialize()`**: Converts Order → wire bytes
  - Converts host byte order to big-endian using `htonll()` and `htonl()`
  - Converts double price to uint64 for transmission
  - Returns `std::vector<uint8_t>` containing packed bytes

- **Byte-order helpers**:
  - `hton64()` / `ntoh64()`: 64-bit network/host conversion
  - `doubleToUint64()` / `uint64ToDouble()`: Type-punning via `memcpy` (safe)

- **Validation functions**:
  - `validateSymbol()`: Checks for alphanumeric characters only
  - `validatePrice()`: Ensures price > 0
  - `validateQuantity()`: Ensures quantity > 0

**2. LatencyTracker** (`LatencyTracker.cpp`)
Analyzes the RDTSC cycle counts stored during parsing:

- Receives array of up to 1M timestamp samples
- Sorts samples to compute percentiles
- Calculates min, median, average, P99, P99.9, and max
- Outputs human-readable latency statistics in nanoseconds

**3. MessageBuilder** (`MessageBuilder.cpp`)
Utility for creating test messages:

- `makeTestOrder()`: Creates pre-configured `Order` or `WireOrder` instances
- Used by benchmark harness to generate realistic test data

### Latency Measurement Design

The parser uses `__rdtsc()` (Read Time-Stamp Counter) for ultra-precise timing:

```cpp
uint64_t start = __rdtsc();
// ... parsing logic ...
uint64_t end = __rdtsc();
recordLatency(timestamps_RDTSC, end - start);
```

- **Why RDTSC?** Sub-nanosecond precision (CPU cycle granularity)
- **Circular buffer**: Stores samples at index `s_idx % MAX_SAMPLES`
- **Zero allocation**: Fixed-size array, no dynamic memory
- **Minimal overhead**: Simple counter increment, no mutex/locking

---

## Benchmark Results

**Test Environment:**
- Single CPU core (no multi-threading)
- 20 million message roundtrip: serialize → parse → validate
- Each message: AAPL stock, varying price/quantity

**Throughput:**
```
Parsed 20,000,000 messages in 4.88 seconds
Throughput: 4.10 million messages/second
```

**Latency Statistics (20M samples):**
```
Count:    20,000,000
Min:      144 ns
Median:   180 ns
Average:  175 ns
P99:      252 ns
P99.9:    324 ns
Max:      647,172 ns (outlier due to OS interrupts)
```

**Key Observations:**
- Consistent sub-200ns median demonstrates efficient hot path
- P99 under 300ns shows low jitter
- Max outlier likely from context switch or cache miss
- Single-threaded baseline before adding concurrency

---

## Build Instructions

### Requirements
- **CMake** 3.10+
- **C++20 compiler**: GCC ≥ 10, Clang ≥ 11, or MSVC ≥ 2019
- **x86-64 processor** (for RDTSC instruction)
- **Windows**: `ws2_32` library (for network byte order functions)
- **Linux**: Standard library provides `htonl`, `ntohl`, etc.

### Building

```bash
git clone https://github.com/10Hr/Low-Latency-Execution-Engine.git
cd Low-Latency-Execution-Engine
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

**Note**: The project links against `ws2_32` (Windows Sockets) on Windows for network byte order functions. On Linux, these functions are provided by the standard library.

**Compiler Flags** (Release mode):
- `-O3`: Maximum optimization level
- `-march=native`: CPU-specific optimizations for your hardware
- `-flto`: Link-time optimization for cross-module inlining

### Running

Execute the benchmark:
```bash
# Windows
./LowLatencyExecutionEngine.exe

# Linux
./LowLatencyExecutionEngine
```

Expected output:
```
Parsed 20000000 messages in 4.87572 seconds
Throughput: 4.10196e+06 messages/sec
Count: 20000000
Min: 144 ns
Median: 180 ns
...
```

---

## Implementation Details

### Why Two Struct Formats?

**`Order` (64 bytes, aligned)**:
- Fits exactly in one CPU cache line (64 bytes on modern x86-64)
- Padding ensures no false sharing between adjacent orders
- Optimized for in-memory processing and manipulation

**`WireOrder` (38 bytes, packed)**:
- Minimizes network bandwidth (41% smaller than Order)
- `#pragma pack(1)` removes all padding
- Big-endian for standard network byte order

### Validation Strategy

All validation happens in the hot path during `parse()`:
1. **Symbol validation**: Reject non-alphanumeric characters
2. **Price validation**: Reject zero or negative prices
3. **Quantity validation**: Reject zero quantity
4. **Early return**: Invalid messages return `std::nullopt` immediately

This design prioritizes correctness over raw speed—every message is validated before entering the system.

### Performance Optimization Techniques Used

1. **Cache-line alignment**: `alignas(64)` prevents cache line splits
2. **`memcpy` for type punning**: Compiler-optimized, avoids undefined behavior
3. **RDTSC for timing**: Faster than `std::chrono` for cycle-level precision
4. **Reserve vector capacity**: `orders.reserve(NUM_MESSAGES)` prevents reallocation
5. **Direct memory access**: No heap allocations in parse path
6. **Compile flags**: `-O3 -march=native` enables aggressive optimizations

---

## Development Roadmap

### Phase 1: Foundation & Message Processing ✅ **COMPLETE**
**Goal**: Build core infrastructure with message parsing and latency measurement.

- [x] Project setup with CMake and dependencies
- [x] Binary message format definition
  - 64-byte cache-aligned `Order` struct
  - 38-byte packed `WireOrder` struct
  - Static assertions for size verification
- [x] Message parser with validation
  - Bidirectional conversion (parse/serialize)
  - Byte-order conversion (big-endian ↔ host)
  - Field validation with early return
- [x] Single-threaded proof-of-concept
  - 20M message benchmark harness
  - Throughput measurement
- [x] Latency measurement and histogram generation
  - RDTSC-based timestamp capture
  - Circular buffer for 1M samples
  - Percentile analysis (P50, P99, P99.9)
---

### Phase 2: Lock-Free SPSC Queue
**Goal**: Implement high-performance lock-free Single-Producer-Single-Consumer queue using ring buffer with atomic operations.

- [x] Study lock-free programming fundamentals
  - Memory ordering: `std::memory_order` (relaxed, acquire, release, seq_cst)
  - Atomic operations and memory barriers
  - False sharing and cache-line alignment
- [x] Implement ring buffer
  - `SPSCQueue<T, Capacity>` template class
  - Power-of-2 capacity for fast modulo (bitmask instead of %)
  - Atomic head/tail indices (`std::atomic<size_t>`)
  - Cache-line align head and tail (`alignas(64)`) to prevent false sharing
  - `push()`: Check full, write item, update tail with release semantics
  - `pop()`: Check empty, read item, update head with release semantics
  - Use acquire semantics when reading other thread's index
  - Use relaxed semantics when reading own thread's index
- [ ] Correctness testing
  - Single-threaded: Push/pop sequence, full/empty handling
  - Multi-threaded: Producer pushes 1M items, consumer pops, verify all received
  - Stress test: Alternating bursts and pauses
  - ThreadSanitizer: Compile with `-fsanitize=thread`, verify no data races
  - Validation: Compare against `std::queue` with mutex (SPSC should be 10-50x faster)

---

### Phase 3: Object Pooling & Memory Optimization
**Goal**: Eliminate all heap allocations in hot path using object pooling.

- [ ] Study memory management patterns 
  - Object pool design patterns
  - Cache-aware data layout principles
  - `std::pmr::memory_resource` and polymorphic allocators
- [ ] Implement object pool
  - `ObjectPool<T, Capacity>` template class
  - Pre-allocate array of T objects (placement new)
  - Free list using intrusive linked list
  - `allocate()`: Pop from free list O(1), return nullptr if exhausted
  - `deallocate()`: Push back to free list O(1)
  - Lock-free version using atomic CAS for free list head
  - Diagnostics: Track allocated count, peak usage, exhaustion events
- [ ] Integration
  - Replace `SPSCQueue<Order>` with `SPSCQueue<Order*>`
  - Parser allocates `Order` from pool before parsing
  - Sender deallocates `Order` back to pool after sending
  - Configurable exhaustion policy (reject vs. block)
- [ ] Verify zero allocations 
  - Valgrind/Massif: No heap allocations after initialization
  - `perf`: No malloc/free calls in hot path
  - Override global `operator new`/`delete` to detect unexpected allocations
  - Benchmark latency improvement (expect 20-50% reduction)

---

### Phase 4: Multi-Threading & Sender Implementation
**Goal**: Complete multi-threaded architecture with parser and sender threads.

- [ ] Thread design & affinity 
  - Parser thread on core 0, sender thread on core 1
  - CPU affinity: `pthread_setaffinity_np` (Linux) / `SetThreadAffinityMask` (Windows)
  - Real-time scheduling: `SCHED_FIFO` if available
  - Graceful shutdown: Atomic bool flag
  - Measure impact of affinity on latency
- [ ] Sender thread implementation
  - `Sender` class with worker thread consuming from SPSC queue
  - Batching logic: Accumulate N messages or T microseconds before send
  - TCP/UDP socket to simulated exchange
  - Serialize orders back to wire format
  - Record send timestamps for end-to-end latency
  - Network error handling: Retry, connection drops, backpressure
  - Configurable batching policy (size-based, time-based, adaptive)
- [ ] End-to-end integration 
  - Wire components: Input → parser thread → queue → sender thread → network
  - Clean shutdown: Signal handling, thread joining, resource cleanup
  - Configuration system: YAML/JSON for tunable parameters
  - Test multiple load scenarios: Burst, sustained, mixed patterns
- [ ] Simulated exchange 
  - TCP/UDP listener for incoming orders
  - Validate and send acknowledgments
  - Configurable latency simulation (10-100µs delay)
  - Error injection: Reject orders, drop connections
  - Log all received orders for verification

---

### Phase 5: Performance Optimization
**Goal**: Profile and optimize to achieve target metrics (P50 < 2µs, P99 < 10µs, 200K+ msg/s).

- [ ] Profiling & analysis 
  - CPU profiling: `perf record`/`perf report` and flame graphs
  - Cache analysis: `perf stat` for cache misses, branch mispredictions
  - Identify tail latency sources (P99, P99.9)
  - Find maximum sustainable throughput
  - Document baseline metrics before optimization
- [ ] Cache optimization 
  - Align hot data structures to cache lines (64 bytes)
  - Pack related data for spatial locality
  - `__builtin_prefetch` hints for predictable access patterns
  - Eliminate false sharing (`alignas` on atomics)
  - Measure: Compare cache miss rates before/after
- [ ] Branch prediction optimization 
  - `__builtin_expect` (likely/unlikely) for error paths
  - Eliminate branches in hot loops where possible
  - Conditional moves instead of branches for simple cases
  - Profile branch misprediction rates with `perf stat`
- [ ] Memory access patterns 
  - Convert linked structures to array-based (sequential access)
  - Struct-of-arrays vs array-of-structs analysis
  - SIMD operations using intrinsics for bulk processing
  - Test queue sizes to find optimal cache footprint
- [ ] System-level tuning 
  - Disable CPU frequency scaling (`performance` governor)
  - Disable hyperthreading if causing cache contention
  - Increase socket buffer sizes (`SO_SNDBUF`, `SO_RCVBUF`)
  - Disable Nagle's algorithm (`TCP_NODELAY`)
  - Pin to NUMA nodes if applicable
  - Test huge pages for large buffers
  - Document all tuning parameters in README

---

### Phase 6: Benchmarking & Documentation
**Goal**: Comprehensive benchmarks and performance documentation.

- [ ] Google Benchmark suite
  - Component-level benchmarks (parser, queue, sender)
  - End-to-end benchmarks at various loads
  - Measure: Throughput (msg/s), latency (P50/P99/P999), CPU utilization
  - Baseline benchmarks for regression detection
  - Export results to JSON for visualization
- [ ] Performance visualization 
  - Latency histograms (Python matplotlib or gnuplot)
  - Throughput vs latency scatter plots
  - Flame graphs from `perf` profiling data
  - Before/after optimization comparison charts
- [ ] Documentation 
  - Architecture diagrams (draw.io or similar)
  - Design decisions: Why SPSC vs MPMC, memory ordering choices
  - API documentation with Doxygen
  - Performance analysis with embedded graphs
  - Future enhancements list

---

### Phase 7: Advanced Extensions

**Extension A: FIX Protocol Support**

**Extension B: Kernel Bypass with DPDK**

**Extension C: Multi-Venue Routing**

---

## Target Success Metrics

**Phase 2-4 Combined:**
- **Latency P50**: < 2 microseconds (message arrival to queue insertion)
- **Latency P99**: < 10 microseconds
- **Throughput**: 200,000+ messages/second sustained
- **Memory**: Zero heap allocations in hot path after startup
- **CPU**: < 50% utilization at max throughput (single core)

---

## Testing

Run tests with:
```bash
cd build
ctest --output-on-failure
```

Current test coverage:
- Valid message parsing with various field combinations
- Invalid input handling (malformed symbols, zero/negative prices, zero quantity)
- Boundary conditions (maximum quantity, edge-case prices)
- Byte order conversion correctness
- Timestamp accuracy and monotonicity

---

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-component`)
3. Write tests for new functionality
4. Ensure all tests pass
5. Submit a pull request with clear description

---

## License

MIT License - see LICENSE file for details

---

## Contact

**Tyler McCluskey**  
GitHub: [@10Hr](https://github.com/10Hr)

---

*"In low-latency systems, every nanosecond counts."*