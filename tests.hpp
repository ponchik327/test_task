#pragma once
#include <cassert>

#include "json.hpp"
#include "Common.hpp"
#include "Core.h"

void TestCreateTradeApplication()
{
    GetCore().RegisterNewUser("Seva"); // id 0
    assert(GetCore().GetUserBalance("0") == "USD: 0\nRUB: 0");

    GetCore().RegisterNewUser("Andrei"); // id 1
    assert(GetCore().GetUserBalance("1") == "USD: 0\nRUB: 0");

    GetCore().RegisterNewUser("Oleg"); // id 2
    assert(GetCore().GetUserBalance("2") == "USD: 0\nRUB: 0");

    GetCore().CreateTradeApp("0", R"({"Price":62,"Type":"buy","Volume":10})");
    GetCore().CheckMatch();
    assert(GetCore().GetUserBalance("0") == "USD: 0\nRUB: 0");

    GetCore().CreateTradeApp("1", R"({"Price":63,"Type":"buy","Volume":20})");
    GetCore().CheckMatch(); 

    assert(GetCore().GetUserBalance("1") == "USD: 0\nRUB: 0");

    GetCore().CreateTradeApp("2", R"({"Price":61,"Type":"sell","Volume":50})");
    GetCore().CheckMatch();

    assert(GetCore().GetUserBalance("0") == "USD: 10\nRUB: -620");
    assert(GetCore().GetUserBalance("1") == "USD: 20\nRUB: -1260");
    assert(GetCore().GetUserBalance("2") == "USD: -30\nRUB: 1880");
}

void RunTests()
{
    TestCreateTradeApplication();
    std::cout << "Tests all ok!" << std::endl;    
}