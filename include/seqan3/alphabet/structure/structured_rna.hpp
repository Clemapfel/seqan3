// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2019, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2019, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE
// -----------------------------------------------------------------------------------------------------

/*!\file
 * \author Joerg Winkler <j.winkler AT fu-berlin.de>
 * \brief Contains the composition of nucleotide with structure alphabets.
 */

#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include <seqan3/alphabet/composition/cartesian_composition.hpp>
#include <seqan3/alphabet/nucleotide/concept.hpp>
#include <seqan3/alphabet/structure/rna_structure_concept.hpp>

namespace seqan3
{

/*!\brief A seqan3::cartesian_composition that joins a nucleotide alphabet with an RNA structure alphabet.
 * \ingroup structure
 * \implements seqan3::RnaStructureAlphabet
 * \implements seqan3::detail::ConstexprAlphabet
 * \implements seqan3::TriviallyCopyable
 * \implements seqan3::StandardLayout
 * \tparam sequence_alphabet_t Type of the first letter; must satisfy seqan3::NucleotideAlphabet.
 * \tparam structure_alphabet_t Types of further letters; must satisfy seqan3::RnaStructureAlphabet.
 *
 * This composition pairs a nucleotide alphabet with a structure alphabet. The rank values
 * correpsond to numeric values in the size of the composition, while the character values
 * are taken from the sequence alphabet and the structure annotation is taken from the structure
 * alphabet.
 *
 * As with all `seqan3::cartesian_composition` s you may access the individual alphabet letters in
 * regular c++ tuple notation, i.e. `get<0>(t)` and objects can be brace-initialized
 * with the individual members.
 *
 * \snippet test/snippet/alphabet/structure/structured_rna.cpp general
 *
 * This seqan3::cartesian_composition itself models both seqan3::NucleotideAlphabet and seqan3::RnaStructureAlphabet.
 */
template <typename sequence_alphabet_t, typename structure_alphabet_t>
//!\cond
    requires NucleotideAlphabet<sequence_alphabet_t> && RnaStructureAlphabet<structure_alphabet_t>
//!\endcond
class structured_rna :
    public cartesian_composition<structured_rna<sequence_alphabet_t, structure_alphabet_t>,
                                 sequence_alphabet_t, structure_alphabet_t>
{
private:
    //!\brief The base type.
    using base_type = cartesian_composition<structured_rna<sequence_alphabet_t, structure_alphabet_t>,
                                            sequence_alphabet_t, structure_alphabet_t>;
public:
    //!\brief First template parameter as member type.
    using sequence_alphabet_type = sequence_alphabet_t;
    //!\brief Second template parameter as member type.
    using structure_alphabet_type = structure_alphabet_t;

    //!\brief Equals the char_type of sequence_alphabet_type.
    using char_type = underlying_char_t<sequence_alphabet_type>;

    /*!\name Constructors, destructor and assignment
     * \{
     */
    constexpr structured_rna() noexcept : base_type{} {}
    constexpr structured_rna(structured_rna const &) = default;
    constexpr structured_rna(structured_rna &&) = default;
    constexpr structured_rna & operator =(structured_rna const &) = default;
    constexpr structured_rna & operator =(structured_rna &&) = default;
    ~structured_rna() = default;

    using base_type::base_type; // Inherit non-default constructors

    //!\copydoc cartesian_composition::cartesian_composition(component_type const alph)
    SEQAN3_DOXYGEN_ONLY(( constexpr structured_rna(component_type const alph) noexcept {} ))
    //!\copydoc cartesian_composition::cartesian_composition(indirect_component_type const alph)
    SEQAN3_DOXYGEN_ONLY(( constexpr structured_rna(indirect_component_type const alph) noexcept {} ))
    //!\copydoc cartesian_composition::operator=(component_type const alph)
    SEQAN3_DOXYGEN_ONLY(( constexpr structured_rna & operator=(component_type const alph) noexcept {} ))
    //!\copydoc cartesian_composition::operator=(indirect_component_type const alph)
    SEQAN3_DOXYGEN_ONLY(( constexpr structured_rna & operator=(indirect_component_type const alph) noexcept {} ))
    //!\}

    // Inherit operators from base
    using base_type::operator=;
    using base_type::operator==;
    using base_type::operator!=;
    using base_type::operator>=;
    using base_type::operator<=;
    using base_type::operator<;
    using base_type::operator>;

    //!\name Write functions
    //!\{
    //!\brief Assign from a nucleotide character. This modifies the internal sequence letter.
    constexpr structured_rna & assign_char(char_type const c) noexcept
    {
        seqan3::assign_char(get<0>(*this), c);
        return *this;
    }

    //!\brief Strict assign from a nucleotide character. This modifies the internal sequence letter.
    structured_rna & assign_char_strict(char_type const c)
    {
        seqan3::assign_char_strict(get<0>(*this), c);
        return *this;
    }
    //!\}

    //!\name Read functions
    //!\{

    //!\brief Return a character. This reads the internal sequence letter.
    constexpr char_type to_char() const noexcept
    {
        return seqan3::to_char(get<0>(*this));
    }

    /*!\brief Return a structured_rna where the sequence letter is converted to its complement.
     * \details
     * See \ref nucleotide for the actual values.
     * Satisfies the seqan3::NucleotideAlphabet::complement() requirement via the seqan3::complement() wrapper.
     * The structure letter is not modified.
     * \par Complexity
     * Constant.
     * \par Exceptions
     * Guaranteed not to throw.
     */
    constexpr structured_rna complement() const noexcept
    {
        return structured_rna{get<0>(*this).complement(), get<1>(*this)};
    }
    //!\}

    //!\brief Validate whether a character is valid in the sequence alphabet.
    static constexpr bool char_is_valid(char_type const c) noexcept
    {
        return char_is_valid_for<sequence_alphabet_type>(c);
    }

    //!\name RNA structure properties
    //!\{

    /*!\brief Check whether the character represents a rightward interaction in an RNA structure.
     * \returns True if the letter represents a rightward interaction, False otherwise.
     */
    constexpr bool is_pair_open() const noexcept
    {
        return get<1>(*this).is_pair_open();
    };

    /*!\brief Check whether the character represents a leftward interaction in an RNA structure.
     * \returns True if the letter represents a leftward interaction, False otherwise.
     */
    constexpr bool is_pair_close() const noexcept
    {
        return get<1>(*this).is_pair_close();
    };

    /*!\brief Check whether the character represents an unpaired position in an RNA structure.
     * \returns True if the letter represents an unpaired site, False otherwise.
     */
    constexpr bool is_unpaired() const noexcept
    {
        return get<1>(*this).is_unpaired();
    };

    //!\brief The ability of this alphabet to represent pseudoknots, i.e. crossing interactions.
    static constexpr uint8_t max_pseudoknot_depth{structure_alphabet_t::max_pseudoknot_depth};

    /*!\brief Get an identifier for a pseudoknotted interaction.
     * \returns The pseudoknot id, if alph denotes an interaction, and no value otherwise.
     * It is guaranteed to be smaller than seqan3::max_pseudoknot_depth.
     */
    constexpr std::optional<uint8_t> pseudoknot_id() const noexcept
    {
        if constexpr (structure_alphabet_type::max_pseudoknot_depth > 1)
        {
            return get<1>(*this).pseudoknot_id();
        }
        else
        {
            return (is_pair_open() || is_pair_close()) ? std::optional<uint8_t>(0) : std::nullopt;
        }
    };
    //!\}
};

//!\brief Type deduction guide enables usage of structured_rna without specifying template args.
//!\relates structured_rna
template <typename sequence_alphabet_type, typename structure_alphabet_type>
structured_rna(sequence_alphabet_type &&, structure_alphabet_type &&)
    -> structured_rna<std::decay_t<sequence_alphabet_type>, std::decay_t<structure_alphabet_type>>;

} // namespace seqan3
