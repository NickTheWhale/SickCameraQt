#include <algorithm>           // std::for_each
#include <boost/format.hpp>    // only needed for printing
#include <boost/histogram.hpp> // make_histogram, regular, weight, indexed
#include <cassert>             // assert (used to test this example for correctness)
#include <functional>          // std::ref
#include <iostream>            // std::cout, std::flush
#include <sstream>             // std::ostringstream


int main() {
    using namespace boost::histogram; // strip the boost::histogram prefix

    /*
      Create a 1d-histogram with a regular axis that has 6 equidistant bins on
      the real line from -1.0 to 2.0, and label it as "x". A family of overloaded
      factory functions called `make_histogram` makes creating histograms easy.

      A regular axis is a sequence of semi-open bins. Extra under- and overflow
      bins extend the axis by default (this can be turned off).

      index    :      -1  |  0  |  1  |  2  |  3  |  4  |  5  |  6
      bin edges:  -inf  -1.0  -0.5   0.0   0.5   1.0   1.5   2.0   inf
    */
    auto h = make_histogram(axis::regular<>(6, 0, 40000, "x"));

    /*
      Let's fill a histogram with data, typically this happens in a loop.

      STL algorithms are supported. std::for_each is very convenient to fill a
      histogram from an iterator range. Use std::ref in the call, if you don't
      want std::for_each to make a copy of your histogram.
    */
    for (int i = 0; i < 1'000'000; ++i)
    {
        h(rand());
    }

    /*
      This does a weighted fill using the `weight` function as an additional
      argument. It may appear at the beginning or end of the argument list. C++
      doesn't have keyword arguments like Python, this is the next-best thing.
    */
    h(0.1, weight(1.0));

    /*
      Iterate over bins with the `indexed` range generator, which provides a
      special accessor object, that can be used to obtain the current bin index,
      and the current bin value by dereferncing (it acts like a pointer to the
      value). Using `indexed` is convenient and gives you better performance than
      looping over the histogram cells with hand-written for loops. By default,
      under- and overflow bins are skipped. Passing `coverage::all` as the
      optional second argument iterates over all bins.

      - Access the value with the dereference operator.
      - Access the current index with `index(d)` method of the accessor.
      - Access the corresponding bin interval view with `bin(d)`.

      The return type of `bin(d)` depends on the axis type (see the axis reference
      for details). It usually is a class that represents a semi-open interval.
      Edges can be accessed with methods `lower()` and `upper()`.
    */

    std::ostringstream os;
    for (auto&& x : indexed(h, coverage::all)) {
        os << boost::format("bin %2i [%4.1f, %4.1f): %i\n")
            % x.index() % x.bin().lower() % x.bin().upper() % *x;
    }

    std::cout << os.str() << std::flush;
}