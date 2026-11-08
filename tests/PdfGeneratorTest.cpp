#include "pdf/PdfGenerator.h"
#include "storage/FileStorage.h"

#include <gtest/gtest.h>
#include <fstream>
#include <functional>
#include <stdexcept>

using namespace lecture2pdf;
namespace fs = std::filesystem;

namespace {

class FakeBackend final : public IPdfBackend {
public:
    std::vector<SlideId> received;
    fs::path stagingPath;
    bool fail = false;
    bool invalidHeader = false;
    bool omitOutput = false;
    std::function<void()> beforeReturn;

    void write(const Document& document, const fs::path& output) override {
        stagingPath = output;
        for (const auto& slide : document.slides()) {
            received.push_back(slide.id());
        }
        if (!omitOutput) {
            std::ofstream stream(output, std::ios::binary);
            stream << (invalidHeader ? "not a pdf" : "%PDF-1.7\nfake unit-test output\n%%EOF\n");
        }
        if (fail) {
            throw std::runtime_error("simulated renderer failure");
        }
        if (beforeReturn) {
            beforeReturn();
        }
    }
};

class PdfGeneratorTest : public ::testing::Test {
protected:
    FileStorage storage;
    FakeBackend backend;
    PdfGenerator generator{backend};
    Document document;

    void SetUp() override {
        const auto a = storage.newPath(".png");
        const auto b = storage.newPath(".png");
        std::ofstream(a) << "fake image";
        std::ofstream(b) << "fake image";
        document.addPage(a);
        document.addPage(b);
    }
};

} // namespace

TEST_F(PdfGeneratorTest, PreservesDocumentOrderAndCleansStaging) {
    document.movePage(0, 1);
    const auto output = storage.newPath(".pdf");
    generator.generate(document, output);
    EXPECT_EQ(backend.received, (std::vector<SlideId>{2, 1}));
    EXPECT_TRUE(fs::is_regular_file(output));
    EXPECT_FALSE(fs::exists(backend.stagingPath.parent_path()));
}

TEST_F(PdfGeneratorTest, RejectsEmptyDocumentBeforeBackend) {
    EXPECT_THROW(generator.generate(Document{}, storage.newPath(".pdf")), std::invalid_argument);
    EXPECT_TRUE(backend.received.empty());
}

TEST_F(PdfGeneratorTest, RejectsMissingImagesBeforeBackend) {
    document.setImagePath(1, storage.directory() / "missing.png");
    EXPECT_THROW(generator.generate(document, storage.newPath(".pdf")), std::invalid_argument);
    EXPECT_TRUE(backend.received.empty());
}

TEST_F(PdfGeneratorTest, RejectsInvalidOutputAndMissingParent) {
    EXPECT_THROW(generator.generate(document, storage.newPath(".txt")), std::invalid_argument);
    EXPECT_ANY_THROW(generator.generate(document, storage.directory() / "missing" / "a.pdf"));
    EXPECT_TRUE(backend.received.empty());
}

TEST_F(PdfGeneratorTest, DoesNotOverwriteExistingFile) {
    const auto output = storage.newPath(".pdf");
    std::ofstream(output) << "keep";
    EXPECT_THROW(generator.generate(document, output), std::invalid_argument);
    EXPECT_EQ(fs::file_size(output), 4u);
    EXPECT_TRUE(backend.received.empty());
}

TEST_F(PdfGeneratorTest, BackendFailureLeavesNoOutputOrStaging) {
    backend.fail = true;
    const auto output = storage.newPath(".pdf");
    EXPECT_THROW(generator.generate(document, output), std::runtime_error);
    EXPECT_FALSE(fs::exists(output));
    EXPECT_FALSE(fs::exists(backend.stagingPath.parent_path()));
    EXPECT_TRUE(fs::exists(document.page(0).imagePath()));
}

TEST_F(PdfGeneratorTest, InvalidHeaderIsNotPublished) {
    backend.invalidHeader = true;
    const auto output = storage.newPath(".pdf");
    EXPECT_THROW(generator.generate(document, output), std::runtime_error);
    EXPECT_FALSE(fs::exists(output));
    EXPECT_FALSE(fs::exists(backend.stagingPath.parent_path()));
}

TEST_F(PdfGeneratorTest, MissingBackendOutputIsNotPublished) {
    backend.omitOutput = true;
    const auto output = storage.newPath(".pdf");
    EXPECT_THROW(generator.generate(document, output), std::runtime_error);
    EXPECT_FALSE(fs::exists(output));
    EXPECT_FALSE(fs::exists(backend.stagingPath.parent_path()));
}

TEST_F(PdfGeneratorTest, ConcurrentDestinationCreationCannotOverwriteResult) {
    const auto output = storage.newPath(".pdf");
    backend.beforeReturn = [&output] { std::ofstream(output) << "another generation"; };
    EXPECT_THROW(generator.generate(document, output), fs::filesystem_error);
    EXPECT_EQ(fs::file_size(output), 18u);
    EXPECT_FALSE(fs::exists(backend.stagingPath.parent_path()));
}
