#include "document/DocumentService.h"

#include <gtest/gtest.h>
#include <fstream>
#include <functional>
#include <stdexcept>

using namespace lecture2pdf;
namespace fs = std::filesystem;

namespace {

class ServiceBackend final : public IPdfBackend {
public:
    std::vector<SlideId> order;
    fs::path staging;
    std::function<void()> onWrite;
    bool fail = false;
    std::string title;

    void write(const Document& document, const fs::path& output) override {
        order.clear();
        title = document.title();
        staging = output.parent_path();
        for (const auto& slide : document.slides()) {
            order.push_back(slide.id());
        }
        if (onWrite) {
            onWrite();
        }
        std::ofstream(output) << "%PDF-1.7\nunit test\n%%EOF\n";
        if (fail) {
            throw std::runtime_error("simulated OCR failure");
        }
    }
};

class DocumentServiceTest : public ::testing::Test {
protected:
    FileStorage inputs;
    ServiceBackend backend;
    fs::path a;
    fs::path b;

    void SetUp() override {
        a = inputs.newPath(".png");
        b = inputs.newPath(".png");
        std::ofstream(a) << "first image";
        std::ofstream(b) << "second image";
    }
};

} // namespace

TEST_F(DocumentServiceTest, ImportsImagesAndGeneratesInEditedOrder) {
    DocumentService service(backend);
    const auto first = service.addPage(a);
    const auto second = service.addPage(b);
    service.rename("Lecture 2");
    service.movePage(0, 1);
    auto snapshot = service.snapshot();
    EXPECT_NE(snapshot.page(0).imagePath(), b);
    EXPECT_EQ(fs::file_size(snapshot.page(0).imagePath()), fs::file_size(b));
    const auto output = service.generatePdf();
    EXPECT_TRUE(fs::exists(output));
    EXPECT_EQ(backend.title, "Lecture 2");
    EXPECT_EQ(backend.order, (std::vector<SlideId>{second, first}));
    EXPECT_EQ(service.state(), DocumentService::State::Collecting);
    EXPECT_FALSE(fs::exists(backend.staging));
}

TEST_F(DocumentServiceTest, CloseRemovesCopiesAndResultsButNotOriginals) {
    DocumentService service(backend);
    service.addPage(a);
    const auto copy = service.snapshot().page(0).imagePath();
    const auto output = service.generatePdf();
    service.close();
    service.close();
    EXPECT_EQ(service.state(), DocumentService::State::Closed);
    EXPECT_FALSE(fs::exists(copy));
    EXPECT_FALSE(fs::exists(output));
    EXPECT_TRUE(fs::exists(a));
}

TEST_F(DocumentServiceTest, DestructorRetainsOutputUntilEndOfScope) {
    fs::path output;
    {
        DocumentService service(backend);
        service.addPage(a);
        output = service.generatePdf();
        EXPECT_TRUE(fs::exists(output));
    }
    EXPECT_FALSE(fs::exists(output.parent_path()));
}

TEST_F(DocumentServiceTest, FailedGenerationCanBeRetried) {
    DocumentService service(backend);
    service.addPage(a);
    const auto directory = service.snapshot().page(0).imagePath().parent_path();
    backend.fail = true;
    EXPECT_THROW(service.generatePdf(), std::runtime_error);
    EXPECT_EQ(service.state(), DocumentService::State::Collecting);
    EXPECT_FALSE(fs::exists(backend.staging));
    EXPECT_EQ(std::distance(fs::directory_iterator(directory), fs::directory_iterator{}), 1);
    backend.fail = false;
    EXPECT_TRUE(fs::exists(service.generatePdf()));
}

TEST_F(DocumentServiceTest, EmptyGenerationAndMissingUploadDoNotBreakSession) {
    DocumentService service(backend);
    EXPECT_THROW(service.generatePdf(), std::invalid_argument);
    EXPECT_EQ(service.state(), DocumentService::State::Collecting);
    EXPECT_ANY_THROW(service.addPage(inputs.directory() / "missing.png"));
    EXPECT_TRUE(service.snapshot().empty());
    EXPECT_EQ(service.addPage(a), 1u);
}

