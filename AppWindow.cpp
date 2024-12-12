#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include "Kits.h"
#include "RegDwordEdit.h"
#include "RegSzEdit.h"
#include "AppWindow.h"

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
    , m_CurMainKey(0)
    , m_CurSubKey(std::wstring(L""))
    , m_CurKeyItem(nullptr)
{
    ui.setupUi(this);

    InitWidget();
    InitKeyActions();
    InitValueActions();
}

AppWindow::~AppWindow()
{
}

std::wstring AppWindow::GetCurKey()
{
    return ui.le_key->text().toStdWString();
}

void AppWindow::InitWidget()
{
    ui.splitter->setSizes({ 300,700 });

    ui.tw_keys->clear();
    ui.tw_keys->setColumnCount(1);
    ui.tw_keys->setHeaderHidden(true);
    ui.tw_keys->header()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui.tw_keys->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
    ui.tw_keys->header()->setStretchLastSection(false);
    ui.tw_keys->setAutoScroll(true);
    ui.tw_keys->setContextMenuPolicy(Qt::CustomContextMenu);

    ui.tw_values->clear();
    ui.tw_values->setColumnCount(3);
    ui.tw_values->setHeaderLabels({ "名称","类型","数据" });
    ui.tw_values->setRootIsDecorated(false);
    ui.tw_values->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui.tw_values->setContextMenuPolicy(Qt::CustomContextMenu);
}

void AppWindow::InitKeyActions()
{
    // 遍历创建主键 Item
    for (const auto& it : WindowsRegistry::KeyStringToKeyHandle)
    {
        QTreeWidgetItem* pMainKey = new QTreeWidgetItem(ui.tw_keys, QStringList(QString::fromStdWString(it.first)));

        pMainKey->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        WindowsRegistry::EnumKeys(it.second, L"", [&](auto pKeyName) 
        {
            QTreeWidgetItem* pSubKey = new QTreeWidgetItem(pMainKey);
            pSubKey->setText(0, QString::fromStdWString(*pKeyName));
        });

        // 触发 Key Item 展开
        connect(ui.tw_keys, &QTreeWidget::itemExpanded, [&](QTreeWidgetItem* pItem)
        {
            LoadSubKeys(pItem);
        });

    }

    // 选中的 Key Item 改变
    connect(ui.tw_keys, &QTreeWidget::itemSelectionChanged, this, [&]() 
    {
        QList<QTreeWidgetItem*> lstSelectedItems = ui.tw_keys->selectedItems();

        if (lstSelectedItems.isEmpty()) { return; }

        m_CurKeyItem = lstSelectedItems.first();

        RenewalValueWidgets(m_CurKeyItem);
    });

    // 触发 Key Item 右键菜单
    connect(ui.tw_keys, &QTreeWidget::customContextMenuRequested, [&](const QPoint& pos)
    {
        QTreeWidgetItem* pItem = ui.tw_keys->itemAt(pos);

        if (!pItem) { return; }

        ShowKeyItemMenu(pItem, pos);
    });

}

void AppWindow::InitValueActions()
{
    // 触发 Value Widget 右键菜单
    connect(ui.tw_values, &QTreeWidget::customContextMenuRequested, [&](const QPoint& pos) 
    {
        QTreeWidgetItem* pItem = ui.tw_values->itemAt(pos);

        if (pItem) 
        {  
            // 触发 Value Item 右键菜单
            ShowValueItemMenu(pItem, pos);
        }
        else
        {
            // 触发 Value Widget 空白区域右键菜单
            ShowValueBlankAreaMenu(pos);      
        }
    });

    // 双击 Value Item 事件
    connect(ui.tw_values, &QTreeWidget::itemDoubleClicked, this, [&](QTreeWidgetItem* pItem, int column)
    {
        EditValueItem(pItem);
        RenewalValueWidgets(m_CurKeyItem);
    });
}

void AppWindow::LoadSubKeys(QTreeWidgetItem* pKeyItem)
{
    bool bIsMainKey = !pKeyItem->parent();

    if (bIsMainKey)
    {
        int n = 1000;

        qDeleteAll(pKeyItem->takeChildren());

        // 如果是主键
        HKEY hMainKey = WindowsRegistry::KeyStringToKeyHandle[pKeyItem->text(0).toStdWString()];
        WindowsRegistry::EnumKeys(hMainKey, L"", [&](auto pKeyName)
        {
            QTreeWidgetItem* pSubKey = new QTreeWidgetItem(pKeyItem);
            pSubKey->setText(0, QString::fromStdWString(*pKeyName));

            if (ExistSubkeys(hMainKey, pKeyName->c_str()))
            {
                QTreeWidgetItem* pSubSubKey = new QTreeWidgetItem(pSubKey);
                pSubSubKey->setText(0, "");
            }

            if (n-- < 0)
            {
                n = 1000;
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }

        });
    }
    else
    {
        qDeleteAll(pKeyItem->takeChildren());

        HKEY hMainKey = GetMainKey(pKeyItem);
        std::wstring strSubKey = GetSubKey(pKeyItem);
        WindowsRegistry::EnumKeys(hMainKey, strSubKey.c_str(), [&](auto pKeyName)
        {
            QTreeWidgetItem* pSubKey = new QTreeWidgetItem(pKeyItem);
            pSubKey->setText(0, QString::fromStdWString(*pKeyName));

            std::wstring strSubSubKey = strSubKey + std::wstring(L"\\") + (*pKeyName);

            if (ExistSubkeys(hMainKey, strSubSubKey.c_str()))
            {
                QTreeWidgetItem* pSubSubKey = new QTreeWidgetItem(pSubKey);
                pSubSubKey->setText(0, "");
            }

        });
    }
}

