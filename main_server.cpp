#include <iostream>

#include "Server.h"

void test_create_application()
{
    GetCore().RegisterNewUser("Seva"); // 0
    GetCore().RegisterNewUser("Andrei"); // 1
    GetCore().RegisterNewUser("Oleg"); // 2

    GetCore().CreateTradeApp("0", R"({"Price":62,"Type":"buy","Volume":10})");
    GetCore().CheckMatch(); 

    GetCore().CreateTradeApp("1", R"({"Price":63,"Type":"buy","Volume":20})");
    GetCore().CheckMatch(); 

    GetCore().CreateTradeApp("2", R"({"Price":61,"Type":"sell","Volume":50})");
    GetCore().CheckMatch(); 

    std::cout << "0: " << GetCore().GetUserBalance("0") << std::endl;
    std::cout << "1: " << GetCore().GetUserBalance("1") << std::endl;
    std::cout << "2: " << GetCore().GetUserBalance("2") << std::endl;
}

int main()
{
    try
    {
        boost::asio::io_service io_service;
        static Core core;

        server s(io_service);

        //test_create_application();
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}