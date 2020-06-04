#include "tmp_file.h"

#include <cstdio>
#include <cerrno>
#include <ios>
#include <limits>
#include <sstream>
#include <system_error>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

tmp_file_factory::tmp_file_factory(const char *basename)
    : basename(basename)
{
}

tmp_file_factory::~tmp_file_factory()
{
}

tmp_file tmp_file_factory::create()
{
    // std::tmpfile(), mkstemp() max file limit is too low
    // open descriptor limit may be too low

    std::uniform_int_distribution<unsigned long long> random_dist;

    for (;;) {
        std::ostringstream name_fmt;
        name_fmt << basename << std::hex << random_dist(rnd);

        auto name = name_fmt.str();

        int fd = ::open(name.c_str(), O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
        if (fd != -1) {
            ::close(fd);
            return tmp_file(name);
        }
        if (errno != EEXIST) {
            throw std::system_error(errno, std::generic_category());
        }
    }
}

tmp_file::tmp_file(const std::string &path)
    : path(path)
{
}

tmp_file::~tmp_file()
{
    unlink();
}

tmp_file::tmp_file(tmp_file &&rhs)
    : path(std::move(rhs.path))
{
    rhs.path.clear();
}

tmp_file &tmp_file::operator =(tmp_file &&rhs)
{
    path = std::move(rhs.path);
    rhs.path.clear();
    return *this;
}

open_file tmp_file::open(const char *mode)
{
    return open_file(path.c_str(), mode);
}

void tmp_file::unlink()
{
    if (!path.empty()) {
        if (::unlink(path.c_str()) != 0) {
            std::perror(path.c_str());
        }
        path.clear();
    }
}
