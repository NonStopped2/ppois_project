#include "TelegramBot.h"

#include <cstdlib>
#include <iostream>

int main() {
  const char *token = std::getenv("TELEGRAM_BOT_TOKEN");

  if (token == nullptr) {
    std::cerr << "Ошибка: токен не найден!\n";
    return 1;
  }

  try {
    TelegramBot bot(token);
    bot.start();
  } catch (const std::exception &e) {
    std::cerr << "Ошибка работы бота: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
