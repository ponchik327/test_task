#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Hello = "Hel";
    static std::string Balance = "Bal";
    static std::string AddTradeApp = "Add";
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

struct TradeApp
{
    enum Type
    {
        Sell,
        Buy,
        NotFound
    };

    TradeApp(int u_id, 
             TradeApp::Type t, 
             int v,
             int pr) 
        : user_id(u_id)
        , type(t)
        , volume(v)
        , price(pr) {
    }

    int user_id;
    Type type;
    int volume;
    int price;
};

using PtrTradeApp = TradeApp*;

#endif //CLIENSERVERECN_COMMON_HPP
