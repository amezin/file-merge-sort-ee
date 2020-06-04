#pragma once

#include <cstdio>
#include <string>

class open_file
{
public:
    explicit open_file(std::FILE *file = nullptr);
    explicit open_file(std::FILE *file, const char *error_context);
    explicit open_file(const char *path, const char *mode);

    static open_file tmpfile();

    open_file(const open_file &) = delete;
    open_file &operator =(const open_file &) = delete;

    open_file(open_file &&rhs);
    open_file &operator =(open_file &&rhs);

    virtual ~open_file();

    std::size_t read(void *ptr, std::size_t size, std::size_t n);

    template<typename T>
    std::size_t read(T *ptr, std::size_t n = 1U)
    {
        return read(ptr, sizeof(T), n);
    }

    std::size_t write(const void *ptr, std::size_t size, std::size_t n);

    template<typename T>
    std::size_t write(const T *ptr, std::size_t n = 1U)
    {
        return write(ptr, sizeof(T), n);
    }

    void close();

    bool eof() const;
    operator bool() const
    {
        return !eof();
    }

    void rewind();

private:
    void throw_if_error();
    void throw_errno();

    std::FILE *file;
    std::string error_context;
};

template<typename T>
class open_file_read_iterator
{
public:
    typedef std::input_iterator_tag iterator_category;
    typedef T value_type;
    typedef const T *pointer;
    typedef const T &reference;
    typedef std::ptrdiff_t difference_type;

    open_file_read_iterator()
        : file(nullptr)
    {
    }

    explicit open_file_read_iterator(open_file &file)
        : file(&file)
    {
        read_value();
    }

    const T &operator *() const
    {
        return current_value;
    }

    open_file_read_iterator &operator ++()
    {
        read_value();
        return *this;
    }

    open_file_read_iterator operator ++(int)
    {
        open_file_read_iterator backup(*this);
        read_value();
        return backup;
    }

    bool operator ==(const open_file_read_iterator &other) const
    {
        auto this_end = is_end();
        return this_end == other.is_end() && (this_end || file == other.file);
    }

    bool operator !=(const open_file_read_iterator &other) const
    {
        return !(*this == other);
    }

    bool is_end() const
    {
        return !file || file->eof();
    }

private:
    void read_value()
    {
        if (!is_end()) {
            file->read(&current_value);
        }
    }

    open_file *file;
    T current_value;
};

template<typename T>
class open_file_write_iterator
{
public:
    // like std::ostream_iterator

    typedef std::output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void pointer;
    typedef void reference;
    typedef void difference_type;

    open_file_write_iterator()
        : file(nullptr)
    {
    }

    explicit open_file_write_iterator(open_file &file)
        : file(&file)
    {
    }

    open_file_write_iterator &operator *()
    {
        return *this;
    }

    open_file_write_iterator &operator ++()
    {
        return *this;
    }

    open_file_write_iterator operator ++(int)
    {
        return *this;
    }

    const T &operator =(const T &value)
    {
        file->write(&value);
        return value;
    }

private:
    open_file *file;
};
