#pragma once

#include <QtWidgets/QWidget>
#include <QTreeWidgetItem>
#include <Windows.h>
#include "ui_AppWindow.h"

class AppWindow : public QWidget
{
    Q_OBJECT

public:
    AppWindow(QWidget *parent = nullptr);
    ~AppWindow();
    
public:
    std::wstring GetCurKey();

private:
    void InitWidget();
    void InitKeyActions();
    void InitValueActions();

private:
    void LoadSubKeys(QTreeWidgetItem* pKeyItem);
    void ShowKeyItemMenu(QTreeWidgetItem* pKeyItem, const QPoint& pos);
    void ShowValueItemMenu(QTreeWidgetItem* pValueItem, const QPoint& pos);
    void ShowValueBlankAreaMenu(const QPoint& pos);
    void RenewalValueWidgets(const QTreeWidgetItem* pNewItem);
    void EditValueItem(QTreeWidgetItem* pValueItem);

private:
    HKEY m_CurMainKey;
    std::wstring m_CurSubKey;
    QTreeWidgetItem* m_CurKeyItem;

private:
    Ui::AppWindowClass ui;
};
