#include "cancel.h"

cancelled::cancelled()
{
}

cancelled::~cancelled()
{
}

const char *cancelled::what() const noexcept
{
    return "Cancelled";
}

cancellation_token::cancellation_token()
    : flag(0)
{}

void cancellation_token::set()
{
    flag = 1;
}

void cancellation_token::test() const
{
    if (flag) {
        throw cancelled();
    }
}
