#include <Python.h>
#include<pybind11/pybind11.h>
#include<string>
#include "cpp_api.h"


namespace py = pybind11;

std::string hello(){
    return "hello from C++ bgbinding";

}

PYBIND11_MODULE(bgbinding, m) {
    // m.doc() = "test binding for bgbinding";
    // m.def("hello", &hello, "Return a hello string from C++");
    m.doc() = "Battlegrounds C++ bindings (PoC)";
    m.def("create_match_json", &create_match_json, "Create a match and return match JSON");
    m.def("push_action_json", &push_action_json, "Push action JSON for a player");
    m.def("get_state_json", &get_state_json, "Get state JSON for a match");
    m.def("start_combat_json", &start_combat_json, "Start combat (PoC) and return JSON");
    m.def("destroy_match", &destroy_match, "Destroy match");

}