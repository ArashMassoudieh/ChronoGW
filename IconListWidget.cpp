#include "IconListWidget.h"
#include "GWA.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include "Well.h"

IconListWidget::IconListWidget(QWidget *parent)
    : QListWidget(parent)
    , currentItemType(ItemType::Well)
    , renameTimer(new QTimer(this))
    , lastClickedItem(nullptr)
{
    setupWidget();

    // Setup rename timer (500ms delay for single-click rename)
    renameTimer->setSingleShot(true);
    renameTimer->setInterval(500);
    connect(renameTimer, &QTimer::timeout, this, &IconListWidget::onRenameTimeout);

    // Connect signals
    connect(this, &QListWidget::itemClicked, this, &IconListWidget::onItemClicked);
    connect(this, &QListWidget::itemChanged, this, &IconListWidget::onItemChanged);
    connect(this, &QListWidget::customContextMenuRequested, this, &IconListWidget::showContextMenu);
}

void IconListWidget::setupWidget()
{
    // Set icon view mode
    setViewMode(QListWidget::IconMode);
    setIconSize(QSize(64, 64));
    setGridSize(QSize(100, 100));

    // Enable drag and drop for reordering
    setDragDropMode(QAbstractItemView::InternalMove);

    // Set selection mode
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Enable right-click context menu
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Set spacing and layout
    setSpacing(10);
    setResizeMode(QListWidget::Adjust);
    setMovement(QListWidget::Static);
    setWrapping(true);
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

void IconListWidget::addItem(const QString& name, const QVariant& data)
{
    QListWidgetItem* item = new QListWidgetItem(currentIcon, name);
    item->setData(Qt::UserRole, data);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QListWidget::addItem(item);
}

void IconListWidget::populateFromGWA(CGWA* gwa, ItemType type)
{
    if (!gwa) {
        return;
    }

    // Clear existing items
    clear();

    // Set the item type and load appropriate icon
    setItemType(type);

    // Populate based on type
    switch (type) {
    case ItemType::Well:
        for (size_t i = 0; i < gwa->getWellCount(); ++i) {
            const CWell& well = gwa->getWell(i);
            addItem(QString::fromStdString(well.getName()), QVariant::fromValue(static_cast<int>(i)));
        }
        break;

    case ItemType::Tracer:
        for (size_t i = 0; i < gwa->getTracerCount(); ++i) {
            const CTracer& tracer = gwa->getTracer(i);
            addItem(QString::fromStdString(tracer.getName()), QVariant::fromValue(static_cast<int>(i)));
        }
        break;

    case ItemType::Parameter:
        for (size_t i = 0; i < gwa->getParameterCount(); ++i) {
            const Parameter* param = gwa->getParameter(i);
            if (param) {
                addItem(QString::fromStdString(param->GetName()), QVariant::fromValue(static_cast<int>(i)));
            }
        }
        break;

    case ItemType::Observation:
        for (size_t i = 0; i < gwa->getObservationCount(); ++i) {
            const Observation& obs = gwa->getObservation(i);
            addItem(QString::fromStdString(obs.GetName()), QVariant::fromValue(static_cast<int>(i)));
        }
        break;
    }
}

QStringList IconListWidget::getItemNames() const
{
    QStringList names;
    for (int i = 0; i < count(); ++i) {
        names << item(i)->text();
    }
    return names;
}

void IconListWidget::removeSelectedItems()
{
    QList<QListWidgetItem*> selected = selectedItems();

    if (selected.isEmpty()) {
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
        for (QListWidgetItem* item : selected) {
            emit itemDeleteRequested(item->text(), item->data(Qt::UserRole));
            delete item;
        }
    }
}

void IconListWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QListWidgetItem* item = itemAt(event->pos());

    if (item && event->button() == Qt::LeftButton) {
        // Stop rename timer if running
        renameTimer->stop();

        // Emit signal to show properties dialog
        emit itemPropertiesRequested(item->text(), item->data(Qt::UserRole));

        event->accept();
    } else {
        QListWidget::mouseDoubleClickEvent(event);
    }
}

void IconListWidget::mousePressEvent(QMouseEvent* event)
{
    QListWidgetItem* item = itemAt(event->pos());

    if (event->button() == Qt::LeftButton && item) {
        // Check if clicking on already selected item
        if (item == currentItem() && item == lastClickedItem) {
            // Start rename timer
            renameTimer->start();
        } else {
            // Stop timer if clicking different item
            renameTimer->stop();
            lastClickedItem = item;
        }
    }

    QListWidget::mousePressEvent(event);
}

void IconListWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F2) {
        // F2 pressed - start rename
        QListWidgetItem* item = currentItem();
        if (item) {
            startRename(item);
        }
        event->accept();
    } else if (event->key() == Qt::Key_Delete) {
        // Delete key pressed
        removeSelectedItems();
        event->accept();
    } else {
        QListWidget::keyPressEvent(event);
    }
}

void IconListWidget::onItemClicked(QListWidgetItem* item)
{
    lastClickedItem = item;
}

void IconListWidget::onRenameTimeout()
{
    // Timer expired - start rename
    if (lastClickedItem && lastClickedItem == currentItem()) {
        startRename(lastClickedItem);
    }
}

void IconListWidget::startRename(QListWidgetItem* item)
{
    if (!item) return;

    // Store old name
    oldItemName = item->text();

    // Start editing
    editItem(item);
}

void IconListWidget::onItemChanged(QListWidgetItem* item)
{
    // Check if name actually changed
    QString newName = item->text().trimmed();

    if (newName.isEmpty()) {
        // Don't allow empty names - revert
        item->setText(oldItemName);
        QMessageBox::warning(this, tr("Invalid Name"), tr("Name cannot be empty."));
        return;
    }

    if (newName != oldItemName && !oldItemName.isEmpty()) {
        // Name changed - emit signal
        emit itemRenamed(oldItemName, newName, item->data(Qt::UserRole));
    }

    // Clear old name
    oldItemName.clear();
}

void IconListWidget::showContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = itemAt(pos);

    QMenu contextMenu(this);

    if (item) {
        // Item-specific menu
        QAction* propertiesAction = contextMenu.addAction(tr("Properties..."));
        QAction* renameAction = contextMenu.addAction(tr("Rename"));
        contextMenu.addSeparator();
        QAction* deleteAction = contextMenu.addAction(tr("Delete"));

        QAction* selectedAction = contextMenu.exec(mapToGlobal(pos));

        if (selectedAction == propertiesAction) {
            emit itemPropertiesRequested(item->text(), item->data(Qt::UserRole));
        } else if (selectedAction == renameAction) {
            startRename(item);
        } else if (selectedAction == deleteAction) {
            removeSelectedItems();
        }
    } else {
        // Empty area menu
        QAction* addAction = contextMenu.addAction(tr("Add New..."));

        QAction* selectedAction = contextMenu.exec(mapToGlobal(pos));

        if (selectedAction == addAction) {
            // You could emit a signal here to add new item
            // For now, we'll let the parent handle this through toolbar/menu
        }
    }
}
