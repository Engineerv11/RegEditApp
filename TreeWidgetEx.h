#pragma once

#include <QTreeWidget>

class TreeWidgetEx : public QTreeWidget
{
	Q_OBJECT

public:
	TreeWidgetEx(QWidget*parent);
	~TreeWidgetEx();

	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
};
