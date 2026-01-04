#pragma once
#include<string>
#include<cstdint>

std::string create_match_json(int num_players);
void push_action_json(const std::string &match_id, int palyer_index,const std::string &action_json);
std::string start_combat_json(const std::string &match_id, uint32_t seed);
std::string get_state_json(const std::string &match_id);
void destroy_match(const std::string &match_id);