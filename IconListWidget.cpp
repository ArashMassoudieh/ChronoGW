#include "IconListWidget.h"
#include "GWA.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include "Well.h"

IconListWidget::IconListWidget(QWidget *parent)
    : QWidget(parent)
    , gwa_(nullptr)
    , currentItemType(ItemType::Well)
    , renameTimer(new QTimer(this))
    , lastClickedItem(nullptr)
{
    setupUI();

    // Setup rename timer (500ms delay for single-click rename)
    renameTimer->setSingleShot(true);
    renameTimer->setInterval(500);
    connect(renameTimer, &QTimer::timeout, this, &IconListWidget::onRenameTimeout);
}

void IconListWidget::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);

    // Create toolbar
    toolbar_ = new QToolBar(this);
    toolbar_->setIconSize(QSize(24, 24));

    // Add button
    addAction_ = toolbar_->addAction(QIcon::fromTheme("list-add"), tr("Add"));
    addAction_->setToolTip(tr("Add new item"));
    connect(addAction_, &QAction::triggered, this, &IconListWidget::onAddClicked);

    // Remove button
    removeAction_ = toolbar_->addAction(QIcon::fromTheme("list-remove"), tr("Remove"));
    removeAction_->setToolTip(tr("Remove selected item(s)"));
    connect(removeAction_, &QAction::triggered, this, &IconListWidget::onRemoveClicked);

    mainLayout_->addWidget(toolbar_);

    // Create list widget
    listWidget_ = new QListWidget(this);
    setupListWidget();
    mainLayout_->addWidget(listWidget_);

    setLayout(mainLayout_);
}

void IconListWidget::setupListWidget()
{
    // Set icon view mode
    listWidget_->setViewMode(QListWidget::IconMode);
    listWidget_->setIconSize(QSize(64, 64));
    listWidget_->setGridSize(QSize(100, 100));

    // Enable drag and drop for reordering
    listWidget_->setDragDropMode(QAbstractItemView::InternalMove);

    // Set selection mode
    listWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Enable right-click context menu
    listWidget_->setContextMenuPolicy(Qt::CustomContextMenu);

    // Set spacing and layout
    listWidget_->setSpacing(10);
    listWidget_->setResizeMode(QListWidget::Adjust);
    listWidget_->setMovement(QListWidget::Static);
    listWidget_->setWrapping(true);

    // Connect signals
    connect(listWidget_, &QListWidget::itemClicked, this, &IconListWidget::onItemClicked);
    connect(listWidget_, &QListWidget::itemChanged, this, &IconListWidget::onItemChanged);
    connect(listWidget_, &QListWidget::customContextMenuRequested, this, &IconListWidget::showContextMenu);
    connect(listWidget_, &QListWidget::itemDoubleClicked, this, &IconListWidget::onItemDoubleClicked);
}

void IconListWidget::setItemType(ItemType type)
{
    currentItemType = type;
    loadIcon();
}

void IconListWidget::loadIcon()
{
    QString iconPath;

    switch (currentItemType) {
    case ItemType::Well:
        iconPath = ":/icons/well.png";
        break;
    case ItemType::Tracer:
        iconPath = ":/icons/tracer.png";
        break;
    case ItemType::Parameter:
        iconPath = ":/icons/parameter.png";
        break;
    case ItemType::Observation:
        iconPath = ":/icons/observation.png";
        break;
    }

    currentIcon = QIcon(iconPath);

    // If icon doesn't exist, use a default icon
    if (currentIcon.isNull()) {
        // Use built-in Qt icon as fallback
        currentIcon = style()->standardIcon(QStyle::SP_FileIcon);
    }
}

void IconListWidget::addItem(const QString& name, int index)
{
    QListWidgetItem* item = new QListWidgetItem(currentIcon, name);
    item->setData(Qt::UserRole, index);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    listWidget_->addItem(item);
}

void IconListWidget::refresh()
{
    if (!gwa_) {
        return;
    }

    // Clear existing items
    listWidget_->clear();

    // Populate based on type
    switch (currentItemType) {
    case ItemType::Well:
        for (size_t i = 0; i < gwa_->getWellCount(); ++i) {
            const CWell& well = gwa_->getWell(i);
            addItem(QString::fromStdString(well.getName()), static_cast<int>(i));
        }
        break;

    case ItemType::Tracer:
        for (size_t i = 0; i < gwa_->getTracerCount(); ++i) {
            const CTracer& tracer = gwa_->getTracer(i);
            addItem(QString::fromStdString(tracer.getName()), static_cast<int>(i));
        }
        break;

    case ItemType::Parameter:
        for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
            const Parameter* param = gwa_->getParameter(i);
            if (param) {
                addItem(QString::fromStdString(param->GetName()), static_cast<int>(i));
            }
        }
        break;

    case ItemType::Observation:
        for (size_t i = 0; i < gwa_->getObservationCount(); ++i) {
            const Observation& obs = gwa_->getObservation(i);
            addItem(QString::fromStdString(obs.GetName()), static_cast<int>(i));
        }
        break;
    }
}

QStringList IconListWidget::getItemNames() const
{
    QStringList names;
    for (int i = 0; i < listWidget_->count(); ++i) {
        names << listWidget_->item(i)->text();
    }
    return names;
}

