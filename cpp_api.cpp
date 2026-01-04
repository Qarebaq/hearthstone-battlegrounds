#include "cpp_api.h"
#include "GameState.h"
#include "GameController.h"
#include "Action.h"
#include "Player.h"
#include "Hero.h"
#include "Shop.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

struct Match{
    std::string id;
    GameState *state = nullptr;
    GameController *controller = nullptr;
    std::thread thread;
    std::mutex mtx;
    bool running = false;
    
};

static std::unordered_map<std::string, Match*> matches;
static std::mutex matches_mtx;

static std::string make_id() {// اینجا دارم UUID میسازم 
    static std::mt19937 rng((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint32_t> d(0, 0xffffffffu);
    std::ostringstream ss;
    ss << std::hex << d(rng);
    return ss.str();
}