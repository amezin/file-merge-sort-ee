#pragma once

#include <csignal>
#include <exception>

class cancelled : public std::exception
{
public:
    cancelled();
    ~cancelled() override;

    const char *what() const noexcept override;
};

class cancellation_token
{
public:
    cancellation_token();

    cancellation_token(const cancellation_token &) = delete;
    cancellation_token &operator =(const cancellation_token &) = delete;

    void set();
    void test() const;

private:
   volatile sig_atomic_t flag;
};
