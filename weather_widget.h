#ifndef WEATHERWIDGETS_H
#define WEATHERWIDGETS_H

#include "weather_data.h"
#include "weather_tool.h"

#include <QWidget>

namespace Ui {
class Weather;
}

class QMenu;
class QAction;
class QNetworkAccessManager;
class QNetworkReply;
class QLabel;

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
  bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
  void onExitApp();
  void onReplyFinished(QNetworkReply* reply);
  void onSearchCity();
  void onRefreshWeather();

private:
  void getWeatherInfo(QNetworkAccessManager* manager);
  void parseJson(const QByteArray& data);
  void setLabelContent();
  void paintSunRiseSet();
  void paintCurve();

private:
  Ui::Weather* ui = nullptr;
  QMenu* m_menu = nullptr;
  QAction* m_actExit = nullptr;
  QNetworkAccessManager* m_manager = nullptr;
  QPoint m_pos{};

  QString m_url = "http://t.weather.itboy.net/api/weather/city/";
  QString m_city = "重庆";
  QString m_tmpCity = m_city;

  WeatherTool m_tool;

  QList<QLabel*> forecastWeekList;
  QList<QLabel*> forecastDateList;
  QList<QLabel*> forecastQualityList;
  QList<QLabel*> forecastTypeList;
  QList<QLabel*> forecastTypeIcoList;
  QList<QLabel*> forecastHighList;
  QList<QLabel*> forecastLowList;

  Today today;
  Forecast forecast[6];
};

#endif // WEATHERWIDGETS_H
