#include "Core.h"

using namespace std;

string Core::RegisterNewUser(const string& aUserName)
{
    size_t newUserId = mUsers.size();
    mUsers[newUserId].name = aUserName;
    mUsers[newUserId].balance = {};

    return to_string(newUserId);
}

string Core::GetUserName(const string& aUserId)
{
    const auto userIt = mUsers.find(stoi(aUserId));
    if (userIt == mUsers.cend())
    {
        return StatusReturn::ErrorUnknownUser;
    }
    else
    {
        return userIt->second.name;
    }
}

string Core::GetUserBalance(const string& aUserId)
{
    const auto userIt = mUsers.find(stoi(aUserId));
    if (userIt == mUsers.cend())
    {
        return StatusReturn::ErrorUnknownUser;
    }
    else
    {
        return userIt->second.balance.ToString();
    }
}

string Core::ValidateTradeApp(const nlohmann::json& trade_app)
{
    // проверяем, что данные не пустые
    if (trade_app["Type"].empty() || trade_app["Type"].empty() || trade_app["Type"].empty())
    {
        return StatusReturn::ErrorEmptyData;
    }

    // проверяем, что тип задан правильно
    if (trade_app["Type"] != "sell" && trade_app["Type"] != "buy")
    {
        return StatusReturn::ErrorType;
    }

    return StatusReturn::Success;
}

string Core::CreateTradeApp(const string& aUserId,
                      const string& TradeApplication)
{
    // парсим json с данными о заявке
    auto ta = nlohmann::json::parse(TradeApplication);

    // валидируем данные
    string res_validate = ValidateTradeApp(ta);
    if (res_validate != StatusReturn::Success)
    {
        return res_validate;
    }   

    TradeApp::Type type = (ta["Type"] == "sell") ? TradeApp::Type::Sell : TradeApp::Type::Buy;
    int price = ta["Price"];
    int volume = ta["Volume"];

    // конструируем и добаляем заявку в хранилище
    trade_apps_.emplace_back(stoi(aUserId),
                                type, 
                                volume,
                                price
                            );

    // делаем доступ к заявке через указатель по её id
    int trade_id = trade_apps_.size();
    id_to_trade_app_[trade_id] = &trade_apps_.back();

    // делам доступ к очереди id-заявок по цене
    // засчёт очереди будет брать самые старые заявки по данной цене

    // цены на покупку и продажу храним в разных контейнерах 
    // (зачёт внутреннего устройства контейнера-мар цены отсортированны)
    if (type == TradeApp::Type::Buy)
    {
        price_to_ids_for_buy_[price].push(trade_id);
    }
    else if (type == TradeApp::Type::Sell)
    {
        price_to_ids_for_sell_[price].push(trade_id);
    }

    return res_validate;
}

void Core::DeleteTradeApp(int trade_id)
{
    // удаляем id-заявки из очереди по цене
    // для контенера со сделками на покупку
    int price = id_to_trade_app_[trade_id]->price;
    if (id_to_trade_app_[trade_id]->type == TradeApp::Type::Buy) {
        price_to_ids_for_buy_[price].pop();

        // если это была последняя заявка в очереди, то 
        // удаляем пару <цена, очередь> из контейнера 
        if (price_to_ids_for_buy_[price].empty()) 
        {
            price_to_ids_for_buy_.erase(price);
        }
    }
    // те же самые действия для контенера со сделками на продажу 
    else 
    {
        price_to_ids_for_sell_[price].pop();
        if (price_to_ids_for_sell_[price].empty()) 
        {
            price_to_ids_for_sell_.erase(price);
        }
    }

    // удаляем пару <id, указатель> из контенера предоставляющего доступ к заявкам
    id_to_trade_app_.erase(trade_id);
}

void Core::Transaction(int buy_user_id,
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

void Core::CalculateMatch(int buy_trade_id, int sell_trade_id)
{
    // вытаскиваем указатели на заявки для удобства
    auto buy_app = id_to_trade_app_[buy_trade_id];
    auto sell_app = id_to_trade_app_[sell_trade_id];

    // находим колчиество долларов для сделки
    int min_volume = min(buy_app->volume, sell_app->volume);

    // меняем значения балансов участников
    Transaction(buy_app->user_id, 
                sell_app->user_id, 
                min_volume,
                min_volume * buy_app->price 
                );

    // уменьшаем количество долларов в заявке
    sell_app->volume -= min_volume;
    if (sell_app->volume == 0)
    {
        // удаляем доступ к заявке в случае, если она закрыта
        DeleteTradeApp(sell_trade_id);
    }

    // те же действия далаем для второй заявки
    buy_app->volume -= min_volume;
    if (buy_app->volume == 0)
    {
        DeleteTradeApp(buy_trade_id);
    }
}

void Core::CheckMatch()
{
    // переменная показывает необходимо ли считать новую сделку или нет
    // при первом входе в цикл всегда - нужно
    bool calculations_required = true;
    while (calculations_required)
    {
        // если нет сделок, то считать нечего
        if (!price_to_ids_for_buy_.empty() && !price_to_ids_for_sell_.empty()) {
            
            // для сделки берём 
            // заявку с максимальной ценой на покупку 
            const auto max_buy = price_to_ids_for_buy_.rbegin();

            // и минимальной ценой на продажу (засчёт структуры мар-ов цены отсортированны)
            const auto min_sell = price_to_ids_for_sell_.begin();

            // если условие выполняется, то можем посчитать сделку
            calculations_required = max_buy->first >= min_sell->first;
            if (calculations_required)
            {
                // выполняем логику торговой сделки
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

Core& GetCore()
{
    static Core core;
    return core;
}