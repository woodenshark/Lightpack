#include "SelectWidget.hpp"
#include <QScrollBar>
#include <QResizeEvent>
SelectWidget::SelectWidget() {
	list = new QListWidget(this);
	upButton = new QPushButton(this);
	downButton = new QPushButton(this);
	connect(upButton, &QPushButton::clicked, this, &SelectWidget::MoveUp);
	connect(downButton, &QPushButton::clicked, this, &SelectWidget::MoveDown);
	list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
SelectWidget::SelectWidget(QWidget *parent) {
	this->setParent(parent);
	list = new QListWidget(this);
	upButton = new QPushButton(this);
	downButton = new QPushButton(this);
	connect(upButton, &QPushButton::clicked, this, &SelectWidget::MoveUp);
	connect(downButton, &QPushButton::clicked, this, &SelectWidget::MoveDown);
	list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
void SelectWidget::MoveUp() {
	list->verticalScrollBar()->setValue(list->verticalScrollBar()->value()-1);
}
void SelectWidget::MoveDown() {
	list->verticalScrollBar()->setValue(list->verticalScrollBar()->value()+1);
}
void SelectWidget::resizeEvent(QResizeEvent *e) {
	upButton->setGeometry(0, 0, e->size().width(), e->size().height()*0.15);
	downButton->setGeometry(0, e->size().height()*0.85, e->size().width(), e->size().height()*0.15);
	list->setGeometry(e->size().width()*0.1, e->size().height()*0.15, e->size().width()*0.8, e->size().height()*0.7);
}
SelectWidget::~SelectWidget() {
}
