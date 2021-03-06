# Ranges {#tutorial_ranges}

[TOC]

This tutorial introduces the notion of *ranges*, a C++20 feature that SeqAn3 makes strong use of.

\tutorial_head{Moderate, 90 min, \ref tutorial_concepts,}

# Motivation

Traditionally most generic algorithms in the C++ standard library, like std::sort, take a pair of iterators
(e.g. the object returned by `begin()`).
If you want to sort a std::vector `v`, you have to call `std::sort(v.begin(), v.end())` and not `std::sort(v)`.
Why was this design with iterators chosen?
It is more flexible, because it allows e.g.:
  * sorting only all elements after the fifth one: `std::sort(v.begin() + 5, v.end())`
  * using non-standard iterators like reverse iterators: `std::sort(v.rbegin() + 5, v.rend())` (sorts in reverse order)

But this interface is less intuitive than just calling std::sort on the entity that you wish to sort and
it allows for more mistakes, e.g. mixing two incompatible iterators.
C++20 introduces the notion of *ranges* and provides algorithms that accept such in the namespace `std::ranges::`, e.g.
`std::ranges::sort(v)` now works if `v` is range – and vectors are ranges!

What about the two examples that suggest superiority of the iterator-based approach? In C++20 you can do the following:
  * sorting only all elements after the fifth one: `std::ranges::sort(std::view::drop(v, 5))`
  * sorting in reverse order: `std::ranges::sort(std::view::reverse(v))`

We will discuss later what `std::view::reverse(v)` does, for now it is enough to understand that it returns something
that appears like a container and that std::ranges::sort can sort it.
Later we will see that this approach offers even more flexibility than working with iterators.

# Ranges

*Ranges* are an abstraction of "a collection of items", or "something iterable". The most basic definition
requires only the existence of `begin()` and `end()` on the range.

There are different ways to classify ranges, one way is through the capabilities of its iterator.

## Range concepts

Ranges are typically either \link std::ranges::InputRange input ranges \endlink (they can be read from) or
\link std::ranges::OutputRange output ranges \endlink (they can be written to) or both.
E.g. a `std::vector<int>` is both, but a `std::vector<int> const` would only be an input range.

\link std::ranges::InputRange Input ranges \endlink have different *strengths* that are realised through more
refined concepts (i.e. types that model a stronger concept, always also model the weaker one):

| Concept                           | Description                                                 |
|-----------------------------------|-------------------------------------------------------------|
| std::ranges::InputRange           | can be iterated from beginning to end **at least once**     |
| std::ranges::ForwardRange         | can be iterated from beginning to end **multiple times**    |
| std::ranges::BidirectionalRange   | iterator can also move backwards with `--`                  |
| std::ranges::RandomAccessRange    | you can jump to elements **in constant-time** `[]`          |
| std::ranges::ContiguousRange      | elements are always stored consecutively in memory          |

For the well-known containers from the standard library this matrix shows which concepts they model:

|                                   | std::forward_list | std::list | std::deque | std::array | std::vector |
|-----------------------------------|:-----------------:|:---------:|:----------:|:----------:|:-----------:|
| std::ranges::InputRange           | ✅                 | ✅         | ✅          | ✅          | ✅           |
| std::ranges::ForwardRange         | ✅                 | ✅         | ✅          | ✅          | ✅           |
| std::ranges::BidirectionalRange   |                   | ✅         | ✅          | ✅          | ✅           |
| std::ranges::RandomAccessRange    |                   |           | ✅          | ✅          | ✅           |
| std::ranges::ContiguousRange      |                   |           |            | ✅          | ✅           |

There are also range concepts that are independent of input or output or one of the above concept, e.g.
std::ranges::SizedRange which requires that the size of a range can be computed and in constant time.

## Storage behaviour

**Containers** are the ranges most well known, they own their elements. SeqAn3 makes use of standard STL containers
like `std::vector`, but also implements some custom containers.

**Decorators** are ranges that are always defined on another range and decorate/annotate the underlying range
with additional information. They do not own the underlying range, but can contain member data of their own.

