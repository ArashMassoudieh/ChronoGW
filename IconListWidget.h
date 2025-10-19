#ifndef ICONLISTWIDGET_H
#define ICONLISTWIDGET_H

#include <QListWidget>
#include <QTimer>
#include <QString>

// Forward declaration
class CGWA;

/**
 * @brief A reusable widget for displaying items (wells, tracers, parameters, observations)
 *        as icons with text labels
 *
 * Features:
 * - Icon view with text labels
 * - Double-click to edit/view properties
 * - Single-click + wait or F2 to rename
 * - Context menu support (right-click)
 */
class IconListWidget : public QListWidget
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
     * @brief Set the type of items this widget will display
     * @param type The type (Well, Tracer, Parameter, or Observation)
     */
    void setItemType(ItemType type);

    /**
     * @brief Add a new item to the list
     * @param name The name/label of the item
     * @param data Optional user data associated with the item
     */
    void addItem(const QString& name, const QVariant& data = QVariant());

    /**
     * @brief Populate widget from GWA model
     * @param gwa Pointer to GWA model
     * @param type Type of items to display (Well, Tracer, Parameter, or Observation)
     */
    void populateFromGWA(CGWA* gwa, ItemType type);

    /**
     * @brief Get all item names in the list
     * @return List of item names
     */
    QStringList getItemNames() const;

    /**
     * @brief Remove selected item(s)
     */
    void removeSelectedItems();

signals:
    /**
     * @brief Emitted when user double-clicks an item to edit properties
     * @param itemName The name of the item
     * @param userData The associated user data
     */
    void itemPropertiesRequested(const QString& itemName, const QVariant& userData);

    /**
     * @brief Emitted when user renames an item
     * @param oldName The previous name
     * @param newName The new name
     * @param userData The associated user data
     */
    void itemRenamed(const QString& oldName, const QString& newName, const QVariant& userData);

    /**
     * @brief Emitted when user requests to delete an item
     * @param itemName The name of the item to delete
     * @param userData The associated user data
     */
    void itemDeleteRequested(const QString& itemName, const QVariant& userData);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onRenameTimeout();
    void onItemChanged(QListWidgetItem* item);
    void showContextMenu(const QPoint& pos);

private:
    void setupWidget();
    void loadIcon();
    void startRename(QListWidgetItem* item);

    ItemType currentItemType;
    QIcon currentIcon;
    QTimer* renameTimer;
    QListWidgetItem* lastClickedItem;
    QString oldItemName;  // Store original name before editing
};

#endif // ICONLISTWIDGET_H
