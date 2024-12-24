#include "weather_widget.h"
#include "ui_weather_widget.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>

WeatherWidget::WeatherWidget(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::Weather)
{
  ui->setupUi(this);
  this->setWindowFlag(Qt::FramelessWindowHint);

  m_menu = new QMenu(this);
  m_actExit = new QAction;
  m_actExit->setText("退出");
  m_actExit->setIcon(QIcon(":/weatherIco/close.ico"));
  m_menu->addAction(m_actExit);
  connect(m_actExit, &QAction::triggered, this, &WeatherWidget::onExitApp);
}

WeatherWidget::~WeatherWidget()
{
  delete ui;
}

void
WeatherWidget::contextMenuEvent(QContextMenuEvent* event)
{
  m_menu->exec(QCursor::pos());
  event->accept();
}

void
WeatherWidget::mouseMoveEvent(QMouseEvent* event)
{
  move(event->globalPos() - m_pos);
}

void
WeatherWidget::mousePressEvent(QMouseEvent* event)
{
  m_pos = event->globalPos() - pos();
}

void
WeatherWidget::onExitApp()
{
  qApp->exit();
}