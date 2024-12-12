#pragma once

#include <QtWidgets/QDialog>
#include <QTreeWidgetItem>
#include <Windows.h>
#include "AppWindow.h"
#include "ui_RegSzEdit.h"

class RegSzEdit : public QDialog
{
	Q_OBJECT
public:
	RegSzEdit(QWidget* parent, const QTreeWidgetItem* pItem);
	~RegSzEdit();

private:
	void InitView();
	void InitAction();

private:
	QString m_ValueName;
	QString m_CurValue;
	QString m_NewValue;
	AppWindow* m_pWin;

private:
	Ui::RegSzEditClass ui;
};

