#ifndef UPDF_PDFMERGEWORKER_H
#define UPDF_PDFMERGEWORKER_H

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>

#include <QObject>

class PdfMergeWorker : public QObject {
    Q_OBJECT

public:
    PdfMergeWorker() : isCancelled(false) {}

    void cancel() {
        isCancelled.store(true);
    }

public slots:
    void startMerge(const QStringList &files, const QString &target) {
        const int total = static_cast<int>(files.size());
        isCancelled.store(false);

        try {
            QPDF output;
            output.emptyPDF();

            QPDFPageDocumentHelper outputPages(output);

            for (int i = 0; i < total; ++i) {
                if (isCancelled.load()) {
                    emit progressChanged(0);
                    emit cancelled();
                    return;
                }

                const QString &file = files.at(i);
                QPDF input;
                input.processFile(file.toStdString().c_str());

                QPDFPageDocumentHelper inputPages(input);
                const auto pages = inputPages.getAllPages();
                for (auto const &page: pages) {
                    if (isCancelled.load()) {
                        emit progressChanged(0);
                        emit cancelled();
                        return;
                    }

                    outputPages.addPage(page, false);
                }

                const int percent =
                        static_cast<int>((i + 1) * 100.0 / total);

                emit progressChanged(percent);
            }

            QPDFWriter writer(output, target.toStdString().c_str());
            writer.write();

            emit finished();
        } catch (std::exception &e) {
            emit failed(QString::fromUtf8(e.what()));
        }
    }

signals:
    void progressChanged(int percent);

    void finished();

    void cancelled();

    void failed(const QString &error);

private:
    std::atomic_bool isCancelled;
};

#endif //UPDF_PDFMERGEWORKER_H
