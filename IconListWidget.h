#ifndef ICONLISTWIDGET_H
#define ICONLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QTimer>
#include <QString>
#include <QToolBar>
#include <QVBoxLayout>

// Forward declaration
class CGWA;

/**
 * @brief A reusable widget for displaying items (wells, tracers, parameters, observations)
 *        as icons with text labels
 *
 * Features:
 * - Icon view with text labels
 * - Add/Remove toolbar buttons
 * - Double-click to edit/view properties
 * - Single-click + wait or F2 to rename
 * - Context menu support (right-click)
 */
class IconListWidget : public QWidget
{
    Q_OBJECT

public:
    enum class ItemType {
        Well,
        Tracer,
        Parameter,
        Observation
    };

    explicit IconListWidget(QWidget *parent = nullptr);

    /**
     * @brief Set the GWA model reference
     * @param gwa Pointer to GWA model
     */
    void setGWA(CGWA* gwa) { gwa_ = gwa; }

    /**
     * @brief Set the type of items this widget will display
     * @param type The type (Well, Tracer, Parameter, or Observation)
     */
    void setItemType(ItemType type);

    /**
     * @brief Get the current item type
     */
    ItemType getItemType() const { return currentItemType; }

    /**
     * @brief Refresh the list from the GWA model
     */
    void refresh();

    /**
     * @brief Get all item names in the list
     * @return List of item names
     */
    QStringList getItemNames() const;

    /**
     * @brief Get the internal QListWidget
     */
    QListWidget* listWidget() { return listWidget_; }

signals:
    /**
     * @brief Emitted when user clicks the Add button
     */
    void addItemRequested();

    /**
     * @brief Emitted when user double-clicks an item to edit properties
     * @param itemName The name of the item
     * @param index Index in the GWA model
     */
    void itemPropertiesRequested(const QString& itemName, int index);

    /**
     * @brief Emitted when the list has been modified (item added/removed/renamed)
     */
    void listModified();

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onItemClicked(QListWidgetItem* item);
    void onRenameTimeout();
    void onItemChanged(QListWidgetItem* item);
    void showContextMenu(const QPoint& pos);
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void setupListWidget();
    void loadIcon();
    void startRename(QListWidgetItem* item);
    void removeSelectedItems();
    void addItem(const QString& name, int index);

    // UI Components
    QVBoxLayout* mainLayout_;
    QToolBar* toolbar_;
    QListWidget* listWidget_;
    QAction* addAction_;
    QAction* removeAction_;

    // Model reference
    CGWA* gwa_;

    // State
    ItemType currentItemType;
    QIcon currentIcon;
    QTimer* renameTimer;
    QListWidgetItem* lastClickedItem;
    QString oldItemName;  // Store original name before editing
};

#endif // ICONLISTWIDGET_H
