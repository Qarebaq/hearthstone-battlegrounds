#include"GameState.h"
#include "HeroPool.h"
#include<iostream>

GameState::GameState(){
    round =1;
    phase = Phase::Buy;



    std::vector<Hero*> heroes = HeroPool::getHeroes();

    int numPlayers = 4;  //تعداد کل بازیکنانی که قراره تو لابی باشن

    for(int i=0; i< numPlayers; i++){
        std::cout<<"Player "<<(i+1)<<"choose your hero:\n";
        for(int j=0;j<heroes.size(); ++j){
            std::cout<<j<<") "<<heroes[j]->name<<std::endl;
        }
        int choice;
        std::cin>>choice;
        if (choice < 0 || choice >= (int)heroes.size()) {
            choice = 0;
        }

        Player* player = new Player("Player " + std::to_string(i + 1), heroes[choice]);
        players.push_back(player);
        heroes.erase(heroes.begin() + choice);
        Shop* shop = new Shop();
        shops.push_back(shop);
    }

    // پسر حواست باشه اینجا میتونی HERO ها رو اضافه کنی

    // Hero *hero1 = new Hero("Hero 1");
    // Hero *hero2 = new Hero("Hero 2");


    //پسرک حواست باشه اینجا بازیکن ها رو اضافه میکنی ها!

    // Player *player1 = new Player("Player 1", hero1);
    // Player *player2 = new Player("Player 2" ,hero2);

    // players.push_back(player1);// حواسته که این وکتور بود دیگه
    // players.push_back(player2);

    // حواست باشه پسرک اینجا داریم برای هر بازیکن shop ایجاد میکنم

    // Shop *shop1 = new Shop();
    // Shop *shop2= new Shop();


    // shops.push_back(shop1);
    // shops.push_back(shop2);// حواسته که این وکتور بود دیگه

}