#ifndef ACTION_H
#define ACTION_H



#include<vector>

enum class ActionType{


    Buy,
    Roll,
    ToggleFreeze,
    Sell,
    Upgrade,
    HeroPower,
    DiscoverChoice,
    EndTurn
};
struct Action{

    ActionType type;
    int slotIndex = -1;// index برای buy/sell/hero power target
    int choice = -1;// برای DiscoverChoice: انتخاب کاربر
// می‌تونی در آینده فیلدهای بیشتر اضافه کنی (مثلاً payload های JSON)


};




#endif