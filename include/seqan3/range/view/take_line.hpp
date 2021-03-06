// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2019, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2019, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE
// -----------------------------------------------------------------------------------------------------

/*!\file
 * \author Hannes Hauswedell <hannes.hauswedell AT fu-berlin.de>
 * \brief Provides seqan3::view::take_line and seqan3::view::take_line_or_throw.
 */

#pragma once

#include <range/v3/view/take_while.hpp>
#include <range/v3/algorithm/copy.hpp>

#include <seqan3/core/metafunction/iterator.hpp>
#include <seqan3/core/metafunction/range.hpp>
#include <seqan3/core/metafunction/transformation_trait_or.hpp>
#include <seqan3/io/exception.hpp>
#include <seqan3/range/concept.hpp>
#include <seqan3/range/shortcuts.hpp>
#include <seqan3/range/view/detail.hpp>
#include <seqan3/range/detail/inherited_iterator_base.hpp>
#include <seqan3/std/iterator>
#include <seqan3/std/ranges>
#include <seqan3/range/container/concept.hpp>
#include <seqan3/std/iterator>
#include <seqan3/std/type_traits>
#include <seqan3/std/ranges>

namespace seqan3::detail
{

// ============================================================================
//  view_take_line
// ============================================================================

/*!\brief The type returned by  seqan3::view::take_line and seqan3::view::take_line_or_throw.
 * \tparam urng_t       The type of the underlying ranges, must satisfy seqan3::view::concept.
 * \tparam require_eol  Whether to throw an exception when the input is exhausted before the end of line is reached.
 * \implements std::ranges::View
 * \implements std::ranges::RandomAccessRange
 * \ingroup view
 *
 * \details
 *
 * Note that most members of this class are generated by ranges::view_interface which is not yet documented here.
 */
template <std::ranges::View urng_t, bool require_eol>
class view_take_line : public ranges::view_interface<view_take_line<urng_t, require_eol>>
{
private:

    //!\brief The underlying range.
    urng_t urange;

    //!\brief The sentinel type is identical to that of the underlying range.
    using sentinel_type = std::ranges::sentinel_t<urng_t>;

    //!\brief The iterator type inherits from the underlying type, but overwrites several operators.
    //!\tparam rng_t Should be `urng_t` for defining #iterator and `urng_t const` for defining #const_iterator.
    template <typename rng_t>
    class iterator_type : public inherited_iterator_base<iterator_type<rng_t>, std::ranges::iterator_t<rng_t>>
    {
    private:
        //!\brief The iterator type of the underlying range.
        using base_base_t = std::ranges::iterator_t<rng_t>;
        //!\brief The CRTP wrapper type.
        using base_t      = inherited_iterator_base<iterator_type, std::ranges::iterator_t<rng_t>>;

        //!\brief Whether this iterator has reached the end (cache is only used on pure input ranges).
        bool at_end = false;

    public:
        /*!\name Constructors, destructor and assignment
         * \{
         */
        iterator_type() = default;
        constexpr iterator_type(iterator_type const & rhs) = default;
        constexpr iterator_type(iterator_type && rhs) = default;
        constexpr iterator_type & operator=(iterator_type const & rhs) = default;
        constexpr iterator_type & operator=(iterator_type && rhs) = default;
        ~iterator_type() = default;

        //!\brief Constructor that delegates to the CRTP layer.
        iterator_type(base_base_t const & it) : base_t{it} {}
        //!\}

        /*!\name Associated types
         * \brief All are derived from the base_base_t.
         * \{
         */
        using difference_type       = typename std::iterator_traits<base_base_t>::difference_type;
        using value_type            = typename std::iterator_traits<base_base_t>::value_type;
        using reference             = typename std::iterator_traits<base_base_t>::reference;
        using pointer               = typename std::iterator_traits<base_base_t>::pointer;
        using iterator_category     = iterator_tag_t<base_base_t>;
        //!\}

        /*!\name Arithmetic operators
         * \brief seqan3::detail::inherited_iterator_base operators are used unless specialised here.
         * \{
         */
        iterator_type & operator++()
        {
            *this = ++static_cast<base_base_t>(*this);

            if constexpr (!std::ranges::ForwardRange<urng_t>) // consuming behaviour for input ranges
            {
                if (!at_end)
                {
                    if (**this == '\r')
                    {
                        *this = ++static_cast<base_base_t>(*this);
                        at_end = true;
                    }

                    if (**this == '\n')
                    {
                        *this = ++static_cast<base_base_t>(*this);
                        at_end = true;
                    }
                }
            }

            return *this;
        }

        iterator_type operator++(int)
        {
            iterator_type cpy{*this};
            ++(*this);
            return cpy;
        }
        //!\}

        /*!\name Comparison operators
         * \brief We define comparison against self and against the sentinel.
         * \{
         */
        bool operator==(iterator_type const & rhs) const noexcept(!require_eol)
            requires std::ForwardIterator<base_base_t>
        {
            return static_cast<base_base_t>(*this) == static_cast<base_base_t>(rhs);
        }

