// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2019, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2019, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE
// -----------------------------------------------------------------------------------------------------

#include <deque>
#include <list>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#include <seqan3/range/view/view_all.hpp>

using namespace seqan3;

// ============================================================================
//  sequential_read
// ============================================================================

template <typename container_t, typename adaptor_t>
void sequential_read(benchmark::State& state)
{
    container_t c;
    c.resize(1'000'000);
    uint8_t i = 0;
    for (auto & e : c)
        e = ++i; // dummy values

    uint8_t dummy = 0;

    if constexpr (std::Same<adaptor_t, void>)
    {
        for (auto _ : state)
        {
            for (auto e : c)
                dummy += e;
        }
    }
    else
    {
        adaptor_t adaptor;
        auto v = c | adaptor;

        for (auto _ : state)
        {
            for (auto e : v)
                dummy += e;
        }
    }

    [[maybe_unused]] volatile uint8_t dummy2 = dummy;
}

BENCHMARK_TEMPLATE(sequential_read, std::string, void);
BENCHMARK_TEMPLATE(sequential_read, std::string, decltype(std::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::string, decltype(::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::string, decltype(seqan3::view::all));

BENCHMARK_TEMPLATE(sequential_read, std::vector<uint8_t>, void);
BENCHMARK_TEMPLATE(sequential_read, std::vector<uint8_t>, decltype(std::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::vector<uint8_t>, decltype(::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::vector<uint8_t>, decltype(seqan3::view::all));

BENCHMARK_TEMPLATE(sequential_read, std::deque<uint8_t>, void);
BENCHMARK_TEMPLATE(sequential_read, std::deque<uint8_t>, decltype(std::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::deque<uint8_t>, decltype(::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::deque<uint8_t>, decltype(seqan3::view::all));

BENCHMARK_TEMPLATE(sequential_read, std::list<uint8_t>, void);
BENCHMARK_TEMPLATE(sequential_read, std::list<uint8_t>, decltype(std::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::list<uint8_t>, decltype(::ranges::view::all));
BENCHMARK_TEMPLATE(sequential_read, std::list<uint8_t>, decltype(seqan3::view::all));

// ============================================================================
//  run
// ============================================================================

BENCHMARK_MAIN();
