#pragma once

#include <QtWidgets/QDialog>
#include <QTreeWidgetItem>
#include <Windows.h>
#include "AppWindow.h"
#include "ui_RegDwordEdit.h"

class RegDwordEdit : public QDialog
{
	Q_OBJECT
public:
	RegDwordEdit(QWidget* parent, const QTreeWidgetItem* pItem);
	~RegDwordEdit();

private:
	void InitView();
	void InitAction();

private:
	bool m_bValid;
	QString m_ValueName;
	DWORD m_CurValue;
	DWORD m_NewValue;
	AppWindow* m_pWin;

private:
	Ui::RegDwordEditClass ui;
};

