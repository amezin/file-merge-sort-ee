#include "open_file.h"

#include <cerrno>
#include <system_error>

open_file::open_file(std::FILE *file)
    : file(file)
{
}

open_file::open_file(std::FILE *file, const char *error_context)
    : file(file), error_context(error_context)
{
}

open_file::open_file(const char *path, const char *mode)
    : file(std::fopen(path, mode)), error_context(path)
{
    if (!file) {
        throw_errno();
    }
}

open_file::open_file(open_file &&rhs)
    : file(rhs.file), error_context(std::move(rhs.error_context))
{
    rhs.file = nullptr;
}

open_file &open_file::operator =(open_file &&rhs)
{
    close();
    std::swap(file, rhs.file);
    error_context = std::move(rhs.error_context);
    return *this;
}

open_file::~open_file()
{
    if (file) {
        if (std::fclose(file) != 0) {
            if (error_context.empty()) {
                std::perror("fclose");
            } else {
                std::perror(error_context.c_str());
            }
        }
    }
}

size_t open_file::read(void *ptr, size_t size, size_t n)
{
    size_t n_read = std::fread(ptr, size, n, file);
    if (n_read < n) {
        throw_if_error();
    }
    return n_read;
}

size_t open_file::write(const void *ptr, size_t size, size_t n)
{
    size_t n_written = std::fwrite(ptr, size, n, file);
    if (n_written < n) {
        throw_if_error();
    }
    return n_written;
}

void open_file::close()
{
    if (file) {
        if (std::fclose(file) != 0) {
            throw_errno();
        }
        file = nullptr;
    }
}

void open_file::throw_if_error()
{
    if (std::ferror(file)) {
        throw_errno();
    }
}

void open_file::throw_errno()
{
    if (error_context.empty()) {
        throw std::system_error(errno, std::generic_category());
    } else {
        throw std::system_error(errno, std::generic_category(), error_context);
    }
}

bool open_file::eof() const
{
    return !file || std::feof(file) || std::ferror(file);
}

void open_file::rewind()
{
    std::rewind(file);
}

open_file open_file::tmpfile()
{
    auto file = std::tmpfile();
    if (!file) {
        throw std::system_error(errno, std::generic_category(), "tmpfile");
    }
    return open_file(file, "tmpfile");
}
