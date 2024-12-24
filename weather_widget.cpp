#include "weather_widget.h"
#include "ui_weather_widget.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QNetworkReply>

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

  forecastWeekList << ui->lbWeek0 << ui->lbWeek1 << ui->lbWeek2 << ui->lbWeek3
                   << ui->lbWeek4 << ui->lbWeek5;
  forecastDateList << ui->lbDate0 << ui->lbDate1 << ui->lbDate2 << ui->lbDate3
                   << ui->lbDate4 << ui->lbDate5;
  forecastQualityList << ui->lbQuality0 << ui->lbQuality1 << ui->lbQuality2
                      << ui->lbQuality3 << ui->lbQuality4 << ui->lbQuality5;
  forecastTypeList << ui->lbType0 << ui->lbType1 << ui->lbType2 << ui->lbType3
                   << ui->lbType4 << ui->lbType5;
  forecastTypeIcoList << ui->lbIcoType0 << ui->lbIcoType1 << ui->lbIcoType2
                      << ui->lbIcoType3 << ui->lbIcoType4 << ui->lbIcoType5;
  forecastHighList << ui->lbHigh0 << ui->lbHigh1 << ui->lbHigh2 << ui->lbHigh3
                   << ui->lbHigh4 << ui->lbHigh5;
  forecastLowList << ui->lbLow0 << ui->lbLow1 << ui->lbLow2 << ui->lbLow3
                  << ui->lbLow4 << ui->lbLow5;

  for (int i = 0; i < 6; ++i) {
    forecastDateList[i]->setStyleSheet(
      "background-color:rgba(0, 255, 255, 100);");
    forecastWeekList[i]->setStyleSheet(
      "background-color:rgba(0, 255, 255, 100);");
  }
  ui->leCity->setStyleSheet(
    "QLineEdit{border: 1px solid gray; border-radius: 4px; background: "
    "rgba(47, "
    "47, 47, 130); color:rgb(255, 255, 255);} QLineEdit:hover{border-color: "
    "rgb(101, 255, 106);}");

  m_manager = new QNetworkAccessManager(this);
  connect(m_manager,
          &QNetworkAccessManager::finished,
          this,
          &WeatherWidget::onReplyFinished);
  getWeatherInfo(m_manager);
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

void
WeatherWidget::onReplyFinished(QNetworkReply* reply)
{
  QVariant statusCode =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (reply->error() != QNetworkReply::NoError || statusCode != 200) {
    QMessageBox::warning(
      this, "错误", "天气：请求数据错误，检查网络连接！", QMessageBox::Ok);
    return;
  }

  QByteArray bytes = reply->readAll();
  parseJson(bytes);
}

void
WeatherWidget::getWeatherInfo(QNetworkAccessManager* manager)
{
  QString cityCode = m_tool[m_city];
  if (cityCode == "000000000") {
    QMessageBox::warning(
      this, "错误", "天气：指定城市不存在！", QMessageBox::Ok);
    return;
  }

  QUrl jsonUrl(m_url + cityCode);
  manager->get(QNetworkRequest(jsonUrl));
}

