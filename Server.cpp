#include "Server.h"

using namespace std;

using boost::asio::ip::tcp;

// ------------------------ реализация класса Session -----------------

void session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';

        // Парсим json, который пришёл нам в сообщении.
        auto j = nlohmann::json::parse(data_);
        auto reqType = j["ReqType"];

        string reply = "Error! Unknown request type";
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
            
            // после добавления заявки, проверяем появилась ли новая сделка
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

void session::handle_write(const boost::system::error_code& error)
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

// ------------------------ реализация класса Server -----------------

server::server(boost::asio::io_service& io_service)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    cout << "Server started! Listen " << port << " port" << endl;

    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session,
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