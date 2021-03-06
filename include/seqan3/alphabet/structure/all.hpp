// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2019, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2019, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE
// -----------------------------------------------------------------------------------------------------

/*!\file
 * \author Joerg Winkler <j.winkler AT fu-berlin.de>
 * \brief Meta-header for the structure module. It includes all headers from alphabet/structure/.
 *
 * \defgroup structure Structure
 * \ingroup alphabet
 * \brief The structure module contains alphabets for RNA and protein structure.
 * \details The following alphabets are currently supported in SeqAn. Please see the format's page for more details.
 *
 * Name                                     | Characters               | Description
 * ---------------------------------------- | ------------------------ | -----------
 * [Dot Bracket](@ref seqan3::dot_bracket3) | `().`                    | Simple annotation that defines base pairs. No pseudoknots allowed.
 * [WUSS](@ref seqan3::wuss)                | `.<>:,-_~;()[]{}AaBb...` | Annotation that provides further markups and pseudoknots.
 * [DSSP](@ref seqan3::dssp9)               | `HBEGITSCX`              | Structure encoding for proteins.
 */

#pragma once

#include <seqan3/alphabet/structure/dot_bracket3.hpp>
#include <seqan3/alphabet/structure/dssp9.hpp>
#include <seqan3/alphabet/structure/rna_structure_concept.hpp>
#include <seqan3/alphabet/structure/structured_aa.hpp>
#include <seqan3/alphabet/structure/structured_rna.hpp>
#include <seqan3/alphabet/structure/wuss.hpp>
