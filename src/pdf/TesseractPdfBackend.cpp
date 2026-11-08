#include "pdf/TesseractPdfBackend.h"

#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>

#include <stdexcept>
#include <utility>

namespace lecture2pdf {

TesseractPdfBackend::TesseractPdfBackend(tesseract::TessBaseAPI& engine,
                                       RecognizePage recognize)
    : engine_(engine), recognize_(std::move(recognize)) {
    if (!recognize_) {
        throw std::invalid_argument("An OCR callback is required");
    }
}

void TesseractPdfBackend::write(const Document& document,
                                const std::filesystem::path& outputPdf) {
    const auto* languages = engine_.GetInitLanguagesAsString();
    if (!languages || *languages == '\0') {
        throw std::logic_error("OCR engine must be initialized before PDF generation");
    }
    if (document.empty() || outputPdf.extension() != ".pdf") {
        throw std::invalid_argument("Need a non-empty document and a .pdf output path");
    }
    auto base = outputPdf;
    base.replace_extension();
    tesseract::TessPDFRenderer renderer(base.string().c_str(), engine_.GetDatapath(), false);
    if (!renderer.BeginDocument(document.title().c_str())) {
        throw std::runtime_error("Cannot begin PDF rendering");
    }
    for (const auto& slide : document.slides()) {
        engine_.Clear();
        recognize_(slide, engine_);
        if (!renderer.AddImage(&engine_)) {
            throw std::runtime_error("Cannot append slide to PDF");
        }
    }
    if (!renderer.EndDocument()) {
        throw std::runtime_error("Cannot finalize PDF");
    }
}

} // namespace lecture2pdf
