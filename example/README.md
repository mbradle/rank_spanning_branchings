# Running the examples.

To run the example codes demonstrating the use of *rank_spanning_branching()*, first compile with
[Boost](https://www.boost.org), which you must have installed and used to build the [b2](https://github.com/boostorg/wiki/wiki/Getting-Started%3A-Overview) code.  If you install boost off of your home directory, you can type, for example in a bash-like shell,

    export BOOST_ROOT=~/boost
    ~/boost/b2 toolset=gcc

To run the first example, type, for example,

    bin/gcc-9.3.0/debug/rank_branchings1

To run the second example, type, for example,

    bin/gcc-9.3.0/debug/rank_branchings2 branchings_input.txt

