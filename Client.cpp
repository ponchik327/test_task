#include <iostream>
#include <vector>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

// Отправка сообщения на сервер по шаблону.
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

// Возвращает строку с ответом сервера на последний запрос.
std::string ReadMessage(tcp::socket& aSocket)
{
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

// "Создаём" пользователя, получаем его ID.
std::string ProcessRegistration(tcp::socket& aSocket)
{
    std::string name;
    std::cout << "Hello! Enter your name: ";
    std::cin >> name;

    // Для регистрации Id не нужен, заполним его нулём
    SendMessage(aSocket, "0", Requests::Registration, name);
    return ReadMessage(aSocket);
}

// шаблонная функция для считыванния строк и чисел из потока
template <typename Type>
Type Parse(const std::string& greeting,
           const std::string& error)
{
    Type answer;
    for (;;) {
        std::cout << greeting << std::flush;
        if ((std::cin >> answer).good()) break;
        std::cin.clear();
        std::cout << error << '\n';
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return answer;
}

// формирует из потока ввода строку с данными торговой заявки 
std::string ParseTradeApp()
{
    nlohmann::json tradeApp;
    
    // обрабатываем тип сделки
    tradeApp["Type"] = Parse<std::string>("Enter type(sell or buy): ", 
                                          "Error parse type");

    // обрабатываем количество долларов (объёма)                                      
    tradeApp["Volume"] = Parse<int>("Enter volume: ", 
                                    "Error parse volume");

    // обрабатываем рубллей за доллар (цену)
    tradeApp["Price"] = Parse<int>("Enter price: ", 
                                   "Error parse price");
    return tradeApp.dump();                               
}

int main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*iterator);

        // Мы предполагаем, что для идентификации пользователя будет использоваться ID.
        // Тут мы "регистрируем" пользователя - отправляем на сервер имя, а сервер возвращает нам ID.
        // Этот ID далее используется при отправке запросов.
        std::string my_id = ProcessRegistration(s);

        while (true)
        {
            // Тут реализовано "бесконечное" меню.
            std::cout << "Menu:\n"
                         "1) Hello Request\n"
                         "2) Check Balance\n"
                         "3) Make trade application\n"
                         "4) Exit\n"
                         << std::endl;

            short menu_option_num;
            std::cin >> menu_option_num;
            switch (menu_option_num)
            {
                case 1:
                {
                    // Этот метод получает от сервера приветствие с именем клиента,
                    // отправляя серверу id, полученный при регистрации.
                    SendMessage(s, my_id, Requests::Hello, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 2:
                {
                    // Этот метод получает от сервера значение баланса пользователя,
                    // отправляя серверу его id.
                    SendMessage(s, my_id, Requests::Balance, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 3:
                {
                    // Этот метод получает от сервера статус операции (success или error),
                    // отправляя серверу строку с данными о торговой заявке.
                    SendMessage(s, my_id, Requests::AddTradeApp, ParseTradeApp());
                    std::cout << ReadMessage(s);
                    break;
                }
                case 4:
                {
                    exit(0);
                    break;
                }
                default:
                {
                    std::cout << "Unknown menu option\n" << std::endl;
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}