#include "Kits.h"
#include "RegDwordEdit.h"

RegDwordEdit::RegDwordEdit(QWidget* parent, const QTreeWidgetItem* pItem)
    : QDialog()
    , m_pWin(dynamic_cast<AppWindow*>(parent))
    , m_CurValue(HexStringToDWORD(pItem->text(2).toStdWString()))
    , m_NewValue(m_CurValue)
    , m_ValueName(pItem->text(0))
    , m_bValid(false)
{
    ui.setupUi(this);
    
    InitView();
    InitAction();
}

RegDwordEdit::~RegDwordEdit()
{
}

void RegDwordEdit::InitView()
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui.le_name->setText(m_ValueName);
    ui.le_value->setText(QString::number(m_CurValue, 16));
}

void RegDwordEdit::InitAction()
{
    connect(ui.rb_hex, &QRadioButton::toggled, [&](bool bChecked)
    {
        if (bChecked)
        {
            m_NewValue = ui.le_value->text().toInt(&m_bValid, 10);
            ui.le_value->setText(QString::number(m_NewValue, 16));
        }
    });

    connect(ui.rb_dec, &QRadioButton::toggled, [&](bool bChecked)
    {
        if (bChecked)
        {
            m_NewValue = m_NewValue = ui.le_value->text().toInt(&m_bValid, 16);
            ui.le_value->setText(QString::number(m_NewValue, 10));
        }
    });

    connect(ui.pb_ok, &QPushButton::clicked, [&]()
    {
        if (ui.rb_hex->isChecked())
        {
            emit ui.rb_dec->toggled(true);
        }
        else
        {
            emit ui.rb_hex->toggled(true);
        }

        if (!m_bValid)
        {
            QMessageBox::information(this, "info", "输入的值无效_");
            return;
        }

        if (m_NewValue == m_CurValue)
        {
            close();
            return;
        }

        WindowsRegistry::RegistryValue rvValue;

        rvValue.m_Name = m_ValueName.toStdWString();
        rvValue.m_Type = REG_DWORD;
        rvValue.m_Data = RVDataFromString(REG_DWORD, DWORDToHexString(m_NewValue));

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
