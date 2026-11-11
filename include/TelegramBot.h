#pragma once
#include <string>
class TelegramBot {
private:
  std::string token_;
  void handleUpdate(/* Update */);

public:
  explicit TelegramBot(const std::string &token);
  void sendMessage(int64_t chatId, const std::string &message);
  void sendDocument(int64_t chatId, const std::string &filePath);
  void start();
  void stop();
};
class CommandHandler {
public:
  void handleStart(int64_t chatId);
  void handleNew(int64_t chatId);
  void handleGenerate(int64_t chatId);
  void handleCancel(int64_t chatId);

private:
};
