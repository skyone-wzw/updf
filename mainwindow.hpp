#ifndef UPDF_MAINWINDOW_H
#define UPDF_MAINWINDOW_H

#include <QLineEdit>
#include <QFileDialog>
#include <QProgressBar>
#include <QMessageBox>
#include <QThread>
#include <QDir>

#include "itemorderlist.hpp"
#include "pdfmergeworker.hpp"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("PDF 快速合并");
        resize(600, 400);
        setMinimumWidth(400);

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

        workerThread = new QThread(this);
        worker = new PdfMergeWorker();
        worker->moveToThread(workerThread);
        // 保持 worker 线程存活
        connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
        // 启动 Merge Task 的信号
        connect(this, &MainWindow::mergeRequested, worker, &PdfMergeWorker::startMerge);
        workerThread->start();

        // 更新文件列表
        connect(pathEdit, &QLineEdit::textChanged, this, &MainWindow::updateFiles);
        // 文件夹选择按钮
        connect(pathSelectButton, &QPushButton::clicked, this, &MainWindow::selectPath);
        // worker 更新进度条
        connect(worker, &PdfMergeWorker::progressChanged, progressBar, &QProgressBar::setValue);
        connect(worker, &PdfMergeWorker::finished, this, &MainWindow::onWorkerFinished);
        connect(worker, &PdfMergeWorker::cancelled, this, &MainWindow::onWorkerCancelled);
        connect(worker, &PdfMergeWorker::failed, this, &MainWindow::onWorkerFailed);

        connect(startActionButton, &QPushButton::clicked, this, &MainWindow::startMergeTask);
        connect(cancelActionButton, &QPushButton::clicked, this, &MainWindow::cancelMergeTask);
    }

    ~MainWindow() override {
        workerThread->quit();
        workerThread->wait();
    }

signals:
    void mergeRequested(const QStringList &files, const QString &target) const;

private:
    QLineEdit *pathEdit;
    QPushButton *pathSelectButton;
    ItemOrderList *fileOrder;
    QPushButton *startActionButton;
    QPushButton *cancelActionButton;
    QProgressBar *progressBar;

    QThread *workerThread;
    PdfMergeWorker *worker;

    void selectPath() {
        const QString startDir = QDir(pathEdit->text()).exists() ? pathEdit->text() : QDir::homePath();
        const QDir dir = QFileDialog::getExistingDirectory(
            this,
            "选择文件夹",
            startDir,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        pathEdit->setText(dir.absolutePath());
    }

    void updateFiles() const {
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
        emit mergeRequested(files, target);
    }

    void cancelMergeTask() const {
        worker->cancel();
        cancelActionButton->setEnabled(false);
    }

    void onWorkerFinished() {
        startActionButton->setEnabled(true);
        cancelActionButton->setEnabled(false);
        QMessageBox::information(
            this,
            "完成",
            "PDF 合并完成"
        );
    }

    void onWorkerCancelled() const {
        startActionButton->setEnabled(true);
        cancelActionButton->setEnabled(false);
    }

    void onWorkerFailed(const QString &error) {
        startActionButton->setEnabled(true);
        cancelActionButton->setEnabled(false);
        QMessageBox::critical(
            this,
            "错误",
            error
        );
    }
};

#endif // UPDF_MAINWINDOW_H
