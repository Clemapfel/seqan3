// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2019, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2019, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE
// -----------------------------------------------------------------------------------------------------

#include <deque>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <range/v3/algorithm/copy.hpp>
#include <range/v3/view/unique.hpp>

#include <seqan3/range/view/take.hpp>
#include <seqan3/range/view/take_exactly.hpp>
#include <seqan3/range/view/single_pass_input.hpp>
#include <seqan3/range/concept.hpp>
#include <seqan3/range/container/concept.hpp>
#include <seqan3/range/view/to_char.hpp>
#include <seqan3/std/concepts>
#include <seqan3/std/ranges>

using namespace seqan3;

// ============================================================================
//  test templates
// ============================================================================

template <typename adaptor_t>
void do_test(adaptor_t const & adaptor, std::string const & vec)
{
    // pipe notation
    auto v = vec | adaptor(3);
    EXPECT_EQ("foo", std::string(v));

    // function notation
    std::string v2{adaptor(vec, 3)};
    EXPECT_EQ("foo", v2);

    // combinability
    auto v3 = vec | adaptor(3) | adaptor(3) | ranges::view::unique;
    EXPECT_EQ("fo", std::string(v3));
    std::string v3b = vec | std::view::reverse | adaptor(3) | ranges::view::unique;
    EXPECT_EQ("rab", v3b);
}

template <typename adaptor_t>
void do_concepts(adaptor_t && adaptor, bool const exactly)
{
    std::vector vec{1, 2, 3};
    EXPECT_TRUE(std::ranges::InputRange<decltype(vec)>);
    EXPECT_TRUE(std::ranges::ForwardRange<decltype(vec)>);
    EXPECT_TRUE(std::ranges::BidirectionalRange<decltype(vec)>);
    EXPECT_TRUE(std::ranges::RandomAccessRange<decltype(vec)>);
    EXPECT_FALSE(std::ranges::View<decltype(vec)>);
    EXPECT_TRUE(std::ranges::SizedRange<decltype(vec)>);
    EXPECT_TRUE(std::ranges::CommonRange<decltype(vec)>);
    EXPECT_TRUE(const_iterable_concept<decltype(vec)>);
    EXPECT_TRUE((std::ranges::OutputRange<decltype(vec), int>));

    auto v1 = vec | adaptor;

    EXPECT_TRUE(std::ranges::InputRange<decltype(v1)>);
    EXPECT_TRUE(std::ranges::ForwardRange<decltype(v1)>);
    EXPECT_TRUE(std::ranges::BidirectionalRange<decltype(v1)>);
    EXPECT_TRUE(std::ranges::RandomAccessRange<decltype(v1)>);
    EXPECT_TRUE(std::ranges::View<decltype(v1)>);
    EXPECT_TRUE(std::ranges::SizedRange<decltype(v1)>);
    EXPECT_TRUE(std::ranges::CommonRange<decltype(v1)>);
    EXPECT_TRUE(const_iterable_concept<decltype(v1)>);
    EXPECT_TRUE((std::ranges::OutputRange<decltype(v1), int>));

    auto v2 = vec | view::single_pass_input | adaptor;

    EXPECT_TRUE(std::ranges::InputRange<decltype(v2)>);
    EXPECT_FALSE(std::ranges::ForwardRange<decltype(v2)>);
    EXPECT_FALSE(std::ranges::BidirectionalRange<decltype(v2)>);
    EXPECT_FALSE(std::ranges::RandomAccessRange<decltype(v2)>);
    EXPECT_TRUE(std::ranges::View<decltype(v2)>);
    EXPECT_EQ(std::ranges::SizedRange<decltype(v2)>, exactly);
    EXPECT_FALSE(std::ranges::CommonRange<decltype(v2)>);
    EXPECT_FALSE(const_iterable_concept<decltype(v2)>);
    EXPECT_TRUE((std::ranges::OutputRange<decltype(v2), int>));
}

// ============================================================================
//  view_take
// ============================================================================

TEST(view_take, regular)
{
    do_test(view::take, "foobar");
}

TEST(view_take, concepts)
{
    do_concepts(view::take(3), false);
}

TEST(view_take, underlying_is_shorter)
{
    std::string vec{"foo"};
    EXPECT_NO_THROW(( view::take(vec, 4) )); // no parsing

    std::string v;
    EXPECT_NO_THROW(( v = vec | view::single_pass_input | view::take(4) )); // full parsing on conversion
    EXPECT_EQ("foo", v);
}

