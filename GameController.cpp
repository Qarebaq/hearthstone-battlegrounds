#include "GameController.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

void GameController::buyPhase(Player &p, Shop &shop) {

    // شروع فاز خرید (دقیقاً مثل قبل)
    shop.roll();
    auto start = steady_clock::now();

    const int TIMER = 120; // پسرک تایمر رو اینجا ست کردی حواست باشه

    while (true) {
        auto now = steady_clock::now();
        int elapsed = duration_cast<seconds>(now - start).count();

        // پایان زمان
        if (elapsed >= TIMER) {
            cout << "\nTime's Up!\n";
            break;
        }

        // نمایش فروشگاه
        shop.show();

        cout << "\nGold: " << p.gold
             << " | Time left: " << (TIMER - elapsed) << "s\n";

        cout << "0=Buy 1=Roll 2=Toggle Freeze 3=Sell 4=Upgrade 9=End\n";
        cout << "Enter command: ";

        int cmd;
        cin >> cmd;

        if (cmd == 0) {
            cout << "Enter slot index: ";
            int i;
            cin >> i;
            shop.buy(p, i);
        }
        else if (cmd == 1 && p.gold >= 1) {
            p.gold--;
            shop.roll();
        }
        else if (cmd == 2) {
            shop.toggleFreeze(p);
        }
        else if (cmd == 3) {
            cout << "Enter minion index to sell: ";
            int i;
            cin >> i;
            shop.sell(p, i);
        }
        else if (cmd == 4) {
            shop.upgrade(p);
        }
        else if (cmd == 9) {
            break;
        }
        else {
            cout << "Invalid command or not enough gold\n";
        }
    }

    // پایان فاز خرید
    if (shop.frozen) {
        cout << "\nShop will remain frozen for next turn\n";
    }
}
