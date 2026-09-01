#ifndef UPDF_PDF_H
#define UPDF_PDF_H

#include <QPromise>

void mergePdfWorker(QPromise<void> &promise, const QStringList& files, const QString& target);

#endif // UPDF_PDF_H
