#pragma once

#include <cstdint>
#include <filesystem>

namespace lecture2pdf {

using SlideId = std::uint64_t;

// A value object; it does not read, process or delete its image file.
class Slide {
public:
    Slide(SlideId id, std::filesystem::path imagePath);

    SlideId id() const noexcept;
    const std::filesystem::path& imagePath() const noexcept;

private:
    SlideId id_;
    std::filesystem::path imagePath_;
};

} // namespace lecture2pdf