        bool operator==(sentinel_type const & rhs) const noexcept(!require_eol)
        {

            if constexpr (!std::ranges::ForwardRange<urng_t>)
            {
                if (at_end)
                    return true;
            }

            if (static_cast<base_base_t>(*this) == rhs)
            {
                if constexpr (require_eol)
                    throw unexpected_end_of_input{"Reached end of input before end-of-line."};
                else
                    return true;
            }

            return **this == '\r' || **this == '\n';
        }

        friend bool operator==(sentinel_type const & lhs, iterator_type const & rhs) noexcept(!require_eol)
        {
            return rhs == lhs;
        }

        bool operator!=(sentinel_type const & rhs) const noexcept(!require_eol)
        {
            return !(*this == rhs);
        }

        bool operator!=(iterator_type const & rhs) const noexcept(!require_eol)
            requires std::ForwardIterator<base_base_t>
        {
            return static_cast<base_base_t>(*this) != static_cast<base_base_t>(rhs);
        }

        friend bool operator!=(sentinel_type const & lhs, iterator_type const & rhs) noexcept(!require_eol)
        {
            return rhs != lhs;
        }
        //!\}
    }; // class iterator_type

public:
    /*!\name Associated types
     * \{
     */
    //!\brief The reference_type.
    using reference         = reference_t<urng_t>;
    //!\brief The const_reference type is equal to the reference type if the underlying range is const-iterable.
    using const_reference   = detail::transformation_trait_or_t<seqan3::reference<urng_t const>, void>;
    //!\brief The value_type (which equals the reference_type with any references removed).
    using value_type        = value_type_t<urng_t>;
    //!\brief The size_type is void, because this range is never sized.
    using size_type         = void;
    //!\brief A signed integer type, usually std::ptrdiff_t.
    using difference_type   = difference_type_t<urng_t>;
    //!\brief The iterator type of this view (a random access iterator).
    using iterator          = iterator_type<urng_t>;
    //!\brief The const_iterator type is equal to the iterator type if the underlying range is const-iterable.
    using const_iterator    = detail::transformation_trait_or_t<std::type_identity<iterator_type<urng_t const>>, void>;
    //!\}

    /*!\name Constructors, destructor and assignment
     * \{
     */
    view_take_line() = default;
    constexpr view_take_line(view_take_line const & rhs) = default;
    constexpr view_take_line(view_take_line && rhs) = default;
    constexpr view_take_line & operator=(view_take_line const & rhs) = default;
    constexpr view_take_line & operator=(view_take_line && rhs) = default;
    ~view_take_line() = default;

    /*!\brief Construct from another range.
     * \param[in] _urange The underlying range.
     */
    view_take_line(urng_t _urange)
        : urange{std::move(_urange)}
    {}
    //!\}

    /*!\name Iterators
     * \{
     */
    /*!\brief Returns an iterator to the first element of the container.
     * \returns Iterator to the first element.
     *
     * If the container is empty, the returned iterator will be equal to end().
     *
     * ### Complexity
     *
     * Constant.
     *
     * ### Exceptions
     *
     * No-throw guarantee.
     */
    iterator begin() noexcept
    {
        return {seqan3::begin(urange)};
    }

    //!\copydoc begin()
    const_iterator begin() const noexcept
        requires const_iterable_concept<urng_t>
    {
        return {seqan3::cbegin(urange)};
    }

    //!\copydoc begin()
    const_iterator cbegin() const noexcept
        requires const_iterable_concept<urng_t>
    {
        return {seqan3::cbegin(urange)};
    }

    /*!\brief Returns an iterator to the element following the last element of the range.
     * \returns Iterator to the end.
     *
     * This element acts as a placeholder; attempting to dereference it results in undefined behaviour.
     *
     * ### Complexity
     *
     * Constant.
     *
     * ### Exceptions
     *
     * No-throw guarantee.
     */
    sentinel_type end() noexcept
    {
        return {seqan3::end(urange)};
    }

    //!\copydoc end()
    sentinel_type end() const noexcept
        requires const_iterable_concept<urng_t>
    {
        return {seqan3::cend(urange)};
    }

    //!\copydoc end()
    sentinel_type cend() const noexcept
        requires const_iterable_concept<urng_t>
    {
        return {seqan3::cend(urange)};
    }
    //!\}

    /*!\brief Convert this view into a container implicitly.
     * \tparam container_t Type of the container to convert to; must satisfy seqan3::sequence_container_concept and the
     *                     seqan3::reference_t of both must model std::CommonReference.
     * \returns This view converted to container_t.
     */
    template <sequence_container_concept container_t>
    operator container_t()
    //!\cond
        requires std::CommonReference<reference_t<container_t>, reference>
    //!\endcond
    {
        container_t ret;
        std::ranges::copy(begin(), end(), std::back_inserter(ret));
        return ret;
    }