**Views** are ranges that are usually defined on another range and transform the underlying range
via some algorithm or operation.
Views do not own any data beyond their algorithm and the time it takes to construct, destruct or copy them should not
depend on the number of elements they represent. The algorithm is required to be lazy-evaluated so it is feasible to
combine multiple views. More on this below.

If you are confused about *decorators* vs *views*, think of decorators as "underlying range + data" and
views as "underlying range + algorithm".

The storage behaviour is orthogonal to the range concepts defined by the iterators mentioned above, i.e. you
can have a container that satisfies std::ranges::RandomAccessRange (e.g. `std::vector` does, but `std::list`
does not) and you can have views or decorators that do so or don't. For some combinations of iterator capabilities
and storage behaviour there are extra concept definitions, e.g. seqan3::random_access_container_concept.

# Views

As mentioned above, views are a specific kind of range.
They are incredibly useful and you will find them throughout the library.

## Lazy-evaluation

A key feature of views is that whatever transformation they apply, they do so at the moment you request an
element, not when the view is created.

\snippet doc/tutorial/ranges/range_snippets.cpp def

Here `v` is a view; creating it neither changes `vec`, nor does `v` store any elements.
The time it takes to construct `v` and its size in memory is independent of the size of `vec`.

\snippet doc/tutorial/ranges/range_snippets.cpp all

This will print "6", but the important thing is that resolving the first element of `v` to the last element of `vec`
happens **on-demand**.
This guarantees that views can be used as flexibly as iterators, but it also means that if the view performs an
expensive transformation, it will have to do so repeatedly if the same element is requested multiple times.


## Combinability

You may have wondered why we wrote

\snippet doc/tutorial/ranges/range_snippets.cpp rev_def

and not
```cpp
std::view::reverse v{vec};
```

That's because `std::view::reverse` is not the view itself, it's an *adaptor* that takes the underlying range
(in our case the vector) and returns a view object over the vector.
The exact type of this view is hidden behind the `auto` statement.
This has the advantage, that we don't need to worry about the template arguments of the view type, but more importantly
the adaptor has an additional feature: it can be *chained* with other adaptors!

\snippet doc/tutorial/ranges/range_snippets.cpp piped

What will this print?
\hint
It will print "4".
\endhint

In the above example the vector is "piped" (similar to the unix command line) into the reverse adaptor and then into
the drop adaptor and a combined view object is returned.
Note that accessing the 0th element of the view is still lazy, determining which element it maps to happens at the time
of access.

\assignment{Exercise: Fun with views I}
Look up the documentation of std::view::transform and std::view::filter.
Both take a Invocable object as parameter, e.g. a lambda function.
std::view::transform applies the lambda on each element in the underlying range and std::view::filter
filter "removes" those elements that its lambda function evaluates to false for.

What does this imply for argument types and return types of the lambda functions?

\hint
The transform's lambda should return something of the same type as the input and the filter's lambda should return
true or false!
\endhint

Task: Create a view on `std::vector vec{1, 2, 3, 4, 5, 6};` that filters out all uneven numbers and squares the
remaining (even) values, i.e.
```cpp
std::vector vec{1, 2, 3, 4, 5, 6};
auto v = vec | // ...?

std::cout << *v.begin() << '\n'; // should print 4
```
\endassignment
\solution
\snippet doc/tutorial/ranges/range_snippets.cpp solution1
\endsolution

## View concepts

Views are a specific kind of range that is formalised in the std::ranges::View concept.
Every view returned by a view adaptor models this concept, but which other range concepts are modeled by a view?

It depends on the underlying range and also the view itself.
With few exceptions, views don't model more/stronger range concepts than their underlying range (other than
std::ranges::View) and they try to preserve as much of the underlying range's concepts as possible.
For instance the view returned by `std::view::reverse` models std::ranges::RandomAccessRange (and weaker concepts)
iff the underlying range also models the respective concept.
It never models std::ranges::ContiguousRange, because the third element of the view is not located immediately after
the second in memory (but instead before the second).

