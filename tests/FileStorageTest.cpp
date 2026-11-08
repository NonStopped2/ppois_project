#include "storage/FileStorage.h"

#include <gtest/gtest.h>
#include <fstream>
#include <stdexcept>

using namespace lecture2pdf;
namespace fs = std::filesystem;

TEST(FileStorage, OwnsOnlyItsGeneratedDirectory) {
    FileStorage root;
    const auto unrelated = root.newPath(".txt");
    std::ofstream(unrelated) << "keep me";
    fs::path firstDirectory;
    FileStorage second(root.directory());
    {
        FileStorage first(root.directory());
        firstDirectory = first.directory();
        EXPECT_NE(firstDirectory, second.directory());
        std::ofstream(first.newPath(".png")) << "temporary";
    }
    EXPECT_FALSE(fs::exists(firstDirectory));
    EXPECT_TRUE(fs::exists(second.directory()));
    EXPECT_TRUE(fs::exists(unrelated));
}

TEST(FileStorage, ImportsCopiesWithGeneratedNonCollidingNames) {
    FileStorage source;
    const auto input = source.newPath(".png");
    std::ofstream(input) << "image bytes";
    FileStorage target;
    const auto a = target.importFile(input);
    const auto b = target.importFile(input);
    EXPECT_NE(a, b);
    EXPECT_EQ(a.parent_path(), target.directory());
    EXPECT_EQ(a.extension(), ".png");
    EXPECT_EQ(fs::file_size(a), fs::file_size(input));
    target.cleanup();
    EXPECT_TRUE(fs::exists(input));
}

TEST(FileStorage, CleanupIsIdempotentAndPreventsReuse) {
    FileStorage storage;
    const auto directory = storage.directory();
    storage.cleanup();
    storage.cleanup();
    EXPECT_FALSE(fs::exists(directory));
    EXPECT_THROW(storage.newPath(".pdf"), std::logic_error);
    EXPECT_THROW(storage.importFile("anything.png"), std::logic_error);
}

TEST(FileStorage, RejectsTraversalExtensions) {
    FileStorage storage;
    for (const std::string extension : {"", ".", "pdf", "../outside", ".png/../../x", ".x\\y"}) {
        EXPECT_THROW(storage.newPath(extension), std::invalid_argument);
    }
    EXPECT_EQ(storage.newPath(".PDF").parent_path(), storage.directory());
}

TEST(FileStorage, RejectsMissingFilesAndDirectories) {
    FileStorage storage;
    EXPECT_THROW(storage.importFile(storage.directory()), std::invalid_argument);
    EXPECT_ANY_THROW(storage.importFile(storage.directory() / "missing.png"));
}

TEST(FileStorage, MissingRootIsNotCreated) {
    FileStorage root;
    const auto missing = root.directory() / "missing";
    EXPECT_THROW(FileStorage{missing}, fs::filesystem_error);
    EXPECT_FALSE(fs::exists(missing));
}

TEST(FileStorage, SkipsExistingGeneratedNames) {
    FileStorage storage;
    const auto existing = storage.directory() / "file-1.png";
    std::ofstream(existing) << "keep me";
    EXPECT_NE(storage.newPath(".png"), existing);
    EXPECT_EQ(fs::file_size(existing), 7u);
}

TEST(FileStorage, UnwindingCleansTemporaryDirectory) {
    fs::path path;
    try {
        FileStorage storage;
        path = storage.directory();
        std::ofstream(storage.newPath(".txt")) << "temporary";
        throw std::runtime_error("simulated failure");
    } catch (const std::runtime_error&) {
    }
    EXPECT_FALSE(fs::exists(path));
}
