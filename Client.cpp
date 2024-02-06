#include "Client.h"

using boost::asio::ip::tcp;

void SendMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aMessage)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

std::string ReadMessage(tcp::socket& aSocket)
{
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

std::string ProcessRegistration(tcp::socket& aSocket)
{
    std::string name;
    std::cout << "Hello! Enter your name: ";
    std::cin >> name;

    // Для регистрации Id не нужен, заполним его нулём
    SendMessage(aSocket, "0", Requests::Registration, name);
    return ReadMessage(aSocket);
}
 
std::string ParseTradeApp()
{
    nlohmann::json tradeApp;
    
    // обрабатываем тип сделки
    tradeApp["Type"] = Parse<std::string>("Enter type(sell or buy): ", 
                                          "Error: parse type");

    // обрабатываем количество долларов (объёма)                                      
    tradeApp["Volume"] = Parse<int>("Enter volume: ", 
                                    "Error: parse volume");

    // обрабатываем рубллей за доллар (цену)
    tradeApp["Price"] = Parse<int>("Enter price: ", 
                                   "Error: parse price");
    return tradeApp.dump();                               
}