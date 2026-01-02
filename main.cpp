// main.cpp -- interactive runner that sends actions to active player
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <chrono>

#include "GameState.h"
#include "GameController.h"
#include "Action.h"

int main() {
    std::ios::sync_with_stdio(true);

    GameState state;
    GameController controller;

    // Run game loop in a background thread
    std::thread game_thread([&controller, &state]() {
        controller.run(state);
    });

    std::cout << "Interactive mode ready. Type 'help' for commands.\n";

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            // EOF or input error
            break;
        }

        // trim
        auto l = line.find_first_not_of(" \t\r\n");
        if (l == std::string::npos) continue;
        auto r = line.find_last_not_of(" \t\r\n");
        std::string cmdline = line.substr(l, r - l + 1);

        std::istringstream iss(cmdline);
        std::string cmd;
        iss >> cmd;

        if (cmd == "help") {
            std::cout << "Commands:\n"
                      << "  buy <slot>       -- buy minion from shop slot (slot 0..)\n"
                      << "  roll             -- refresh shop (costs 1 gold)\n"
                      << "  freeze           -- toggle freeze\n"
                      << "  sell <idx>       -- sell minion on your board index\n"
                      << "  upgrade          -- upgrade tavern\n"
                      << "  hero <idx>       -- use hero power target (board idx)\n"
                      << "  discover <n>     -- choose discover option n (0..2)\n"
                      << "  end              -- end your turn\n"
                      << "  who              -- show active player (for debugging)\n"
                      << "  help             -- show this help\n"
                      << "  quit             -- quit program\n";
            continue;
        } else if (cmd == "quit") {
            std::cout << "Quitting...\n";
            break;
        } else if (cmd == "who") {
            int active = state.activePlayerIndex.load();
            if (active < 0) {
                std::cout << "No active player right now.\n";
            } else if (active < (int)state.players.size()) {
                std::cout << "Active player: Player" << (active+1)
                          << " (" << state.players[active]->name
                          << "), Hero: " << state.players[active]->hero->name
                          << ", Gold: " << state.players[active]->gold << "\n";
            } else {
                std::cout << "Active index out of range: " << active << "\n";
            }
            continue;
        }

        // For action commands we need an active player
        int target = state.activePlayerIndex.load();
        if (target < 0) {
            std::cout << "No active player right now. Wait for your turn.\n";
            continue;
        }
        if (target >= (int)state.players.size()) {
            std::cout << "Invalid active player index: " << target << "\n";
            continue;
        }

        // helper: print which player we're sending for
        std::cout << "Sending action for Player" << (target+1)
                  << " (" << state.players[target]->name << ")\n";

        if (cmd == "buy") {
            int idx; if (!(iss >> idx)) { std::cout << "usage: buy <slot>\n"; continue; }
            Action a; a.type = ActionType::Buy; a.slotIndex = idx;
            state.pushAction(target, a);
        } else if (cmd == "roll") {
            Action a; a.type = ActionType::Roll; state.pushAction(target, a);
        } else if (cmd == "freeze") {
            Action a; a.type = ActionType::ToggleFreeze; state.pushAction(target, a);
        } else if (cmd == "sell") {
            int idx; if (!(iss >> idx)) { std::cout << "usage: sell <index>\n"; continue; }
            Action a; a.type = ActionType::Sell; a.slotIndex = idx;
            state.pushAction(target, a);
        } else if (cmd == "upgrade") {
            Action a; a.type = ActionType::Upgrade; state.pushAction(target, a);
        } else if (cmd == "hero") {
            int idx; if (!(iss >> idx)) { std::cout << "usage: hero <index>\n"; continue; }
            Action a; a.type = ActionType::HeroPower; a.slotIndex = idx;
            state.pushAction(target, a);
        } else if (cmd == "discover") {
            int choice; if (!(iss >> choice)) { std::cout << "usage: discover <choice>\n"; continue; }
            Action a; a.type = ActionType::DiscoverChoice; a.choice = choice;
            state.pushAction(target, a);
        } else if (cmd == "end") {
            Action a; a.type = ActionType::EndTurn; state.pushAction(target, a);
        } else {
            std::cout << "Unknown command. Type 'help'\n";
            continue;
        }
    } // end input loop

    // we will detach the game thread so the process can exit cleanly
    if (game_thread.joinable()) {
        game_thread.detach();
    }

    return 0;
}
