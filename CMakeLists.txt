cmake_minimum_required(VERSION 3.14)
project(bgbinding)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for editors (optional but useful)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pybind11 REQUIRED)
find_package(nlohmann_json REQUIRED)   # اگر از nlohmann::json استفاده می‌کنید

# لیست فایل‌های منبعی که باید در ماژول لینک شوند
set(BG_SOURCES
    bindings.cpp
    cpp_api.cpp
    GameController.cpp
    GameState.cpp
    Board.cpp
    Shop.cpp
    # اگر فایل cpp دیگری داری که پیاده‌سازی تو را دارد اضافه کن، مثلاً:
    # Combat.cpp
    # Player.cpp
    # MinionPool.cpp
)

# Build the python extension module 'bgbinding'
pybind11_add_module(bgbinding ${BG_SOURCES})

# include project root so headers like "GameState.h" are found
target_include_directories(bgbinding PRIVATE ${CMAKE_SOURCE_DIR})

# link nlohmann_json if used
target_link_libraries(bgbinding PRIVATE nlohmann_json::nlohmann_json)