void AppWindow::ShowKeyItemMenu(QTreeWidgetItem* pKeyItem, const QPoint& pos)
{
    QMenu Menu;

    QAction* pNewKeyAction = Menu.addAction("新建 Key");
    QAction* pNewSzAction = Menu.addAction("新建 REG_SZ");
    QAction* pNewDwordAction = Menu.addAction("新建 REG_DWORD");
    QAction* pDeleteAction = Menu.addAction("删除");
    QAction* pRenameAction = Menu.addAction("重命名_");

    QAction* pSelAction = Menu.exec(ui.tw_keys->mapToGlobal(pos));

    if (pSelAction == pNewKeyAction)
    {
        bool bOk;
        QString strNewKey = QInputDialog::getText(nullptr, "info", "new key", QLineEdit::Normal, "", &bOk);
        if (!bOk) { return; }

        HKEY hMainKey = GetMainKey(pKeyItem);
        std::wstring strSubKey = GetSubKey(pKeyItem) + std::wstring(L"\\") + strNewKey.toStdWString();

        auto Error = WindowsRegistry::CreateKey(hMainKey, strSubKey.c_str());

        if (Error)
        {
            QMessageBox::information(this, "info", "创建失败");
            return;
        }

        pKeyItem->setExpanded(true);
        QTreeWidgetItem* pNewItem = new QTreeWidgetItem(pKeyItem);
        pNewItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        pNewItem->setText(0, strNewKey);

        LoadSubKeys(pKeyItem);
    }
    else if (pSelAction == pNewSzAction)
    {
        bool bOk;
        QString strNewSz = QInputDialog::getText(nullptr, "info", "new REG_SZ", QLineEdit::Normal, "", &bOk);
        if (!bOk) { return; }

        HKEY hMainKey = GetMainKey(pKeyItem);
        std::wstring strSubKey = GetSubKey(pKeyItem);

        WindowsRegistry::RegistryValue rvValue;
        rvValue.m_Name = strNewSz.toStdWString();
        rvValue.m_Type = REG_SZ;

        auto Error = WindowsRegistry::SetValue(hMainKey, strSubKey.c_str(), rvValue);

        if (Error)
        {
            QMessageBox::information(this, "info", "创建失败");
            return;
        }

        RenewalValueWidgets(pKeyItem);
    }
    else if (pSelAction == pNewDwordAction)
    {
        bool bOk;
        QString strNewDword = QInputDialog::getText(nullptr, "info", "new REG_DWORD", QLineEdit::Normal, "", &bOk);
        if (!bOk) { return; }

        HKEY hMainKey = GetMainKey(pKeyItem);
        std::wstring strSubKey = GetSubKey(pKeyItem);

        WindowsRegistry::RegistryValue rvValue;
        rvValue.m_Name = strNewDword.toStdWString();
        rvValue.m_Type = REG_DWORD;

        auto Error = WindowsRegistry::SetValue(hMainKey, strSubKey.c_str(), rvValue);

        if (Error)
        {
            QMessageBox::information(this, "info", "创建失败");
            return;
        }

        RenewalValueWidgets(pKeyItem);
    }
    else if (pSelAction == pDeleteAction)
    {
        if (pKeyItem->parent())
        {
            auto MsgCode = QMessageBox::information(this, "info", "删除Key可能会导致系统不稳定，确定要删除吗？", QMessageBox::Yes | QMessageBox::No);

            if (MsgCode != QMessageBox::Yes) { return; };

            HKEY hMainKey = GetMainKey(pKeyItem);
            std::wstring strSubKey = GetSubKey(pKeyItem);
            auto Error = WindowsRegistry::DeleteKey(hMainKey, strSubKey.c_str());

            if (Error)
            {
                QMessageBox::information(this, "info", "删除失败");
                return;
            }

            qDeleteAll(pKeyItem->takeChildren());
            QTreeWidgetItem* pParentItem = pKeyItem->parent();
            pParentItem->takeChild(pParentItem->indexOfChild(pKeyItem));
            delete pKeyItem;
        }
    }
    else if (pSelAction == pRenameAction)
    {
        if (pKeyItem->parent())
        {
            HKEY hMainKey = GetMainKey(pKeyItem);
            std::wstring strSubKey = GetSubKey(pKeyItem);

            bool bOk;
            QString strNewName = QInputDialog::getText(nullptr, "info", "new name", QLineEdit::Normal, "", &bOk);
            if (!bOk) { return; }

            auto Error = WindowsRegistry::RenameKey(hMainKey, strSubKey.c_str(), strNewName.toStdWString().c_str());

            if (Error)
            {
                QMessageBox::information(this, "info", "failed !");
                return;
            }

            pKeyItem->setText(0, strNewName);
        }
    }
}

