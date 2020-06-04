#pragma once

#include <random>
#include <string>

#include "open_file.h"

class tmp_file
{
public:
    tmp_file(const std::string &path);
    virtual ~tmp_file();

    tmp_file(const tmp_file &) = delete;
    tmp_file &operator =(const tmp_file &) = delete;

    tmp_file(tmp_file &&);
    tmp_file &operator =(tmp_file &&);

    open_file open(const char *mode = "wb+");
    void unlink();

private:
    std::string path;
};

class tmp_file_factory
{
public:
    tmp_file_factory(const char *basename);
    virtual ~tmp_file_factory();

    tmp_file create();

private:
    std::string basename;
    std::default_random_engine rnd;
};
