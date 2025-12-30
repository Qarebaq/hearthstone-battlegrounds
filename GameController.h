#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H
#include "Shop.h"
#include "Player.h"
#include"GameState.h"
#include <random>
#include <chrono>
#include <cstdint>


class GameController{
    public:


        uint32_t seed;
        std::mt19937 rng;

        GameController();

        void run(GameState &state);
        void combatPhase(GameState &state);
        void buyPhase(Player &p, Shop &shop);
    
};


#endif
