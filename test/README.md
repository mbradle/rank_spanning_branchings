# Running a test.

To run a test of the *rank_spanning_branchings()* routine, first compile with
Boost, which you must have installed and built the *b2* code.  If you install
boost off of your home directory, you can type, for example in a bash-like
shell,

    export BOOST_ROOT=~/boost
    ~/boost/b2 toolset=gcc

Then run a test of 10 random, complete graphs with 4 vertices by typing, for
example,

    bin/gcc-9.3.0/debug/rank_branchings_test 4 10
