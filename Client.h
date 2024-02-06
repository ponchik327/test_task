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
    const std::string& aMessage);

// Возвращает строку с ответом сервера на последний запрос.
std::string ReadMessage(tcp::socket& aSocket);

// "Создаём" пользователя, получаем его ID.
std::string ProcessRegistration(tcp::socket& aSocket);

// формирует из потока ввода строку с данными торговой заявки 
std::string ParseTradeApp();

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