Perhaps surprising to some, many views also model std::ranges::OutputRange if the underlying range does, i.e. **views
are not read-only**:

\snippet doc/tutorial/ranges/range_snippets.cpp assign_through

\assignment{Exercise: Fun with views II}
Have a look at the solution to the previous exercise (filter+transform).
Which of the following concepts do you think `v` models?

| Concept                         | yes/no? |
|---------------------------------|:-------:|
| std::ranges::InputRange         |         |
| std::ranges::ForwardRange       |         |
| std::ranges::BidirectionalRange |         |
| std::ranges::RandomAccessRange  |         |
| std::ranges::ContiguousRange    |         |
|                                 |         |
| std::ranges::View               |         |
| std::ranges::SizedRange         |         |
| std::ranges::OutputRange        |         |

\endassignment
\solution

| Concept                         | yes/no? |
|---------------------------------|:-------:|
| std::ranges::InputRange         |   ✅    |
| std::ranges::ForwardRange       |   ✅    |
| std::ranges::BidirectionalRange |   ✅    |
| std::ranges::RandomAccessRange  |        |
| std::ranges::ContiguousRange    |        |
|                                 |        |
| std::ranges::View               |   ✅    |
| std::ranges::SizedRange         |        |
| std::ranges::OutputRange        |        |

The filter does not preserve RandomAccess and therefore not Contiguous, because it doesn't "know" which element
of the underlying range is the i-th one in constant time.
This also means we don't know the size.

The transform on the other hand produces a new element on every access (the result of the multiplication), therefore
`v` is not an output range, you cannot assign values to its elements.
This would have prevented modelling the ContiguousRange as well – if it hadn't been already by the filter – because
values are created on-demand and are not stored in memory at all.
\endsolution

We provide overview tables for all our view adaptors that document which concepts are modelled by the views they return.

## Views in the standard library and in SeqAn

The standard library in C++20 provides a number of useful views and SeqAn provides many views, as well.
Most views provided by SeqAn3 are specific to biological operations, like seqan3::view::trim which trims sequences
based on the quality or seqan3::view::complement which generates the complement of a nucleotide sequence.
But SeqAn3 also provides some general purpose views.

Have a look at the \link view View-submodule \endlink to get an overview of SeqAn's views and also read through the
detailed description on that page now that you had a more gentle introduction.

\assignment{Exercise: Fun with views III}
Create a small program that
  1. reads a string from the command line (first argument to the program)
  2. "converts" the string to a range of seqan3::dna5 (Bonus: throw an exception if loss of information occurs)
  3. prints the string and it's reverse complement
  4. prints the six-frame translation of the string

Use views to implement steps 2.-4.
\endassignment
\solution
\include doc/tutorial/ranges/range_solution3.cpp
\endsolution

# Containers

Containers are ranges that own their data.
SeqAn3 uses the standard library containers, like std::vector and std::list to store elements.
For certain use-cases we have introduced our own containers, though.

All standard library containers model std::ranges::ForwardRange (see above), but we have introduced container
concepts that encompass more of a containers interface.
Have a look at the API documentation of seqan3::container_concept and unfold the inheritance diagram.
What can you learn about the different refinements and their relation to the range concepts?

## The bitcompressed vector

If you followed the alphabet tutorial closely, you will know that seqan3::dna4 needs only two bits to represent it's
state.
However, single objects are always at least a byte (eight bits) big in C++.
To store sequences of small alphabets more space-efficiently, we have developed seqan3::bitcompressed_vector.

Open the API documentation of seqan3::bitcompressed_vector, display the inheritance diagram and read through the
interface overview and the detailed description.

\assignment{Exercise: The bitcompressed vector}
Create two small programs that each ask the user for a size and then create a vector of seqan3::dna4 of that size.
One program shall use std::vector, the other shall use seqan3::bitcompressed_vector.

Measure the amount of main memory that your programs use (this depends on your operating system, on UNIX the commands
`top` and `/usr/bin/time` may be of help).

Are your results as expected?
\endassignment
