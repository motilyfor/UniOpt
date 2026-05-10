#include <benchmark/benchmark.h>

#include <cstdint>
#include <cstdlib>
#include <vector>

#include "src/common/clz.hpp"

namespace {

inline int NlzForLoop(uint32_t x)
{
    if (x == 0) {
        return 32;
    }

    int count = 0;
    // Check bit from MSB to LSB until the first 1 is found.
    for (int bit = 31; bit >= 0; --bit) {
        if ((x >> bit) & 1U) {
            break;
        }
        ++count;
    }
    return count;
}

inline int NlzJump(uint32_t x)
{
    int n = 0;
    if (x == 0) {
    		return 32;
    }
	n = 1;
	if ((x >> 16) == 0) {n = n + 16; x = x << 16;}
	if ((x >> 24) == 0) {n = n + 8; x = x << 8;}
	if ((x >> 28) == 0) {n = n + 4; x = x << 4;}
	if ((x >> 30) == 0) {n = n + 2; x = x << 2;}
	n = n - (x >> 31);
	return n;
}

inline int NlzUnJump(uint32_t x)
{
    int y, m, n = 0;

    y = -(x >> 16);
    m = (y >> 16) & 16u;
    n = 16 - m;
    x = x >> m;

    y = x - 0x100;
    m = (y >> 16) & 8u;
    n += m;
    x <<= m;

    y = x - 0x1000;
    m = (y >> 16) & 4u;
    n += m;
    x <<= m;

    y = x - 0x4000;
    m = (y >> 16) & 2u;
    n += m;
    x <<= m;

    y = x >> 14;
    m = y & ~(y >> 1);
    return n + 2 - m;
}


std::vector<uint32_t> BuildInput(std::size_t count)
{
    std::vector<uint32_t> input(count);
    uint32_t              state = 0x12345678U;
    for (std::size_t i = 0; i < count; ++i) {
        // Simple LCG to avoid predictable patterns.
        state = state * 1664525U + 1013904223U;
        input[i] = state;
    }

    // Force edge cases into the workload.
    if (!input.empty()) {
        input[0] = 0U;
    }
    if (input.size() > 1) {
        input[1] = 1U;
    }
    if (input.size() > 2) {
        input[2] = 0x80000000U;
    }
    return input;
}

} // namespace

static void BM_NlzForLoop(benchmark::State& state)
{
    const std::size_t         count = static_cast<std::size_t>(state.range(0));
    const std::vector<uint32_t> input = BuildInput(count);
    int64_t                   sum = 0;

    for (auto _ : state) {
        for (uint32_t x : input) {
            sum += NlzForLoop(x);
        }
    }

    benchmark::DoNotOptimize(sum);
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    state.counters["lookups_per_second"] = benchmark::Counter(
        static_cast<double>(state.iterations()) * static_cast<double>(count),
        benchmark::Counter::kIsRate);
}

static void BM_NlzUnJump(benchmark::State& state)
{
    const std::size_t         count = static_cast<std::size_t>(state.range(0));
    const std::vector<uint32_t> input = BuildInput(count);
    int64_t                   sum = 0;

    for (auto _ : state) {
        for (uint32_t x : input) {
            sum += clz(x);
        }
    }

    benchmark::DoNotOptimize(sum);
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    state.counters["lookups_per_second"] = benchmark::Counter(
        static_cast<double>(state.iterations()) * static_cast<double>(count),
        benchmark::Counter::kIsRate);
}

static void BM_NlzJump(benchmark::State& state)
{
    const std::size_t         count = static_cast<std::size_t>(state.range(0));
    const std::vector<uint32_t> input = BuildInput(count);
    int64_t                   sum = 0;

    for (auto _ : state) {
        for (uint32_t x : input) {
            sum += NlzJump(x);
        }
    }

    benchmark::DoNotOptimize(sum);
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    state.counters["lookups_per_second"] = benchmark::Counter(
        static_cast<double>(state.iterations()) * static_cast<double>(count),
        benchmark::Counter::kIsRate);
}

inline int NlzTable(uint32_t x)
{
    constexpr char u = 99;
    constexpr char table[64] = {32, 31, u,  16, u,  30, 3,  u, 15, u,  u,  u,  29, 10, 2, u,
                                u,  u,  12, 14, 21, u,  19, u, u,  28, u,  25, u,  9,  1, u,
                                17, u,  4,  u,  u,  u,  11, u, 13, 22, 20, u,  26, u,  u, 18,
                                5,  u,  u,  23, u,  27, u,  6, u,  24, 7,  u,  8,  u,  0, u};

    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    x = (x << 3) - x;
    x = (x << 8) - x;
    x = (x << 8) - x;
    x = (x << 8) - x;
    return table[x >> 26];
}

static void BM_NlzTable(benchmark::State& state)
{
    const std::size_t         count = static_cast<std::size_t>(state.range(0));
    const std::vector<uint32_t> input = BuildInput(count);
    int64_t                   sum = 0;

    for (auto _ : state) {
        for (uint32_t x : input) {
            sum += NlzTable(x);
        }
    }

    benchmark::DoNotOptimize(sum);
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    state.counters["lookups_per_second"] = benchmark::Counter(
        static_cast<double>(state.iterations()) * static_cast<double>(count),
        benchmark::Counter::kIsRate);
}

BENCHMARK(BM_NlzForLoop)->Arg(65536)
                        ->Iterations(2000)->Repetitions(3)->ReportAggregatesOnly(true);

BENCHMARK(BM_NlzTable)->Arg(65536)
                      ->Iterations(2000)->Repetitions(3)->ReportAggregatesOnly(true);

BENCHMARK(BM_NlzJump)->Arg(65536)
                      ->Iterations(2000)->Repetitions(3)->ReportAggregatesOnly(true);

BENCHMARK(BM_NlzUnJump)->Arg(65536)
                         ->Iterations(2000)->Repetitions(3)->ReportAggregatesOnly(true);

BENCHMARK_MAIN();
