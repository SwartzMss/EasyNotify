#include "ui/widgets/reminderliststyler.h"

namespace {
void styleButton(QPushButton *button,
                 const QString &bg,
                 const QString &fg,
                 const QString &border,
                 const QString &hoverBg)
{
    if (!button) {
        return;
    }
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    border-radius: 6px;"
        "    padding: 6px 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %4;"
        "}"
    ).arg(bg, fg, border, hoverBg));
}
} // namespace

namespace ReminderListStyler {

void apply(QLineEdit *searchEdit,
           QPushButton *primaryButton,
           QPushButton *secondaryButton,
           QTableView *tableView)
{
    if (searchEdit) {
        searchEdit->setClearButtonEnabled(true);
        searchEdit->setStyleSheet(QStringLiteral(
            "QLineEdit {"
            "    border: 1px solid #d5deef;"
            "    border-radius: 18px;"
            "    padding: 6px 30px 6px 12px;"
            "    background-color: #ffffff;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #2563eb;"
            "    background-color: #ffffff;"
            "}"
        ));
    }

    styleButton(primaryButton, "#2563eb", "#ffffff", "#1d4ed8", "#1d4ed8");
    styleButton(secondaryButton, "#ffffff", "#111827", "#e5e7eb", "#f3f4f6");

    if (tableView) {
        tableView->setAlternatingRowColors(true);
        tableView->setStyleSheet(QStringLiteral(
            "QTableView {"
            "    border: 1px solid #e0e6f4;"
            "    border-radius: 12px;"
            "    background-color: #ffffff;"
            "    gridline-color: transparent;"
            "    selection-background-color: #e3f2ff;"
            "    selection-color: #111827;"
            "}"
            "QTableView::item {"
            "    padding: 6px;"
            "}"
            "QHeaderView::section {"
            "    background-color: #f5f7fb;"
            "    border: none;"
            "    padding: 8px 4px;"
            "    font-weight: 600;"
            "    color: #1f2937;"
            "}"
        ));
    }
}

} // namespace ReminderListStyler
