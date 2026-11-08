#pragma once

#include "pdf/IPdfBackend.h"

namespace lecture2pdf {

class PdfGenerator {
public:
    // Non-owning reference: backend must outlive the generator.
    explicit PdfGenerator(IPdfBackend& backend);

    // Output parent must exist. Refuses to overwrite any existing target.
    // Stages output and publishes via an exclusive hard link on the same filesystem.
    // Pass a stable document snapshot; concurrent mutation is not supported.
    void generate(const Document& document, const std::filesystem::path& outputPdf);

private:
    IPdfBackend& backend_;
};

} // namespace lecture2pdf
