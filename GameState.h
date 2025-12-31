#ifndef GAME_STATE_H
#define GAME_STATE_H


#include<vector>
#include"Player.h"
#include"Shop.h"
#include "Action.h"
// اینجا داریم فاز های بازی رو مشخص میکنیم - تیکه بانمک
enum class Phase{
    Buy,
    Combat
};


class GameState{
    public:
        int round =1;
        Phase phase = Phase::Buy;

        vector<Player*> players;
        vector<Shop*> shops;

        // پسرک اینجا داریم یه صف اکشن درست میکنیم برای هر بازیکن - سرور کلاینت میتواند پوش کنند
        std::vector<std::vector<Action>> pendingActions;

                // Discover offers: برای هر بازیکن یک لیست از Minion* گزینه‌ها که وقتی discover رخ دهد پر می‌شود
       
        std::vector<std::vector<Minion*>> discoverOffers;
        std::vector<bool> discoverPending;
                GameState(); // اینم از کانستراکتورش
};



#endif