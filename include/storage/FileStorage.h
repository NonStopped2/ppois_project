#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace lecture2pdf {

// Owns exactly one generated directory. Keep alive until the bot sends its files.
// Non-copyable/non-movable: one owner controls cleanup. Not thread-safe.
class FileStorage {
public:
    explicit FileStorage(std::filesystem::path root = std::filesystem::temp_directory_path());
    ~FileStorage() noexcept;
    FileStorage(const FileStorage&) = delete;
    FileStorage& operator=(const FileStorage&) = delete;
    FileStorage(FileStorage&&) = delete;
    FileStorage& operator=(FileStorage&&) = delete;

    const std::filesystem::path& directory() const noexcept;
    std::filesystem::path importFile(const std::filesystem::path& source);
    // Returns an unused generated pathname; caller creates the actual file.
    // Extension must be . followed by 1..10 ASCII letters/digits (no user names).
    std::filesystem::path newPath(const std::string& extension);
    void cleanup(); // Idempotent. Unlike the destructor, reports filesystem errors.

private:
    std::filesystem::path directory_;
    std::uint64_t nextFile_ = 1;
    bool active_ = true;
};

} // namespace lecture2pdf