void IconListWidget::removeSelectedItems()
{
    if (!gwa_) {
        return;
    }

    QList<QListWidgetItem*> selected = listWidget_->selectedItems();

    if (selected.isEmpty()) {
        QMessageBox::information(this, tr("No Selection"),
                                 tr("Please select one or more items to remove."));
        return;
    }

    // Confirm deletion
    QString message = selected.count() == 1
                          ? tr("Are you sure you want to delete '%1'?").arg(selected[0]->text())
                          : tr("Are you sure you want to delete %1 items?").arg(selected.count());

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Confirm Delete"),
        message,
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        // Collect indices to delete (sort in descending order to delete from end)
        QList<int> indicesToDelete;
        for (QListWidgetItem* item : selected) {
            indicesToDelete.append(item->data(Qt::UserRole).toInt());
        }
        std::sort(indicesToDelete.begin(), indicesToDelete.end(), std::greater<int>());

        // Delete from GWA model
        for (int index : indicesToDelete) {
            switch (currentItemType) {
            case ItemType::Well:
                gwa_->removeWell(index);
                break;
            case ItemType::Tracer:
                gwa_->removeTracer(index);
                break;
            case ItemType::Parameter:
                gwa_->removeParameter(index);
                break;
            case ItemType::Observation:
                gwa_->removeObservation(index);
                break;
            }
        }

        // Refresh the list
        refresh();

        emit listModified();
    }
}

void IconListWidget::onAddClicked()
{
    emit addItemRequested();
}

void IconListWidget::onRemoveClicked()
{
    removeSelectedItems();
}

void IconListWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item) {
        // Stop rename timer if running
        renameTimer->stop();

        // Emit signal to show properties dialog
        int index = item->data(Qt::UserRole).toInt();
        emit itemPropertiesRequested(item->text(), index);
    }
}

void IconListWidget::onItemClicked(QListWidgetItem* item)
{
    // Check if clicking on already selected item
    if (item == listWidget_->currentItem() && item == lastClickedItem) {
        // Start rename timer
        renameTimer->start();
    } else {
        // Stop timer if clicking different item
        renameTimer->stop();
        lastClickedItem = item;
    }
}

void IconListWidget::onRenameTimeout()
{
    // Timer expired - start rename
    if (lastClickedItem && lastClickedItem == listWidget_->currentItem()) {
        startRename(lastClickedItem);
    }
}

void IconListWidget::startRename(QListWidgetItem* item)
{
    if (!item) return;

    // Store old name
    oldItemName = item->text();

    // Start editing
    listWidget_->editItem(item);
}

void IconListWidget::onItemChanged(QListWidgetItem* item)
{
    if (!gwa_) return;

    // Check if name actually changed
    QString newName = item->text().trimmed();

    if (newName.isEmpty()) {
        // Don't allow empty names - revert
        item->setText(oldItemName);
        QMessageBox::warning(this, tr("Invalid Name"), tr("Name cannot be empty."));
        return;
    }

    if (newName != oldItemName && !oldItemName.isEmpty()) {
        // Name changed - update in GWA model
        int index = item->data(Qt::UserRole).toInt();

        switch (currentItemType) {
        case ItemType::Well:
            if (index >= 0 && static_cast<size_t>(index) < gwa_->getWellCount()) {
                gwa_->getWell(index).setName(newName.toStdString());
            }
            break;
        case ItemType::Tracer:
            if (index >= 0 && static_cast<size_t>(index) < gwa_->getTracerCount()) {
                gwa_->getTracer(index).setName(newName.toStdString());
            }
            break;
        case ItemType::Parameter:
            if (index >= 0 && static_cast<size_t>(index) < gwa_->getParameterCount()) {
                Parameter* param = gwa_->getParameter(index);
                if (param) {
                    param->SetName(newName.toStdString());
                }
            }
            break;
        case ItemType::Observation:
            if (index >= 0 && static_cast<size_t>(index) < gwa_->getObservationCount()) {
                gwa_->getObservation(index).SetName(newName.toStdString());
            }
            break;
        }

        emit listModified();
    }

    // Clear old name
    oldItemName.clear();
}

void IconListWidget::showContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = listWidget_->itemAt(pos);

    QMenu contextMenu(this);

    if (item) {
        // Item-specific menu
        QAction* propertiesAction = contextMenu.addAction(tr("Properties..."));

        // Add type-specific actions
        QAction* plotAction = nullptr;
        if (currentItemType == ItemType::Observation) {
            plotAction = contextMenu.addAction(QIcon(":/icons/modeledvsmeasured.png"),
                                               tr("Plot Modeled vs. Observed"));
        }

        QAction* renameAction = contextMenu.addAction(tr("Rename"));
        contextMenu.addSeparator();
        QAction* deleteAction = contextMenu.addAction(tr("Delete"));

        QAction* selectedAction = contextMenu.exec(listWidget_->mapToGlobal(pos));

        if (selectedAction == propertiesAction) {
            int index = item->data(Qt::UserRole).toInt();
            emit itemPropertiesRequested(item->text(), index);
        }
        else if (plotAction && selectedAction == plotAction) {
            int index = item->data(Qt::UserRole).toInt();
            emit itemContextActionRequested(item->text(), index, "plot");
        }
        else if (selectedAction == renameAction) {
            startRename(item);
        }
        else if (selectedAction == deleteAction) {
            removeSelectedItems();
        }
    } else {
        // Empty area menu
        QAction* addAction = contextMenu.addAction(tr("Add New..."));

        QAction* selectedAction = contextMenu.exec(listWidget_->mapToGlobal(pos));

        if (selectedAction == addAction) {
            emit addItemRequested();
        }
    }
}
