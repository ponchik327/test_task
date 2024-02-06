#pragma once
#include <iostream>
#include <deque>
#include <queue>

#include "json.hpp"
#include "Common.hpp"

class Core
{
public:
    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& aUserName);

    // Запрос имени клиента по ID
    std::string GetUserName(const std::string& aUserId);

    // Запрос баланса клиента по ID
    std::string GetUserBalance(const std::string& aUserId);

    // Проверяет данные торговой заявке
    std::string ValidateTradeApp(const nlohmann::json& trade_app);

    // Создаёт торговую заявку и возвращает статус строкой 
    std::string CreateTradeApp(const std::string& aUserId,
                               const std::string& TradeApplication);

    // Проверяет появились ли в контейнерах две заявки, которые можно оформить в сделку
    void CheckMatch();

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

    // ------------- вспомогательные методы ----------------

    // Удаляет доступ к торговой заявке и чистит данные о ней
    void DeleteTradeApp(int trade_id);

    // Меняет значения балансов участников сделки
    void Transaction(int buy_user_id,
                     int sell_user_id, 
                     int USD, 
                     int RUB
                    );

    // Выполняет логику торговой сделки между продавцом и покупателем
    void CalculateMatch(int buy_trade_id, int sell_trade_id);
};

Core& GetCore();