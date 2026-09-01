#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>

#include "pdftools.h"

#include <QThread>

void mergePdfWorker(QPromise<void> &promise, const QStringList &files, const QString &target) {
    const int total = static_cast<int>(files.size());

    QPDF output;
    output.emptyPDF();

    QPDFPageDocumentHelper outputPages(output);

    for (int i = 0; i < total; ++i) {
        if (promise.isCanceled()) {
            return;
        }

        const QString &file = files.at(i);
        QPDF input;
        input.processFile(file.toStdString().c_str());

        QPDFPageDocumentHelper inputPages(input);
        const auto pages = inputPages.getAllPages();

        auto percent = i * 100.0 / total;
        const auto percentPerPage = 1 * 100.0 / total / static_cast<double>(pages.size());
        for (auto const &page: pages) {
            if (promise.isCanceled()) {
                return;
            }
            outputPages.addPage(page, false);
            percent += percentPerPage;
            promise.setProgressValue(static_cast<int>(percent));
        }
    }

    QPDFWriter writer(output, target.toStdString().c_str());
    writer.write();

    promise.finish();
}
