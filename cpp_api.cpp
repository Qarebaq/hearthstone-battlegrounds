// این فایل مهره کلیدی سرور هست 
//api ها رو اینجا داریم میسازیم

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

std::string create_match_json(int num_players) {
    Match *m = new Match();
    m->id = make_id();
    m->state = new GameState();

    // prepare players and shops and pendingActions vectors
    for (int i = 0; i < num_players; ++i) {
        Hero* h = new Hero("Sylvanas");
        Player* p = new Player("Player" + std::to_string(i+1), h);
        m->state->players.push_back(p);
        m->state->shops.push_back(new Shop());
        m->state->pendingActions.emplace_back();
        m->state->discoverOffers.emplace_back();
        m->state->discoverPending.push_back(false);
    }

    m->controller = new GameController();

    // run the controller in a separate thread
    m->running = true;
    m->thread = std::thread([m]() {
        try {
            m->controller->run(*m->state);
        } catch (const std::exception &e) {
            std::cerr << "Match thread exception: " << e.what() << std::endl;
        }
        m->running = false;
    });

    {
        std::lock_guard<std::mutex> lg(matches_mtx);
        matches[m->id] = m;
    }

    json out;
    out["match_id"] = m->id;
    out["num_players"] = num_players;
    return out.dump();
}

void push_action_json(const std::string &match_id, int player_index, const std::string &action_json) {
    std::lock_guard<std::mutex> lg(matches_mtx);
    auto it = matches.find(match_id);
    if (it == matches.end()) return;
    Match* mm = it->second;
    if (!mm || !mm->state) return;

    json j;
    try {
        j = json::parse(action_json);
    } catch(...) {
        return;
    }
    Action a = action_from_json(j);

    // bounds check for player index
    if (player_index < 0 || player_index >= (int)mm->state->players.size()) return;

    {
        std::lock_guard<std::mutex> lg2(mm->mtx);
        mm->state->pushAction(player_index, a);
        // notify waiting controller
        mm->state->actionCv.notify_all();
    }
}



std::string get_state_json(const std::string &match_id) {
    std::lock_guard<std::mutex> lg(matches_mtx);
    auto it = matches.find(match_id);
    if (it == matches.end()) return "{}";
    Match* mm = it->second;
    if (!mm || !mm->state) return "{}";

    json out;
    out["round"] = mm->state->round;
    out["phase"] = mm->state->phase == Phase::Buy ? "Buy" : "Combat";

    json players = json::array();
    for (size_t i = 0; i < mm->state->players.size(); ++i) {
        Player* p = mm->state->players[i];
        json pj;
        pj["name"] = p->name;
        pj["gold"] = p->gold;
        pj["hero"] = { {"name", p->hero->name}, {"health", p->hero->health} };
        // board
        json board = json::array();
        for (auto mptr : p->board.minions) {
            board.push_back(minion_to_json(mptr));
        }
        pj["board"] = board;
        players.push_back(pj);
    }
    out["players"] = players;

    // shops
    json shops = json::array();
    for (size_t i = 0; i < mm->state->shops.size(); ++i) {
        Shop* s = mm->state->shops[i];
        json sj;
        sj["tavernTier"] = s->tavernTier;
        // shop slots
        json slots = json::array();
        for (int k = 0; k < s->slots.size(); ++k) {
            Minion* mptr = s->slots[k];
            if (mptr) slots.push_back(minion_to_json(mptr));
            else slots.push_back(nullptr);
        }
        sj["slots"] = slots;
        shops.push_back(sj);
    }
    out["shops"] = shops;

    return out.dump();
}

std::string start_combat_json(const std::string &match_id, uint32_t seed) {
    // For now controller.run() runs in match thread and will perform combats.
    // This helper returns state; future: trigger a forced combat and return replay JSON.
    return get_state_json(match_id);
}
