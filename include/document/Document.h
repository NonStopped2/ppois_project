#pragma once

#include "document/Slide.h"

#include <string>
#include <vector>

namespace lecture2pdf {

// Document has no dependency on Telegram, storage, OCR or PDF libraries.
// Indices are zero-based; IDs are stable and never reused within a document.
class Document {
public:
    explicit Document(std::string title = "Lecture");

    const std::string& title() const noexcept;
    void rename(std::string title);
    bool empty() const noexcept;
    std::size_t pageCount() const noexcept;
    const std::vector<Slide>& slides() const noexcept;
    const Slide& page(std::size_t index) const;

    SlideId addPage(std::filesystem::path imagePath);
    void removePage(std::size_t index);
    // The page ends up at toIndex in the resulting document (not an insertion gap).
    void movePage(std::size_t fromIndex, std::size_t toIndex);
    void setImagePath(std::size_t index, std::filesystem::path imagePath);

private:
    std::string title_;
    std::vector<Slide> slides_;
    SlideId nextId_ = 1;
};

} // namespace lecture2pdf
