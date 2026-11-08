#include "document/Slide.h"

#include <stdexcept>
#include <utility>

namespace lecture2pdf {

Slide::Slide(SlideId id, std::filesystem::path imagePath)
    : id_(id), imagePath_(std::move(imagePath)) {
    if (id_ == 0 || imagePath_.empty()) {
        throw std::invalid_argument("Slide needs a non-zero ID and an image path");
    }
}

SlideId Slide::id() const noexcept { return id_; }

const std::filesystem::path& Slide::imagePath() const noexcept { return imagePath_; }

} // namespace lecture2pdf
