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

namespace StatusReturn
{
    static std::string Success = "Success";
    static std::string ErrorUnknownUser = "Error: unknown user";
    static std::string ErrorEmptyData = "Error: data empty";
    static std::string ErrorType = "Error: invalid type";
}

struct Balance {
    int USD = {};
    int RUB = {};

    std::string ToString()
    {
        return "USD: " + std::to_string(USD) + '\n' +
               "RUB: " + std::to_string(RUB);
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
        Buy
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
