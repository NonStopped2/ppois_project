#include "TelegramBot.h"

#include <iostream>
#include <tgbot/tgbot.h>

TelegramBot::TelegramBot(const std::string &token) : token_(token) {}

void TelegramBot::start() {
  TgBot::Bot bot(token_);

  bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
    bot.getApi().sendMessage(message->chat->id,
                             "Привет! Я бот для создания PDF-документов.");
  });
  bot.getEvents().onCommand("r", [&bot](TgBot::Message::Ptr message) {
    bot.getApi().sendMessage(message->chat->id, "Привет!");
  });
  bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
    if (!message->photo || message->photo->empty()) {
      return;
    }

    const auto &photo = message->photo->back();

    std::cout << "Получена фотография!\n";
    std::cout << "File ID: " << photo->fileId << '\n';
    std::cout << "Размер: " << photo->width << "x" << photo->height << '\n';

    bot.getApi().sendMessage(
        message->chat->id, "Фотография получена! Можете отправить следующую.");
  });
  std::cout << "Bot started!" << std::endl;

  TgBot::TgLongPoll longPoll(bot);
  longPoll.startLoop();
}