    //!\overload
    template <sequence_container_concept container_t>
    operator container_t() const
    //!\cond
        requires const_iterable_concept<urng_t> && std::CommonReference<reference_t<container_t>, const_reference>
    //!\endcond
    {
        container_t ret;
        std::ranges::copy(cbegin(), cend(), std::back_inserter(ret));
        return ret;
    }
};

//!\brief Template argument type deduction guide that strips references.
//!\relates seqan3::detail::view_take_line
template <typename urng_t, bool require_eol = false>
view_take_line(urng_t) -> view_take_line<std::remove_reference_t<urng_t>, require_eol>;

// ============================================================================
//  take_line_fn (adaptor definition)
// ============================================================================

/*!\brief View adaptor definition for view::take_line and view::take_line_or_throw.
 * \tparam require_eol Whether to throw an exception when the input is exhausted before the end of line is reached.
 */
template <bool require_eol>
class take_line_fn : public pipable_adaptor_base<take_line_fn<require_eol>>
{
private:
    //!\brief Type of the CRTP-base.
    using base_t = pipable_adaptor_base<take_line_fn<require_eol>>;

public:
    //!\brief Inherit the base class's Constructors.
    using base_t::base_t;

private:
    //!\brief Befriend the base class so it can call impl().
    friend base_t;

    /*!\brief       Call the view's constructor with the underlying view as argument.
     * \returns     An instance of seqan3::detail::view_take_line.
     */
    template <std::ranges::View urng_t>
    static auto impl(urng_t urange)
    {
        return view_take_line<urng_t, require_eol>{std::move(urange)};
    }

    /*!\brief       Call the view's constructor with the underlying range wrapped in seqan3::view::all as argument.
     * \returns     An instance of seqan3::detail::view_take_line.
     */
    template <std::ranges::ViewableRange urng_t>
    static auto impl(urng_t && urange)
    {
        return impl(std::view::all(std::forward<urng_t>(urange)));
    }
};

} // namespace seqan3::detail

// ============================================================================
//  view::take_line (adaptor instance definition)
// ============================================================================

namespace seqan3::view
{

/*!\name General purpose views
 * \{
 */

/*!\brief               A view adaptor that returns a single line from the underlying range or the full range if there
 *                      is no newline.
 * \tparam urng_t       The type of the range being processed. See below for requirements. [template parameter is
 *                      omitted in pipe notation]
 * \param[in] urange    The range being processed. [parameter is omitted in pipe notation]
 * \returns             All characters of the underlying range up until, but excluding a unix or windows end-line
 *                      (`\n` or `\r\n`). See below for the properties of the returned range.
 * \ingroup view
 *
 * \details
 *
 * This adaptor returns a single line **excluding** the end-line character(s), *but moving the cursor behind them
 * for single-pass ranges.* I.e. for all ranges that satisfy std::ranges::ForwardRange this is the same as calling
 * \snippet test/snippet/range/view/take_line.cpp adaptor_def
 * but for *single pass input ranges* this means that the endline is also consumed.
 *
 * ### View properties
 *
 * | range concepts and reference_t  | `urng_t` (underlying range type)      | `rrng_t` (returned range type)                     |
 * |---------------------------------|:-------------------------------------:|:--------------------------------------------------:|
 * | std::ranges::InputRange         | *required*                            | *preserved*                                        |
 * | std::ranges::ForwardRange       |                                       | *preserved*                                        |
 * | std::ranges::BidirectionalRange |                                       | *preserved*                                        |
 * | std::ranges::RandomAccessRange  |                                       | *preserved*                                        |
 * | std::ranges::ContiguousRange    |                                       | *preserved*                                        |
 * |                                 |                                       |                                                    |
 * | std::ranges::ViewableRange      | *required*                            | *guaranteed*                                       |
 * | std::ranges::View               |                                       | *guaranteed*                                       |
 * | std::ranges::SizedRange         |                                       | *lost*                                             |
 * | std::ranges::CommonRange        |                                       | *lost*                                             |
 * | std::ranges::OutputRange        |                                       | *preserved*                                        |
 * | seqan3::const_iterable_concept  |                                       | *preserved*                                        |
 * |                                 |                                       |                                                    |
 * | seqan3::reference_t             | std::CommonReference<char>            | seqan3::reference_t<urng_t>                        |
 *
 * See the \link view view submodule documentation \endlink for detailed descriptions of the view properties.
 *
 * ### Example
 *
 * Behaviour on std::ranges::ForwardRange:
 * \snippet test/snippet/range/view/take_line.cpp behaviour
 *
 * On single pass std::ranges::InputRange it can be used to tokenise the input stream line-wise:
 * \snippet test/snippet/range/view/take_line.cpp tokenise
 *
 * \hideinitializer
 */
inline auto constexpr take_line = detail::take_line_fn<false>{};

// ============================================================================
//  view::take_line_or_throw (adaptor instance definition)
// ============================================================================

/*!\brief A view adaptor that returns a single line from the underlying range (throws if there is no end-of-line).
 * \throws seqan3::unexpected_end_of_input If the underlying range contains no end-of-line marker.
 * \ingroup view
 *
 * \copydetails seqan3::view::take_line
 * \hideinitializer
 */
inline auto constexpr take_line_or_throw = detail::take_line_fn<true>{};

//!\}

} // namespace seqan3::view
