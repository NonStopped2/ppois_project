cmake_minimum_required(VERSION 3.16)

project(TelegramBot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(TgBot CONFIG REQUIRED)

add_executable(telegram_bot
    src/main.cpp
    src/TelegramBot.cpp
)
target_include_directories(telegram_bot 
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(telegram_bot
    PRIVATE TgBot::TgBot
)
