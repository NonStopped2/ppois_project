#pragma once

#include "pdf/IPdfBackend.h"

#include <functional>

namespace tesseract {
class TessBaseAPI;
}

namespace lecture2pdf {

// Bridge to Egor's OCR module: it initializes the engine and supplies recognize.
// recognize must SetImage and complete Recognize for this slide on this engine.
// Image geometry must match the image that is to appear in the PDF.
class TesseractPdfBackend final : public IPdfBackend {
public:
    using RecognizePage = std::function<void(const Slide&, tesseract::TessBaseAPI&)>;

    TesseractPdfBackend(tesseract::TessBaseAPI& engine, RecognizePage recognize);
    void write(const Document& document, const std::filesystem::path& outputPdf) override;

private:
    tesseract::TessBaseAPI& engine_;
    RecognizePage recognize_;
};

} // namespace lecture2pdf
