#include "Kits.h"
#include "RegSzEdit.h"

RegSzEdit::RegSzEdit(QWidget* parent, const QTreeWidgetItem* pItem)
    : QDialog()
    , m_pWin(dynamic_cast<AppWindow*>(parent))
    , m_CurValue(pItem->text(2))
    , m_NewValue(m_CurValue)
    , m_ValueName(pItem->text(0))
{
    ui.setupUi(this);
    
    InitView();
    InitAction();
}

RegSzEdit::~RegSzEdit()
{
}

void RegSzEdit::InitView()
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui.le_name->setText(m_ValueName);
    ui.le_value->setText(m_CurValue);
}

void RegSzEdit::InitAction()
{
    connect(ui.le_value, &QLineEdit::textChanged, [&](const QString& text) 
    {
        m_NewValue = text;
    });

    connect(ui.pb_ok, &QPushButton::clicked, [&]()
    {
        if (m_NewValue == m_CurValue)
        {
            close();
            return;
        }

        WindowsRegistry::RegistryValue rvValue;

        rvValue.m_Name = m_ValueName.toStdWString();
        rvValue.m_Type = REG_SZ;
        rvValue.m_Data = RVDataFromString(REG_SZ, m_NewValue.toStdWString());

        auto Key = m_pWin->GetCurKey();

        auto Error = WindowsRegistry::SetValue(ExtractMainKey(Key), ExtractSubKey(Key).c_str(), rvValue);

        if (Error)
        {
            QMessageBox::information(this, "info", "failed");
        }

        close();
    });

    connect(ui.pb_cancle, &QPushButton::clicked, [&]()
    {
        close();
    });
}
