#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <limits>
#include <vector>

#include "open_file.h"

typedef unsigned long long element;

enum {
    ARG_CMD,
    ARG_SIZE,
    ARG_INPUT_FILE,
    ARG_EXPECTED_OUTPUT_FILE,
    ARGC_MAX,
    ARGC_MIN = ARG_INPUT_FILE + 1
};

int main(int argc, char *argv[])
{
    if (argc > ARGC_MAX || argc < ARGC_MIN) {
        std::fprintf(stderr, "Usage:\n%s input_file expected_output_file test_size\n", argv[ARG_CMD]);
        return EXIT_FAILURE;
    }

    size_t size;
    char c;
    if (std::sscanf(argv[ARG_SIZE], "%zu%c", &size, &c) != 1) {
        std::fprintf(stderr, "Can't convert %s to a number\n", argv[ARG_SIZE]);
        return EXIT_FAILURE;
    }

    std::default_random_engine generator;
    std::uniform_int_distribution<element> dist;

    std::vector<element> data;
    data.reserve(size);
    while (size--) {
        data.push_back(dist(generator));
    }

    {
        open_file input(argv[ARG_INPUT_FILE], "wb");
        std::copy(data.begin(), data.end(), open_file_write_iterator<element>(input));
        input.close();
    }

    if (argc > ARG_EXPECTED_OUTPUT_FILE) {
        std::sort(data.begin(), data.end());

        {
            open_file expected_output(argv[ARG_EXPECTED_OUTPUT_FILE], "wb");
            std::copy(data.begin(), data.end(), open_file_write_iterator<element>(expected_output));
            expected_output.close();
        }
    }
}
