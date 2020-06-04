#include <algorithm>
#include <cassert>
#include <list>
#include <vector>
#include <filesystem>

#include "open_file.h"
#include "tmp_file.h"

typedef unsigned long long value;

void merge_files(open_file &input_a, open_file &input_b, open_file &output)
{
    open_file_read_iterator<value> input_a_iter(input_a), input_b_iter(input_b), input_end;
    std::merge(input_a_iter, input_end, input_b_iter, input_end, open_file_write_iterator<value>(output));
}

void merge_files(tmp_file &input_a, tmp_file &input_b, open_file &output)
{
    auto open_input_a = input_a.open("rb");
    auto open_input_b = input_b.open("rb");
    merge_files(open_input_a, open_input_b, output);
}

void split_chunk(open_file_read_iterator<value> &input, std::size_t chunk_size, open_file &output)
{
    std::vector<value> values;
    values.reserve(chunk_size);

    open_file_read_iterator<value> end;
    while (values.size() < chunk_size && input != end) {
        values.push_back(*input);
        ++input;
    }

    std::sort(values.begin(), values.end());
    std::copy(values.begin(), values.end(), open_file_write_iterator<value>(output));
}

typedef std::list<tmp_file> tmp_file_set;

tmp_file_set split_input_to_chunks(open_file &input, std::size_t chunk_size, tmp_file_factory &tmp_files)
{
    assert(chunk_size > 0U);

    tmp_file_set chunks;

    open_file_read_iterator<value> input_iter(input), input_end;
    while (input_iter != input_end) {
        auto chunk = tmp_files.create();
        auto open_chunk = chunk.open("wb");
        split_chunk(input_iter, chunk_size, open_chunk);
        open_chunk.close();
        chunks.push_back(std::move(chunk));
    }

    return chunks;
}

tmp_file take_chunk(tmp_file_set &chunks)
{
    auto chunk(std::move(chunks.front()));
    chunks.pop_front();
    return chunk;
}

tmp_file_set merge_chunks(tmp_file_set &chunks, tmp_file_factory &tmp_files)
{
    tmp_file_set out_chunks;

    while (chunks.size() >= 2U) {
        auto chunk_a(take_chunk(chunks)), chunk_b(take_chunk(chunks));
        auto out_chunk = tmp_files.create();
        auto open_out_chunk = out_chunk.open("wb");
        merge_files(chunk_a, chunk_b, open_out_chunk);
        open_out_chunk.close();
        out_chunks.push_back(std::move(out_chunk));
    }

    if (!chunks.empty()) {
        out_chunks.push_back(take_chunk(chunks));
    }

    return out_chunks;
}

void merge_sort_file(open_file &input, open_file &output, std::size_t chunk_size, tmp_file_factory &tmp_files)
{
    auto chunks = split_input_to_chunks(input, chunk_size, tmp_files);

    while (chunks.size() > 2U) {
        std::fprintf(stderr, "Chunk count: %zu\n", chunks.size());
        chunks = merge_chunks(chunks, tmp_files);
    }

    std::fprintf(stderr, "Chunk count: %zu\n", chunks.size());

    if (chunks.size() == 2U) {
        auto chunk_a(take_chunk(chunks)), chunk_b(take_chunk(chunks));
        merge_files(chunk_a, chunk_b, output);
    } else if (chunks.size() == 1U) {
        auto chunk = take_chunk(chunks);
        auto open_chunk = chunk.open("rb");
        open_file_read_iterator<value> input(open_chunk), input_end;
        std::copy(input, input_end, open_file_write_iterator<value>(output));
    }
}

enum {
    ARG_CMD,
    ARG_INPUT_FILE,
    ARG_OUTPUT_FILE,
    ARG_CHUNK_SIZE,
    ARGC_MAX,
    ARGC_MIN = ARG_OUTPUT_FILE + 1
};

int main(int argc, char *argv[])
{
    if (argc > ARGC_MAX || argc < ARGC_MIN) {
        std::fprintf(stderr, "Usage:\n%s input_file output_file [chunk_size]\n", argv[ARG_CMD]);
        return EXIT_FAILURE;
    }

    size_t chunk_size = 1024U * 1024U;
    if (argc > ARG_CHUNK_SIZE) {
        char c;
        if (std::sscanf(argv[ARG_CHUNK_SIZE], "%zu%c", &chunk_size, &c) != 1) {
            std::fprintf(stderr, "Can't convert %s to a number\n", argv[ARG_CHUNK_SIZE]);
            return EXIT_FAILURE;
        }
    }

    try {
        auto tempdir = std::filesystem::temp_directory_path();
        auto tempbase = tempdir / "fms-tmp";

        tmp_file_factory tmp_files(tempbase.c_str());

        auto input_file = open_file(argv[ARG_INPUT_FILE], "rb");
        auto output_file = open_file(argv[ARG_OUTPUT_FILE], "wb");

        merge_sort_file(input_file, output_file, chunk_size, tmp_files);

        output_file.close();

    } catch (const std::exception &ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