void
WeatherWidget::parseJson(const QByteArray& data)
{
  QJsonParseError err;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError)
    return;

  QJsonObject jsonObj = jsonDoc.object();
  QString message = jsonObj["message"].toString();
  if (!message.contains("success")) {
    QMessageBox::information(this,
                             "The information of json_desc",
                             "天气：城市错误！",
                             QMessageBox::Ok);
    m_city = m_tmpCity;
    return;
  }

  QString qsDate = jsonObj["date"].toString();
  today.date = QDate::fromString(qsDate, "yyyyMMdd").toString("yyyy-MM-dd");
  today.city = jsonObj["cityInfo"].toObject()["city"].toString();

  QJsonObject dataObj = jsonObj["data"].toObject();
  today.humidity = dataObj["shidu"].toString();
  today.pm25 = QString::number(dataObj["pm25"].toDouble());
  today.quality = dataObj["quality"].toString();
  today.temperature = dataObj["wendu"].toString();
  today.cold = dataObj["ganmao"].toString();

  QJsonObject yesterdayObj = dataObj["yesterday"].toObject();
  forecast[0].date = yesterdayObj["date"].toString();
  forecast[0].high = yesterdayObj["high"].toString();
  forecast[0].low = yesterdayObj["low"].toString();
  forecast[0].quality = QString::number(yesterdayObj["aqi"].toDouble());
  forecast[0].type = yesterdayObj["type"].toString();
  forecast[0].week = "昨天";

  QJsonArray forecastArr = dataObj["forecast"].toArray();
  for (int i = 1; i < 6; ++i) {
    QJsonObject forecastObj = forecastArr[i - 1].toObject();
    forecast[i].date = forecastObj["date"].toString();
    forecast[i].high = forecastObj["high"].toString();
    forecast[i].low = forecastObj["low"].toString();
    forecast[i].quality = QString::number(forecastObj["aqi"].toDouble());
    forecast[i].type = forecastObj["type"].toString();
    forecast[i].week = i == 1 ? "今天" : forecastObj["week"].toString();
  }

  QJsonObject todayObj = forecastArr[0].toObject();
  today.windDirect = todayObj["fx"].toString();
  today.windPower = todayObj["fl"].toString();
  today.type = todayObj["type"].toString();
  today.sunrise = todayObj["sunrise"].toString();
  today.sunset = todayObj["sunset"].toString();
  today.notice = todayObj["notice"].toString();

  setLabelContent();
}

void
WeatherWidget::setLabelContent()
{
  QTime sunrise = QTime::fromString(today.sunrise, "HH:mm");
  QTime sunset = QTime::fromString(today.sunset, "HH:mm");
  QString now =
    (QTime::currentTime() > sunrise && QTime::currentTime() < sunset) ? "day"
                                                                      : "night";

  ui->lbDate->setText(today.date);
  ui->lbTemperature->setText(today.temperature);
  ui->lbCity->setText(today.city);
  ui->lbType->setText(today.type);
  ui->lbNotice->setText(today.notice);
  ui->lbHumidity->setText(today.humidity);
  ui->lbPm->setText(today.pm25);
  ui->lbWindDirect->setText(today.windDirect);
  ui->lbWindPower->setText(today.windPower);
  ui->tbCold->setText(today.cold);
  ui->lbTypeIco->setStyleSheet(QString("border-image: url(:/%1/%2.png);"
                                       "background-color: rgba(60, 60, 60, 0);")
                                 .arg(now)
                                 .arg(WeatherTool::getWeather(today.type)));

  for (int i = 0; i < 6; ++i) {
    forecastWeekList[i]->setText(forecast[i].week);
    forecastDateList[i]->setText(forecast[i].date);
    forecastTypeList[i]->setText(forecast[i].type);
    forecastHighList[i]->setText(forecast[i].high.split(" ").at(1));
    forecastLowList[i]->setText(forecast[i].low.split(" ").at(1));
    forecastTypeIcoList[i]->setStyleSheet(
      QString("image:url(:/%1/%2.png)")
        .arg(now)
        .arg(WeatherTool::getWeather(forecast[i].type)));

    if (forecast[i].quality >= 0 && forecast[i].quality <= 50) {
      forecastQualityList[i]->setText("优质");
      forecastQualityList[i]->setStyleSheet("color: rgb(0, 255, 0);");
    } else if (forecast[i].quality > 50 && forecast[i].quality <= 100) {
      forecastQualityList[i]->setText("良好");
      forecastQualityList[i]->setStyleSheet("color: rgb(255, 255, 0);");
    } else if (forecast[i].quality > 100 && forecast[i].quality <= 150) {
      forecastQualityList[i]->setText("轻度污染");
      forecastQualityList[i]->setStyleSheet("color: rgb(255, 170, 0);");
    } else if (forecast[i].quality > 150 && forecast[i].quality <= 200) {
      forecastQualityList[i]->setText("重度污染");
      forecastQualityList[i]->setStyleSheet("color: rgb(255, 0, 0);");
    } else {
      forecastQualityList[i]->setText("严重污染");
      forecastQualityList[i]->setStyleSheet("color: rgb(170, 0, 0);");
    }
  }
}
