#include <Python.h>
#include<pybind11/pybind11.h>
#include<string>



namespace py = pybind11;

std::string hello(){
    return "hello from C++ bgbinding";

}

PYBIND11_MODULE(bgbinding, m) {
    m.doc() = "test binding for bgbinding";
    m.def("hello", &hello, "Return a hello string from C++");
}