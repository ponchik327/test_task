#include <cstdlib>
#include <iostream>
#include <deque>
#include <queue>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"

using boost::asio::ip::tcp;

class Core
{
public:
    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& aUserName)
    {
        size_t newUserId = mUsers.size();
        mUsers[newUserId].name = aUserName;
        mUsers[newUserId].balance = {};

        return std::to_string(newUserId);
    }

    // Запрос имени клиента по ID
    std::string GetUserName(const std::string& aUserId)
    {
        const auto userIt = mUsers.find(std::stoi(aUserId));
        if (userIt == mUsers.cend())
        {
            return "Error! Unknown User";
        }
        else
        {
            return userIt->second.name;
        }
    }

    // Запрос баланса клиента по ID
    std::string GetUserBalance(const std::string& aUserId)
    {
        const auto userIt = mUsers.find(std::stoi(aUserId));
        if (userIt == mUsers.cend())
        {
            return "Error! Unknown User";
        }
        else
        {
            return userIt->second.balance.ToString();
        }
    }

    std::string CreateTradeApp(const std::string& aUserId,
                               const std::string& TradeApplication)
    {
        const std::string error = "Error add trade application\n";
        auto ta = nlohmann::json::parse(TradeApplication);
        if (ta["Type"].empty() || ta["Type"].empty() || ta["Type"].empty())
        {
            return error;
        } 
        TradeApp::Type type = ParseTradeType(ta["Type"]);
        if (type == TradeApp::Type::NotFound)
        {
            return error;
        }
        int price = ta["Price"];
        int volume = ta["Volume"];
        trade_apps_.emplace_back(std::stoi(aUserId),
                                 type, 
                                 volume,
                                 price
                                );
        int trade_id = trade_apps_.size();
        id_to_trade_app_[trade_id] = &trade_apps_.back();
        if (type == TradeApp::Type::Buy)
        {
            price_to_ids_for_buy_[price].push(trade_id);
        }
        else if (type == TradeApp::Type::Sell)
        {
            price_to_ids_for_sell_[price].push(trade_id);
        }
        return "Success\n";
    }

    void DeleteTradeApp(int trade_id)
    {
        int price = id_to_trade_app_[trade_id]->price;
        if (id_to_trade_app_[trade_id]->type == TradeApp::Type::Buy) {
            price_to_ids_for_buy_[price].pop();
            if (price_to_ids_for_buy_[price].empty()) 
            {
                price_to_ids_for_buy_.erase(price);
            }
        } 
        else 
        {
            price_to_ids_for_sell_[price].pop();
            if (price_to_ids_for_sell_[price].empty()) 
            {
                price_to_ids_for_sell_.erase(price);
            }
        } 
        id_to_trade_app_.erase(trade_id);
    }

    void Transaction(int buy_user_id,
                     int sell_user_id, 
                     int USD, 
                     int RUB
                    ) 
    {
        auto& user_buy = mUsers[buy_user_id];
        auto& user_sell = mUsers[sell_user_id];

        user_buy.balance.USD += USD;
        user_buy.balance.RUB -= RUB;

        user_sell.balance.USD -= USD;
        user_sell.balance.RUB += RUB;
    }

    void CalculateMatch(int buy_trade_id, int sell_trade_id)
    {
        auto buy_app = id_to_trade_app_[buy_trade_id];
        auto sell_app = id_to_trade_app_[sell_trade_id];

        /*int diff = buy_app->volume - sell_app->volume;
        if (diff > 0)
        {
            buy_app->volume -= sell_app->volume;
            sell_app->volume = 0;

            Transaction(buy_app->user_id, 
                        sell_app->user_id, 
                        sell_app->volume,
                        sell_app->volume * buy_app->price 
                       );
            
            DeleteTradeApp(sell_trade_id);
        }
        else if (diff < 0)
        {
            sell_app->volume -= buy_app->volume;
            buy_app->volume = 0;
            
            Transaction(buy_app->user_id, 
                        sell_app->user_id, 
                        buy_app->volume,
                        buy_app->volume * buy_app->price 
                       );
            
            DeleteTradeApp(buy_trade_id);
        }
        else
        {

            Transaction(buy_app->user_id, 
                        sell_app->user_id, 
                        buy_app->volume,
                        buy_app->volume * buy_app->price 
                       );
            
            DeleteTradeApp(sell_trade_id);
            DeleteTradeApp(buy_trade_id);
        }*/
        int min_volume = std::min(buy_app->volume, sell_app->volume);

        Transaction(buy_app->user_id, 
                    sell_app->user_id, 
                    min_volume,
                    min_volume * buy_app->price 
                   );

        sell_app->volume -= min_volume;
        if (sell_app->volume == 0)
        {
            DeleteTradeApp(sell_trade_id);
        }

        buy_app->volume -= min_volume;
        if (buy_app->volume == 0)
        {
            DeleteTradeApp(buy_trade_id);
        }
    }

    void CheckMatch()
    {
        bool calculations_required = true;
        while (calculations_required)
        {
            if (!price_to_ids_for_buy_.empty() && !price_to_ids_for_sell_.empty()) {
                const auto max_buy = price_to_ids_for_buy_.rbegin();
                const auto min_sell = price_to_ids_for_sell_.begin();
                calculations_required = max_buy->first >= min_sell->first;
                if (calculations_required)
                {
                    CalculateMatch(max_buy->second.front(),
                                min_sell->second.front() 
                    );
                }
            }
            else
            {
                calculations_required = false;
            }
        }
    }

private:
    // <UserId, User>
    std::unordered_map<size_t, User> mUsers;

    // Хранилище для торговых сделок
    std::deque<TradeApp> trade_apps_;

    // <TradeId, TradeAppS>
    std::unordered_map<size_t, PtrTradeApp> id_to_trade_app_;

    // <Price, queue<TradeId>>
    std::map<int, std::queue<size_t>> price_to_ids_for_sell_;
    std::map<int, std::queue<size_t>> price_to_ids_for_buy_;

    TradeApp::Type ParseTradeType(const std::string type)
    {   
        if (type == "sell") return TradeApp::Sell;
        if (type == "buy" ) return TradeApp::Buy;
        return TradeApp::Type::NotFound;
    }
};

Core& GetCore()
{
    static Core core;
    return core;
}

class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error)
        {
            data_[bytes_transferred] = '\0';

            // Парсим json, который пришёл нам в сообщении.
            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];

            std::string reply = "Error! Unknown request type";
            if (reqType == Requests::Registration)
            {
                // Это реквест на регистрацию пользователя.
                // Добавляем нового пользователя и возвращаем его ID.
                reply = GetCore().RegisterNewUser(j["Message"]);
            }
            else if (reqType == Requests::Hello)
            {
                // Это реквест на приветствие.
                // Находим имя пользователя по ID и приветствуем его по имени.
                reply = "Hello, " + GetCore().GetUserName(j["UserId"]) + "!\n";
            }
            else if (reqType == Requests::Balance)
            {
                // Это реквест на проверку суммы баланса.
                // Находим имя пользователя по ID и говорим его баланс.
                reply = "Your balance: \n" + GetCore().GetUserBalance(j["UserId"]) + '\n';
            }
            else if (reqType == Requests::AddTradeApp)
            {
                // Это реквест на добаление информации 
                // о новой заявке от клиента 
                reply = GetCore().CreateTradeApp(j["UserId"], j["Message"]);

                GetCore().CheckMatch();
            }

            boost::asio::async_write(socket_,
                boost::asio::buffer(reply, reply.size()),
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service& io_service)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

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