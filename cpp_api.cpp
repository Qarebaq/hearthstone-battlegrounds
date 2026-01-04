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

static Action action_from_json(const json &j) {
    Action a;
    std::string t = "";
    if (j.contains("type") && j["type"].is_string()) {
        t = j["type"].get<std::string>();
    }
    // normalize to upper
    for (auto &c : t) c = toupper((unsigned char)c);

    if (t == "BUY") a.type = ActionType::Buy;
    else if (t == "ROLL") a.type = ActionType::Roll;
    else if (t == "TOGGLEFREEZE") a.type = ActionType::ToggleFreeze;
    else if (t == "SELL") a.type = ActionType::Sell;
    else if (t == "UPGRADE") a.type = ActionType::Upgrade;
    else if (t == "HERO" || t == "HEROPOWER") a.type = ActionType::HeroPower;
    else if (t == "DISCOVER" || t == "DISCOVERCHOICE") a.type = ActionType::DiscoverChoice;
    else if (t == "END" || t == "ENDTURN") a.type = ActionType::EndTurn;
    else a.type = ActionType::EndTurn; // default

    if (j.contains("slot") && j["slot"].is_number_integer()) {
        a.slotIndex = j["slot"].get<int>();
    } else if (j.contains("slotIndex") && j["slotIndex"].is_number_integer()) {
        a.slotIndex = j["slotIndex"].get<int>();
    }

    if (j.contains("choice") && j["choice"].is_number_integer()) {
        a.choice = j["choice"].get<int>();
    }
    return a;
}




static json minion_to_json(Minion *m) {
    json mj;
    if (!m) return mj;
    mj["name"] = m->name;
    mj["attack"] = m->attack;
    mj["health"] = m->health;
    mj["tier"] = m->tier;
    mj["divineShield"] = m->divineShield;
    mj["poisonous"] = m->poisonous;
    mj["reborn"] = m->reborn;
    mj["deathrattle"] = m->deathrattle;
    return mj;
}

