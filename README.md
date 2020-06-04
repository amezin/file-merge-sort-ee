[![Build Status](https://travis-ci.org/amezin/file-merge-sort-ee.svg?branch=master)](https://travis-ci.org/amezin/file-merge-sort-ee)

Build:

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..
    $ make -j$(nproc)

Run tests:

    $ cd build
    $ make test-small  # small fast tests

or

    $ make test-big  # heavy test, no output validation

or

    $ make check  # run all tests

Run:

    $ cd build
    $ ./filemergesort input_file output_file [chunk_size]

`chunk_size` is the number of elements read into memory at once and sorted using `std::sort`
