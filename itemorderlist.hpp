#ifndef UPDF_ITEMORDERLIST_H
#define UPDF_ITEMORDERLIST_H

#include <qboxlayout.h>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

class ItemOrderList : public QWidget {
public:
    explicit ItemOrderList(QWidget *parent = nullptr) : QWidget(parent) {
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        list = new QListWidget();
        list->setDragEnabled(true);
        list->setAcceptDrops(true);
        list->setDropIndicatorShown(true);
        list->setDragDropMode(QAbstractItemView::InternalMove);
        list->setDefaultDropAction(Qt::MoveAction);
        list->setMinimumWidth(350);

        auto *orderButtonLayout = new QVBoxLayout();
        orderUp1 = new QPushButton("上移");
        orderUp2 = new QPushButton("置顶");
        orderDown1 = new QPushButton("下移");
        orderDown2 = new QPushButton("置底");
        orderRemove = new QPushButton("移除");
        orderButtonLayout->addStretch(1);
        orderButtonLayout->addWidget(orderUp2);
        orderButtonLayout->addWidget(orderUp1);
        orderButtonLayout->addWidget(orderDown1);
        orderButtonLayout->addWidget(orderDown2);
        orderButtonLayout->addStretch(1);
        orderButtonLayout->addWidget(orderRemove);

        layout->addWidget(list);
        layout->addLayout(orderButtonLayout);

        // 上移
        connect(orderUp1, &QPushButton::clicked, this, [this] {
            const int row = list->currentRow();

            if (row <= 0)
                return;

            moveItem(row, row - 1);
        });

        // 置顶
        connect(orderUp2, &QPushButton::clicked, this, [this] {
            const int row = list->currentRow();

            if (row <= 0)
                return;

            moveItem(row, 0);
        });

        // 下移
        connect(orderDown1, &QPushButton::clicked, this, [this] {
            const int row = list->currentRow();

            if (row < 0 || row >= list->count() - 1)
                return;

            moveItem(row, row + 1);
        });

        // 置底
        connect(orderDown2, &QPushButton::clicked, this, [this] {
            const int row = list->currentRow();

            if (row < 0 || row >= list->count() - 1)
                return;

            moveItem(row, list->count() - 1);
        });

        // 移除
        connect(orderRemove, &QPushButton::clicked, this, [this] {
            const int row = list->currentRow();

            if (row < 0)
                return;

            delete list->takeItem(row);

            // 删除后尽量保持当前行位置
            if (list->count() > 0) {
                list->setCurrentRow(qMin(row, list->count() - 1));
                list->setFocus();
            }

            updateButtons();
        });

        // 当前选中项变化时更新按钮状态
        connect(list, &QListWidget::currentRowChanged,
                this, [this](int) {
                    updateButtons();
                });

        // 拖动排序结束后更新按钮状态
        connect(list->model(), &QAbstractItemModel::rowsMoved,
                this, [this]() {
                    updateButtons();
                });

        updateButtons();
    }

    void setItems(const QStringList &items) {
        list->clear();
        list->addItems(items);

        if (!items.isEmpty())
            list->setCurrentRow(0);

        updateButtons();
    }

    [[nodiscard]] QStringList getItems() const {
        QStringList items;

        for (int i = 0; i < list->count(); ++i) {
            items << list->item(i)->text();
        }

        return items;
    }

private:
    QListWidget *list;
    QPushButton *orderUp1;
    QPushButton *orderUp2;
    QPushButton *orderDown1;
    QPushButton *orderDown2;
    QPushButton *orderRemove;

    /**
     * 将某一行移动到目标行。
     */
    void moveItem(const int from, const int to) {
        if (from < 0 || from >= list->count())
            return;

        if (to < 0 || to >= list->count())
            return;

        if (from == to)
            return;

        const int destination = to > from ? to + 1 : to;

        if (list->model()->moveRow(
            QModelIndex(),
            from,
            QModelIndex(),
            destination)) {
            list->setCurrentRow(to);
            list->setFocus();
        }

        updateButtons();
    }

    void updateButtons() {
        const int row = list->currentRow();
        const int count = list->count();

        const bool hasCurrent = row >= 0;
        const bool canMoveUp = hasCurrent && row > 0;
        const bool canMoveDown = hasCurrent && row < count - 1;

        orderUp1->setEnabled(canMoveUp);
        orderUp2->setEnabled(canMoveUp);

        orderDown1->setEnabled(canMoveDown);
        orderDown2->setEnabled(canMoveDown);

        orderRemove->setEnabled(hasCurrent);
    }
};

#endif //UPDF_ITEMORDERLIST_H
