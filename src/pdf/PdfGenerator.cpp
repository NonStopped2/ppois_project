#include "pdf/PdfGenerator.h"
#include "storage/FileStorage.h"

#include <array>
#include <fstream>
#include <stdexcept>

namespace lecture2pdf {

PdfGenerator::PdfGenerator(IPdfBackend& backend) : backend_(backend) {}

void PdfGenerator::generate(const Document& document,
                            const std::filesystem::path& outputPdf) {
    if (document.empty()) {
        throw std::invalid_argument("Cannot generate an empty document");
    }
    if (outputPdf.extension() != ".pdf") {
        throw std::invalid_argument("Output filename must end with .pdf");
    }
    const auto destination = std::filesystem::absolute(outputPdf).lexically_normal();
    if (std::filesystem::exists(std::filesystem::symlink_status(destination))) {
        throw std::invalid_argument("Output already exists; choose a new filename");
    }
    for (const auto& slide : document.slides()) {
        if (!std::filesystem::is_regular_file(slide.imagePath())) {
            throw std::invalid_argument("Missing image: " + slide.imagePath().string());
        }
    }

    FileStorage staging(destination.parent_path());
    const auto temporary = staging.newPath(".pdf");
    backend_.write(document, temporary);

    if (!std::filesystem::is_regular_file(std::filesystem::symlink_status(temporary))) {
        throw std::runtime_error("PDF backend did not produce a regular file");
    }
    // A cheap sanity check, not a full PDF validator.
    std::ifstream stream(temporary, std::ios::binary);
    std::array<char, 5> signature{};
    stream.read(signature.data(), static_cast<std::streamsize>(signature.size()));
    if (!stream || std::string(signature.data(), signature.size()) != "%PDF-") {
        throw std::runtime_error("PDF backend produced an invalid header");
    }
    stream.close();

    // Unlike rename on POSIX, this never replaces an existing file, even if
    // another generation created it after the initial existence check.
    // Unsupported filesystems report an error; there is no unsafe fallback.
    std::filesystem::create_hard_link(temporary, destination);
    // Staging's destructor removes only its directory; destination keeps its link.
}

} // namespace lecture2pdf
