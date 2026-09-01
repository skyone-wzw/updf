#ifndef UPDF_MAINWINDOW_H
#define UPDF_MAINWINDOW_H

#include <QLineEdit>
#include <QFileDialog>
#include <QProgressBar>
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrent/qtconcurrentrun.h>

#include "itemorderlist.hpp"
#include "pdftools.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("PDF 快速合并");
        resize(600, 400);
        setAcceptDrops(true);

        auto *layout = new QVBoxLayout(this);

        // 第一行: 文件夹选择
        auto *selectPathLayout = new QHBoxLayout();
        pathEdit = new QLineEdit();
        pathSelectButton = new QPushButton("选择目录");
        selectPathLayout->addWidget(pathEdit);
        selectPathLayout->addWidget(pathSelectButton);

        // 第二行: 文件排序
        fileOrder = new ItemOrderList();

        // 第三行: 进度条
        progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(0);

        // 第四行: 操作控制
        auto *actionLayout = new QHBoxLayout();
        startActionButton = new QPushButton("开始合并");
        cancelActionButton = new QPushButton("取消");
        cancelActionButton->setEnabled(false);
        actionLayout->addWidget(startActionButton);
        actionLayout->addWidget(cancelActionButton);

        layout->addLayout(selectPathLayout);
        layout->addWidget(fileOrder);
        layout->addWidget(progressBar);
        layout->addLayout(actionLayout);

        mergeWatcher = new QFutureWatcher<void>(this);

        // 更新文件列表
        connect(pathEdit, &QLineEdit::textChanged, this, &MainWindow::updateFiles);
        // 文件夹选择按钮
        connect(pathSelectButton, &QPushButton::clicked, this, &MainWindow::selectPath);
        // worker 更新进度条
        connect(mergeWatcher, &QFutureWatcher<void>::progressValueChanged, progressBar, &QProgressBar::setValue);
        connect(mergeWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::onWorkerFinished);

        connect(startActionButton, &QPushButton::clicked, this, &MainWindow::startMergeTask);
        connect(cancelActionButton, &QPushButton::clicked, this, &MainWindow::cancelMergeTask);
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        const QMimeData *mimeData = event->mimeData();

        if (!mimeData->hasUrls()) {
            event->ignore();
            return;
        }

        const QList<QUrl> urls = mimeData->urls();

        if (urls.isEmpty()) {
            event->ignore();
            return;
        }

        const QString path = urls.first().toLocalFile();

        // 只接受文件夹
        if (!path.isEmpty() && QFileInfo(path).isDir()) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent *event) override {
        const QMimeData *mimeData = event->mimeData();

        if (!mimeData->hasUrls()) {
            event->ignore();
            return;
        }

        const QList<QUrl> urls = mimeData->urls();

        if (urls.isEmpty()) {
            event->ignore();
            return;
        }

        const QString path = urls.first().toLocalFile();

        if (!path.isEmpty() && QFileInfo(path).isDir()) {
            pathEdit->setText(path);
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

private:
    QLineEdit *pathEdit;
    QPushButton *pathSelectButton;
    ItemOrderList *fileOrder;
    QPushButton *startActionButton;
    QPushButton *cancelActionButton;
    QProgressBar *progressBar;

    QFutureWatcher<void> *mergeWatcher;

    void selectPath() {
        const QString startDir = QDir(pathEdit->text()).exists() ? pathEdit->text() : QDir::homePath();
        const QString path = QFileDialog::getExistingDirectory(
            this,
            "选择文件夹",
            startDir,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        if (path.isEmpty()) {
            return;
        }
        const QDir dir(path);
        pathEdit->setText(dir.absolutePath());
    }

    void updateFiles() {
        const auto dir = QDir(pathEdit->text());
        if (!dir.exists()) {
            fileOrder->setItems(QStringList());
            return;
        }
        const auto files = dir.entryList(
            QStringList("*.pdf"),
            QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name
        );
        fileOrder->setItems(files);
    }

    void startMergeTask() {
        auto filenames = fileOrder->getItems();
        const auto dir = QDir(pathEdit->text());
        if (filenames.isEmpty()) {
            return;
        }

        const auto target = QFileDialog::getSaveFileName(
            this,
            "保存文件",
            dir.absolutePath(),
            "PDF文件 (*.pdf)"
        );
        if (target.isEmpty()) {
            return;
        }

        startActionButton->setEnabled(false);
        cancelActionButton->setEnabled(true);
        progressBar->setValue(0);

        auto files = QStringList();
        for (const auto &filename: filenames) {
            files.append(dir.filePath(filename));
        }

        const auto future = QtConcurrent::run(
            mergePdfWorker,
            files,
            target
        );

        mergeWatcher->setFuture(future);
    }

    void cancelMergeTask() {
        mergeWatcher->cancel();
        cancelActionButton->setEnabled(false);
    }

    void onWorkerFinished() {
        startActionButton->setEnabled(true);
        cancelActionButton->setEnabled(false);

        try {
            mergeWatcher->future().waitForFinished();
        } catch (const std::exception &error) {
            QMessageBox::critical(
                this,
                "错误",
                QString::fromUtf8(error.what())
            );
            return;
        } catch (...) {
            QMessageBox::critical(
                this,
                "错误",
                "未知错误"
            );
            return;
        }
        progressBar->setValue(100);
    }
};

#endif // UPDF_MAINWINDOW_H
