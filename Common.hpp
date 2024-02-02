#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Hello = "Hel";
    static std::string Balance = "Bal";
}

struct Balance {
    int USD = {};
    int RUB = {};

    std::string ToString()
    {
        return "USD: " + std::to_string(USD) + '\n'
             + "RUB: " + std::to_string(RUB);
    }
};

struct User
{
    std::string name;
    Balance balance;
};

#endif //CLIENSERVERECN_COMMON_HPP
