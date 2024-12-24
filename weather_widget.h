#ifndef WEATHERWIDGETS_H
#define WEATHERWIDGETS_H

#include <QWidget>

namespace Ui {
class Weather;
}

class QMenu;
class QAction;

class WeatherWidget : public QWidget
{
  Q_OBJECT
public:
  WeatherWidget(QWidget* parent = nullptr);
  ~WeatherWidget();

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

private slots:
  void onExitApp();

private:
  Ui::Weather* ui = nullptr;
  QMenu* m_menu = nullptr;
  QAction* m_actExit = nullptr;
  QPoint m_pos{};
};

#endif // WEATHERWIDGETS_H
