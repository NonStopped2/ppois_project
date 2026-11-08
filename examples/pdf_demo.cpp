// Diagnostic example only. The production OCR implementation belongs to Egor.
#include "document/DocumentService.h"
#include "pdf/TesseractPdfBackend.h"

#include <allheaders.h>
#include <tesseract/baseapi.h>

#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
struct PixDeleter {
    void operator()(PIX* image) const { pixDestroy(&image); }
};
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: lecture2pdf_pdf_demo output.pdf image1 [image2 ...]\n";
        return 1;
    }
    try {
        const auto destination = std::filesystem::absolute(std::filesystem::u8path(argv[1]));
        if (destination.extension() != ".pdf" ||
            std::filesystem::exists(std::filesystem::symlink_status(destination))) {
            throw std::invalid_argument("Need a new output filename ending in .pdf");
        }
        tesseract::TessBaseAPI engine;
        if (engine.Init(nullptr, "rus+eng") != 0) {
            throw std::runtime_error("Cannot initialize rus+eng; check tessdata");
        }
        lecture2pdf::TesseractPdfBackend backend(engine,
            [](const lecture2pdf::Slide& slide, tesseract::TessBaseAPI& api) {
                std::unique_ptr<PIX, PixDeleter> image(pixRead(slide.imagePath().string().c_str()));
                if (!image) {
                    throw std::runtime_error("Cannot decode image: " + slide.imagePath().string());
                }
                api.SetImage(image.get()); // Tesseract retains its own reference/copy.
                if (api.Recognize(nullptr) != 0) {
                    throw std::runtime_error("OCR failed");
                }
            });
        // Same filesystem as the destination, so the exclusive export can use a
        // hard link. The service owns all copies until this scope ends.
        lecture2pdf::DocumentService service(backend, "Lecture2PDF", destination.parent_path());
        for (int index = 2; index < argc; ++index) {
            service.addPage(std::filesystem::u8path(argv[index]));
        }
        const auto localPdf = service.generatePdf();
        std::filesystem::create_hard_link(localPdf, destination);
        std::cout << "Created " << argv[1] << " (" << service.snapshot().pageCount() << " pages)\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
