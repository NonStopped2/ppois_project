#pragma once

#include "document/Document.h"

namespace lecture2pdf {

// Writes exactly one complete PDF, in document order, or throws.
// Must close all output handles before returning/throwing. Not thread-safe.
class IPdfBackend {
public:
    virtual ~IPdfBackend() = default;
    virtual void write(const Document& document,
                       const std::filesystem::path& outputPdf) = 0;
};

} // namespace lecture2pdf