TEST(view_take, overloads)
{
    {   // string overload
        std::string urange{"foobar"};

        auto v = view::take(urange, 3);

        EXPECT_TRUE((std::Same<decltype(v), std::string_view>));
        EXPECT_TRUE((std::ranges::equal(v, urange.substr(0,3))));
    }

    {   // stringview overload
        std::string_view urange{"foobar"};

        auto v = view::take(urange, 3);

        EXPECT_TRUE((std::Same<decltype(v), std::string_view>));
        EXPECT_TRUE((std::ranges::equal(v, urange.substr(0,3))));
    }

    {   // contiguous overload
        std::vector<int> urange{1, 2, 3, 4, 5, 6};

        auto v = view::take(urange, 3);

        EXPECT_TRUE((std::Same<decltype(v), std::span<int, std::dynamic_extent>>));
        EXPECT_TRUE((std::ranges::equal(v, std::vector{1, 2, 3})));
    }

    {   // contiguous overload
        std::array<int, 6> urange{1, 2, 3, 4, 5, 6};

        auto v = view::take(urange, 3);

        EXPECT_TRUE((std::Same<decltype(v), std::span<int, std::dynamic_extent>>));
        EXPECT_TRUE((std::ranges::equal(v, std::vector{1, 2, 3})));
    }

    {   // random-access overload
        std::deque<int> urange{1, 2, 3, 4, 5, 6};

        auto v = view::take(urange, 3);

        EXPECT_TRUE((std::Same<decltype(v), std::ranges::subrange<typename std::deque<int>::iterator,
                                                                  typename std::deque<int>::iterator>>));
        EXPECT_TRUE((std::ranges::equal(v, std::vector{1, 2, 3})));
    }

    {   // generic overload (bidirectional container)
        std::list<int> urange{1, 2, 3, 4, 5, 6};

        auto v = view::take(urange, 3);

        EXPECT_TRUE((std::Same<decltype(v), detail::view_take<std::ranges::all_view<std::list<int> &>, false, false>>));
        EXPECT_TRUE((std::ranges::equal(v, std::vector{1, 2, 3})));
    }

    {   // generic overload (view)
        std::array<int, 6> urange{1, 2, 3, 4, 5, 6};

        auto v = urange | std::view::filter([] (int) { return true; });
        auto v2 = view::take(v, 3);

        EXPECT_TRUE((std::Same<decltype(v2), detail::view_take<decltype(v), false, false>>));
        EXPECT_TRUE((std::ranges::equal(v2, std::vector{1, 2, 3})));
    }
}

// ============================================================================
//  view_take_exactly
// ============================================================================

TEST(view_take_exactly, regular)
{
    do_test(view::take_exactly, "foobar");
}

TEST(view_take_exactly, concepts)
{
    do_concepts(view::take_exactly(3), true);
}

TEST(view_take_exactly, underlying_is_shorter)
{
    std::string vec{"foo"};
    EXPECT_NO_THROW(( view::take_exactly(vec, 4) )); // no parsing

    std::string v;
    EXPECT_NO_THROW(( v = vec | view::single_pass_input | view::take_exactly(4) )); // full parsing on conversion
    EXPECT_EQ("foo", v);

    auto v2 = vec | view::single_pass_input | view::take_exactly(4);
    EXPECT_EQ(size(v2), 4u); // here be dragons
}

// ============================================================================
//  view_take_exactly_or_throw
// ============================================================================

TEST(view_take_exactly_or_throw, regular)
{
    do_test(view::take_exactly_or_throw, "foo\nbar");
}

TEST(view_take_exactly_or_throw, concepts)
{
    do_concepts(view::take_exactly_or_throw(3), true);
}

TEST(view_take_exactly_or_throw, underlying_is_shorter)
{
    std::string vec{"foo"};
    EXPECT_THROW(( view::take_exactly_or_throw(vec, 4) ),
                   std::invalid_argument); // no parsing, but throws on construction

    std::string v;
    EXPECT_THROW(( v = vec | view::single_pass_input | view::take_exactly_or_throw(4)),
                   unexpected_end_of_input); // full parsing on conversion, throw on conversion
}
