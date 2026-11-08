// Diagnostic example only. The production OCR implementation belongs to Egor.
#include "pdf/PdfGenerator.h"
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
        lecture2pdf::Document document("Lecture2PDF");
        for (int index = 2; index < argc; ++index) {
            document.addPage(std::filesystem::u8path(argv[index]));
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
        lecture2pdf::PdfGenerator generator(backend);
        generator.generate(document, std::filesystem::u8path(argv[1]));
        std::cout << "Created " << argv[1] << " (" << document.pageCount() << " pages)\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
