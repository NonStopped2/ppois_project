#include "storage/FileStorage.h"

#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

namespace lecture2pdf {
namespace {

std::string randomName() {
    std::random_device random;
    std::ostringstream name;
    name << "lecture2pdf-" << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i) {
        name << std::setw(8) << random();
    }
    return name.str();
}

bool validExtension(const std::string& extension) {
    if (extension.size() < 2 || extension.size() > 11 || extension.front() != '.') {
        return false;
    }
    for (std::size_t i = 1; i < extension.size(); ++i) {
        const auto c = extension[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

} // namespace

FileStorage::FileStorage(std::filesystem::path root) {
    root = std::filesystem::canonical(root);
    if (!std::filesystem::is_directory(root)) {
        throw std::invalid_argument("Storage root must be an existing directory");
    }
    for (int attempt = 0; attempt < 32; ++attempt) {
        auto candidate = root / randomName();
        if (std::filesystem::create_directory(candidate)) {
            try {
                std::filesystem::permissions(candidate, std::filesystem::perms::owner_all);
            } catch (...) {
                std::error_code ignored;
                std::filesystem::remove(candidate, ignored);
                throw;
            }
            directory_ = std::move(candidate);
            return;
        }
    }
    throw std::runtime_error("Could not allocate a unique storage directory");
}

FileStorage::~FileStorage() noexcept {
    if (active_) {
        std::error_code ignored;
        std::filesystem::remove_all(directory_, ignored);
    }
}

const std::filesystem::path& FileStorage::directory() const noexcept { return directory_; }

std::filesystem::path FileStorage::newPath(const std::string& extension) {
    if (!active_) {
        throw std::logic_error("Storage has already been cleaned up");
    }
    if (!validExtension(extension)) {
        throw std::invalid_argument("Invalid extension; use e.g. .png or .pdf");
    }
    while (nextFile_ != std::numeric_limits<std::uint64_t>::max()) {
        auto path = directory_ / ("file-" + std::to_string(nextFile_++) + extension);
        if (!std::filesystem::exists(std::filesystem::symlink_status(path))) {
            return path;
        }
    }
    throw std::overflow_error("Storage filename space exhausted");
}

std::filesystem::path FileStorage::importFile(const std::filesystem::path& source) {
    if (!active_) {
        throw std::logic_error("Storage has already been cleaned up");
    }
    if (!std::filesystem::is_regular_file(std::filesystem::symlink_status(source))) {
        throw std::invalid_argument("Source must be a regular file, not a symlink");
    }
    const auto extension = source.extension().string();
    const auto destination = newPath(validExtension(extension) ? extension : ".bin");
    try {
        std::filesystem::copy_file(source, destination); // Never overwrite another file.
    } catch (const std::filesystem::filesystem_error& error) {
        std::error_code ignored;
        if (error.code() != std::errc::file_exists) {
            std::filesystem::remove(destination, ignored);
        }
        throw;
    }
    return destination;
}

void FileStorage::cleanup() {
    if (active_) {
        std::filesystem::remove_all(directory_);
        active_ = false;
    }
}

} // namespace lecture2pdf
