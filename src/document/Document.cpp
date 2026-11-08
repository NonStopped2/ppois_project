#include "document/Document.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lecture2pdf {
namespace {

void validateTitle(const std::string& title) {
    if (title.empty() || title.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument("Document title must not be blank");
    }
}

} // namespace

Document::Document(std::string title) : title_(std::move(title)) { validateTitle(title_); }

const std::string& Document::title() const noexcept { return title_; }

void Document::rename(std::string title) {
    validateTitle(title);
    title_ = std::move(title);
}

bool Document::empty() const noexcept { return slides_.empty(); }
std::size_t Document::pageCount() const noexcept { return slides_.size(); }
const std::vector<Slide>& Document::slides() const noexcept { return slides_; }
const Slide& Document::page(std::size_t index) const { return slides_.at(index); }

SlideId Document::addPage(std::filesystem::path imagePath) {
    if (nextId_ == std::numeric_limits<SlideId>::max()) {
        throw std::overflow_error("Slide ID space exhausted");
    }
    slides_.emplace_back(nextId_, std::move(imagePath));
    return nextId_++;
}

void Document::removePage(std::size_t index) {
    (void)slides_.at(index); // Validate before modifying the document.
    slides_.erase(slides_.begin() + static_cast<std::ptrdiff_t>(index));
}

void Document::movePage(std::size_t fromIndex, std::size_t toIndex) {
    (void)slides_.at(fromIndex);
    (void)slides_.at(toIndex);
    auto begin = slides_.begin();
    const auto from = static_cast<std::ptrdiff_t>(fromIndex);
    const auto to = static_cast<std::ptrdiff_t>(toIndex);
    if (from < to) {
        std::rotate(begin + from, begin + from + 1, begin + to + 1);
    } else if (from > to) {
        std::rotate(begin + to, begin + from, begin + from + 1);
    }
}

void Document::setImagePath(std::size_t index, std::filesystem::path imagePath) {
    auto& slide = slides_.at(index);
    slide = Slide(slide.id(), std::move(imagePath));
}

} // namespace lecture2pdf
