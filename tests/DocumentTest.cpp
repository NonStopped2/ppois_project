#include "document/Document.h"

#include <gtest/gtest.h>
#include <stdexcept>

using namespace lecture2pdf;

TEST(Slide, ValidatesIdAndPath) {
    EXPECT_THROW(Slide(0, "a.png"), std::invalid_argument);
    EXPECT_THROW(Slide(1, {}), std::invalid_argument);
    EXPECT_EQ(Slide(7, "missing.png").id(), 7u); // Model does not access disk.
}

TEST(Document, StartsEmptyAndStoresTitle) {
    Document document("Lecture 1");
    EXPECT_TRUE(document.empty());
    EXPECT_EQ(document.pageCount(), 0u);
    EXPECT_EQ(document.title(), "Lecture 1");
    EXPECT_THROW(document.page(0), std::out_of_range);
}

TEST(Document, RejectsBlankTitleWithoutChangingExistingTitle) {
    EXPECT_THROW(Document(" \t"), std::invalid_argument);
    Document document;
    EXPECT_THROW(document.rename(""), std::invalid_argument);
    EXPECT_EQ(document.title(), "Lecture");
    document.rename("New lecture");
    EXPECT_EQ(document.title(), "New lecture");
}

TEST(Document, AddsPagesInOrderWithStableIds) {
    Document document;
    const auto first = document.addPage("first.png");
    const auto second = document.addPage("second.png");
    EXPECT_NE(first, second);
    EXPECT_EQ(document.pageCount(), 2u);
    EXPECT_EQ(document.page(0).imagePath(), "first.png");
    EXPECT_EQ(document.page(1).id(), second);
    document.removePage(0);
    EXPECT_EQ(document.page(0).id(), second);
    EXPECT_GT(document.addPage("third.png"), second);
}

TEST(Document, InvalidAddDoesNotConsumeId) {
    Document document;
    EXPECT_THROW(document.addPage({}), std::invalid_argument);
    EXPECT_TRUE(document.empty());
    EXPECT_EQ(document.addPage("a.png"), 1u);
}

TEST(Document, MovesForwardBackwardAndToSamePosition) {
    Document document;
    const auto a = document.addPage("a.png");
    const auto b = document.addPage("b.png");
    const auto c = document.addPage("c.png");
    document.movePage(0, 2);
    EXPECT_EQ(document.page(0).id(), b);
    EXPECT_EQ(document.page(1).id(), c);
    EXPECT_EQ(document.page(2).id(), a);
    document.movePage(2, 0);
    document.movePage(1, 1);
    EXPECT_EQ(document.page(0).id(), a);
    EXPECT_EQ(document.page(1).id(), b);
    EXPECT_EQ(document.page(2).id(), c);
}

TEST(Document, InvalidIndicesDoNotModifyPages) {
    Document document;
    document.addPage("a.png");
    EXPECT_THROW(document.movePage(0, 1), std::out_of_range);
    EXPECT_THROW(document.movePage(1, 0), std::out_of_range);
    EXPECT_THROW(document.removePage(1), std::out_of_range);
    EXPECT_EQ(document.pageCount(), 1u);
    EXPECT_EQ(document.page(0).imagePath(), "a.png");
    document.removePage(0);
    EXPECT_TRUE(document.empty());
    EXPECT_THROW(document.movePage(0, 0), std::out_of_range);
}

TEST(Document, UpdatesPreparedImageWithoutChangingId) {
    Document document;
    const auto id = document.addPage("original.png");
    document.setImagePath(0, "prepared.png");
    EXPECT_EQ(document.page(0).id(), id);
    EXPECT_EQ(document.page(0).imagePath(), "prepared.png");
    EXPECT_THROW(document.setImagePath(0, {}), std::invalid_argument);
    EXPECT_EQ(document.page(0).imagePath(), "prepared.png");
}

TEST(Document, SnapshotCanBeChangedIndependently) {
    Document document;
    document.addPage("a.png");
    auto snapshot = document;
    document.removePage(0);
    EXPECT_EQ(snapshot.pageCount(), 1u);
    EXPECT_TRUE(document.empty());
}
