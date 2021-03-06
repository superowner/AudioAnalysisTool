#include "TabControl.h"
#include <QVBoxLayout>
#include <QTabWidget>

TabControl::TabControl()
{

}

void TabControl::AddControl(QVBoxLayout* vBoxLayout)
{
	tabControl_ = new QTabWidget();
	vBoxLayout->addWidget(tabControl_);
}

QWidget* TabControl::AddTab(const std::string& tabName)
{
	auto tab = new QWidget();
	tabControl_->addTab(tab, tabName.c_str());
	return tab;
}

void TabControl::Resize(std::size_t width, std::size_t height)
{
	tabControl_->resize(width, height);
}