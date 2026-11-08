#include "document/DocumentService.h"

#include <stdexcept>
#include <utility>

namespace lecture2pdf {

DocumentService::DocumentService(IPdfBackend& backend, std::string title,
                                 std::filesystem::path storageRoot)
    : document_(std::move(title)), storage_(std::move(storageRoot)), generator_(backend) {}

DocumentService::State DocumentService::state() const noexcept { return state_; }

void DocumentService::requireCollecting() const {
    if (state_ != State::Collecting) {
        throw std::logic_error("Lecture is closed or generation is already in progress");
    }
}

Document DocumentService::snapshot() const {
    requireCollecting();
    return document_;
}

SlideId DocumentService::addPage(const std::filesystem::path& source) {
    requireCollecting();
    auto local = storage_.importFile(source);
    try {
        return document_.addPage(local);
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(local, ignored);
        throw;
    }
}

void DocumentService::removePage(std::size_t index) {
    requireCollecting();
    document_.removePage(index);
    // Retain old files until close: existing document snapshots may refer to them.
}

void DocumentService::movePage(std::size_t fromIndex, std::size_t toIndex) {
    requireCollecting();
    document_.movePage(fromIndex, toIndex);
}

void DocumentService::rename(std::string title) {
    requireCollecting();
    document_.rename(std::move(title));
}

void DocumentService::replacePageImage(std::size_t index,
                                      const std::filesystem::path& source) {
    requireCollecting();
    (void)document_.page(index); // Check index before creating any file.
    auto local = storage_.importFile(source);
    try {
        document_.setImagePath(index, local);
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(local, ignored);
        throw;
    }
}

std::filesystem::path DocumentService::generatePdf() {
    requireCollecting();
    auto snapshot = document_;
    const auto output = storage_.newPath(".pdf");
    state_ = State::Generating;
    try {
        generator_.generate(snapshot, output);
    } catch (...) {
        state_ = State::Collecting;
        throw;
    }
    state_ = State::Collecting;
    return output;
}

void DocumentService::close() {
    if (state_ == State::Closed) {
        return;
    }
    requireCollecting();
    storage_.cleanup();
    state_ = State::Closed;
}

} // namespace lecture2pdf
