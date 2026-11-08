#pragma once

#include "document/Document.h"
#include "pdf/PdfGenerator.h"
#include "storage/FileStorage.h"

namespace lecture2pdf {

// One lecture, not one global bot. The caller serializes access and keeps this
// object alive until all processing and file uploads finish. No worker threads.
class DocumentService {
public:
    enum class State { Collecting, Generating, Closed };

    explicit DocumentService(
        IPdfBackend& backend, std::string title = "Lecture",
        std::filesystem::path storageRoot = std::filesystem::temp_directory_path());

    State state() const noexcept;
    Document snapshot() const;
    SlideId addPage(const std::filesystem::path& source);
    void removePage(std::size_t index);
    void movePage(std::size_t fromIndex, std::size_t toIndex);
    void rename(std::string title);
    // Takes a copy of an already prepared image; does not preprocess it.
    void replacePageImage(std::size_t index, const std::filesystem::path& source);

    // Synchronous. Each successful call returns a new file, retained until close.
    // Failure restores Collecting so the user may correct input and retry.
    std::filesystem::path generatePdf();
    // Use for cancellation or after upload completion. Idempotent; disallowed
    // while Generating. Destructor also performs best-effort cleanup via storage.
    void close();

private:
    void requireCollecting() const;

    Document document_;
    FileStorage storage_;
    PdfGenerator generator_;
    State state_ = State::Collecting;
};

} // namespace lecture2pdf