TEST_F(DocumentServiceTest, TwoLecturesHaveIndependentFilesAndDocuments) {
    DocumentService first(backend, "First");
    DocumentService second(backend, "Second");
    first.addPage(a);
    second.addPage(a);
    const auto firstPath = first.snapshot().page(0).imagePath();
    const auto secondPath = second.snapshot().page(0).imagePath();
    EXPECT_NE(firstPath.parent_path(), secondPath.parent_path());
    first.close();
    EXPECT_TRUE(fs::exists(secondPath));
    EXPECT_EQ(second.snapshot().title(), "Second");
    EXPECT_TRUE(fs::exists(second.generatePdf()));
}

TEST_F(DocumentServiceTest, RemovedAndReplacedFilesLiveUntilCloseForSnapshots) {
    DocumentService service(backend);
    const auto id = service.addPage(a);
    const auto previous = service.snapshot();
    service.replacePageImage(0, b);
    EXPECT_EQ(service.snapshot().page(0).id(), id);
    EXPECT_NE(service.snapshot().page(0).imagePath(), previous.page(0).imagePath());
    EXPECT_TRUE(fs::exists(previous.page(0).imagePath()));
    service.removePage(0);
    EXPECT_TRUE(service.snapshot().empty());
    EXPECT_TRUE(fs::exists(previous.page(0).imagePath()));
    service.close();
    EXPECT_FALSE(fs::exists(previous.page(0).imagePath()));
}

TEST_F(DocumentServiceTest, InvalidReplacementDoesNotCopyFilesOrChangeDocument) {
    DocumentService service(backend);
    service.addPage(a);
    const auto original = service.snapshot().page(0).imagePath();
    EXPECT_THROW(service.replacePageImage(1, b), std::out_of_range);
    EXPECT_ANY_THROW(service.replacePageImage(0, inputs.directory() / "missing.png"));
    EXPECT_EQ(service.snapshot().page(0).imagePath(), original);
    const auto root = original.parent_path();
    EXPECT_EQ(std::distance(fs::directory_iterator(root), fs::directory_iterator{}), 1);
}

TEST_F(DocumentServiceTest, RepeatedGenerationKeepsBothResultsUntilUploadCompletes) {
    DocumentService service(backend);
    service.addPage(a);
    const auto first = service.generatePdf();
    service.addPage(b);
    const auto second = service.generatePdf();
    EXPECT_NE(first, second);
    EXPECT_TRUE(fs::exists(first));
    EXPECT_TRUE(fs::exists(second));
    EXPECT_EQ(backend.order.size(), 2u);
}

TEST_F(DocumentServiceTest, ClosedLectureRejectsOperations) {
    DocumentService service(backend);
    service.close();
    EXPECT_THROW(service.addPage(a), std::logic_error);
    EXPECT_THROW(service.removePage(0), std::logic_error);
    EXPECT_THROW(service.movePage(0, 0), std::logic_error);
    EXPECT_THROW(service.replacePageImage(0, b), std::logic_error);
    EXPECT_THROW(service.rename("New"), std::logic_error);
    EXPECT_THROW(service.snapshot(), std::logic_error);
    EXPECT_THROW(service.generatePdf(), std::logic_error);
}

TEST_F(DocumentServiceTest, ReentrantChangesDuringGenerationAreRejected) {
    DocumentService service(backend);
    service.addPage(a);
    backend.onWrite = [&] {
        EXPECT_EQ(service.state(), DocumentService::State::Generating);
        EXPECT_THROW(service.addPage(b), std::logic_error);
        EXPECT_THROW(service.removePage(0), std::logic_error);
        EXPECT_THROW(service.movePage(0, 0), std::logic_error);
        EXPECT_THROW(service.replacePageImage(0, b), std::logic_error);
        EXPECT_THROW(service.rename("Changed"), std::logic_error);
        EXPECT_THROW(service.close(), std::logic_error);
        EXPECT_THROW(service.generatePdf(), std::logic_error);
    };
    EXPECT_TRUE(fs::exists(service.generatePdf()));
    EXPECT_EQ(service.snapshot().pageCount(), 1u);
}