void AppWindow::ShowValueItemMenu(QTreeWidgetItem* pValueItem, const QPoint& pos)
{
    QMenu Menu;

    QAction* pEditAction = Menu.addAction("修改");
    QAction* pDeleteAction = Menu.addAction("删除");
    QAction* pRenameAction = Menu.addAction("重命名_");

    QAction* pSelAction = Menu.exec(ui.tw_values->mapToGlobal(pos));

    if (pSelAction == pEditAction)
    {
        EditValueItem(pValueItem);
        RenewalValueWidgets(m_CurKeyItem);
    }
    else if (pSelAction == pDeleteAction)
    {
        auto MsgCode = QMessageBox::information(this, "info", "delete ?", QMessageBox::Yes | QMessageBox::No);

        if (MsgCode != QMessageBox::Yes) { return; }

        QString strKeyPath = ui.le_key->text();

        if (strKeyPath.isEmpty()) { return; }

        HKEY hMainKey = WindowsRegistry::KeyStringToKeyHandle[strKeyPath.left(strKeyPath.indexOf('\\')).toStdWString()];
        std::wstring strSubKey = strKeyPath.right(strKeyPath.size() - strKeyPath.indexOf('\\') - 1).toStdWString();

        auto Error = WindowsRegistry::DeleteValue(hMainKey, strSubKey.c_str(), pValueItem->text(0).toStdWString().c_str());

        if (Error)
        {
            QMessageBox::information(this, "info", "failed !");
            return;
        }

        ui.tw_values->takeTopLevelItem(ui.tw_values->indexOfTopLevelItem(pValueItem));

        delete pValueItem;
    }
    else if (pSelAction == pRenameAction)
    {
        QString strKeyPath = ui.le_key->text();

        if (strKeyPath.isEmpty()) { return; }

        HKEY hMainKey = WindowsRegistry::KeyStringToKeyHandle[strKeyPath.left(strKeyPath.indexOf('\\')).toStdWString()];
        std::wstring strSubKey = strKeyPath.right(strKeyPath.size() - strKeyPath.indexOf('\\') - 1).toStdWString();

        bool bOk;
        QString strNewName = QInputDialog::getText(nullptr, "info", "rename", QLineEdit::Normal, "", &bOk);
        if (!bOk) { return; }

        if (strNewName.isEmpty())
        {
            QMessageBox::information(this, "info", "名称不能为空");
            return;
        }

        auto Error = WindowsRegistry::RenameValue(hMainKey, strSubKey.c_str(), pValueItem->text(0).toStdWString().c_str(), strNewName.toStdWString().c_str());

        if (Error)
        {
            QMessageBox::information(this, "info", "failed !");
            return;
        }

        pValueItem->setText(0, strNewName);
    }
}

void AppWindow::ShowValueBlankAreaMenu(const QPoint& pos)
{
    QMenu Menu;

    QAction* pNewSzAction = Menu.addAction("新建 REG_SZ");
    QAction* pNewDwordAction = Menu.addAction("新建 REG_DWORD");

    QAction* pSelAction = Menu.exec(ui.tw_values->mapToGlobal(pos));

    if (pSelAction == pNewSzAction)
    {

    }
    else if (pSelAction == pNewDwordAction)
    {

    }
}

void AppWindow::RenewalValueWidgets(const QTreeWidgetItem* pNewItem)
{
    ui.tw_values->clear();

    m_CurSubKey = GetSubKey(pNewItem);
    m_CurMainKey = GetMainKey(pNewItem);

    ui.le_key->setText(QString::fromStdWString(WindowsRegistry::KeyHandleToKeyString[m_CurMainKey] + L"\\" + m_CurSubKey));

    WindowsRegistry::EnumValues(m_CurMainKey, m_CurSubKey.c_str(), [&](auto pRegistryValue)
    {
        QTreeWidgetItem* pValueItem = new QTreeWidgetItem(ui.tw_values);
        pValueItem->setText(0, QString::fromStdWString(pRegistryValue->m_Name));
        pValueItem->setText(1, QString::fromStdWString(WindowsRegistry::ValueTypeToValueTypeString[pRegistryValue->m_Type]));
        pValueItem->setText(2, QString::fromStdWString(RVDataToString(pRegistryValue->m_Type, pRegistryValue->m_Data)));
    });
}

void AppWindow::EditValueItem(QTreeWidgetItem* pValueItem)
{
    DWORD dwType = WindowsRegistry::ValueTypeStringToValueType[pValueItem->text(1).toStdWString()];

    if (REG_SZ == dwType)
    {
        RegSzEdit(this, pValueItem).exec();
    }
    else if (REG_DWORD == dwType)
    {
        RegDwordEdit(this, pValueItem).exec();
    }

}
