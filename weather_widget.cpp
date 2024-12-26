#include "ui_weather_widget.h"
#include "weather_widget.h"

#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

static const QPoint sun[2] = { QPoint(20, 75), QPoint(130, 75) };
static const QRect sunRiseSet[2] = { QRect(0, 80, 50, 20),
                                     QRect(100, 80, 50, 20) };
static const QRect rect[2] = {
  QRect(25, 25, 100, 100), // 虚线圆弧
  QRect(50, 80, 50, 20)    // “日出日落”文本
};

static constexpr int TEMPERATURE_STARTING_COORDINATE = 60;
static constexpr int ORIGIN_SIZE = 3;

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

  ui->lbSunRiseSet->installEventFilter(this);
  ui->lbCurve->installEventFilter(this);

  QTimer* sunTimer = new QTimer(this);
  connect(sunTimer, &QTimer::timeout, [this]() { ui->lbSunRiseSet->update(); });
  sunTimer->start(1000);

  connect(
    ui->tbSearch, &QToolButton::clicked, this, &WeatherWidget::onSearchCity);
  connect(ui->tbRefresh,
          &QToolButton::clicked,
          this,
          &WeatherWidget::onRefreshWeather);
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

bool
WeatherWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == ui->lbSunRiseSet && event->type() == QEvent::Paint) {
    paintSunRiseSet();
  } else if (watched == ui->lbCurve && event->type() == QEvent::Paint) {
    paintCurve();
  }
  return QWidget::eventFilter(watched, event);
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
WeatherWidget::onSearchCity()
{
  m_tmpCity = m_city;
  m_city = ui->leCity->text();
  getWeatherInfo(m_manager);
}

void
WeatherWidget::onRefreshWeather()
{
  getWeatherInfo(m_manager);
  ui->lbCurve->update();
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

  ui->lbCurve->update();
}

void
WeatherWidget::paintSunRiseSet()
{
  QPainter painter(ui->lbSunRiseSet);
  painter.setRenderHint(QPainter::Antialiasing);

  // 绘制横线
  painter.save();
  QPen pen = painter.pen();
  pen.setWidthF(0.5f);
  pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawLine(sun[0], sun[1]);
  painter.restore();

  // 绘制日出日落时间
  painter.save();
  painter.setFont(QFont("Microsoft YaHei", 8, QFont::Normal));
  painter.setPen(Qt::white);
  if (!today.sunrise.isEmpty() && !today.sunset.isEmpty()) {
    painter.drawText(sunRiseSet[0], Qt::AlignHCenter, today.sunrise);
    painter.drawText(sunRiseSet[1], Qt::AlignHCenter, today.sunset);
  }
  painter.drawText(::rect[1], Qt::AlignHCenter, "日出日落");
  painter.restore();

  // 绘制弧形
  painter.save();
  pen.setWidthF(0.5f);
  pen.setStyle(Qt::DotLine);
  pen.setColor(Qt::green);
  painter.setPen(pen);
  painter.drawArc(::rect[0], 0 * 16, 180 * 16);
  if (!today.sunrise.isEmpty() && !today.sunset.isEmpty()) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 85, 0, 100));

    int startAngle, spanAngle;
    if (QTime::currentTime() > QTime::fromString(today.sunset, "HH:mm")) {
      startAngle = 0 * 16;
      spanAngle = 180 * 16;
    } else {
      static int sunrise =
        QTime::fromString(today.sunrise, "HH:mm").hour() * 60 +
        QTime::fromString(today.sunrise, "HH:mm").minute();
      static int sunset = QTime::fromString(today.sunset, "HH:mm").hour() * 60 +
                          QTime::fromString(today.sunset, "HH:mm").minute();

      int currentTime =
        QTime::currentTime().hour() * 60 + QTime::currentTime().minute();

      startAngle =
        (static_cast<double>(sunset - currentTime) / (sunset - sunrise)) * 180 *
        16;
      spanAngle =
        (static_cast<double>(currentTime - sunrise) / (sunset - sunrise)) *
        180 * 16;

      if (startAngle >= 0 && spanAngle >= 0) {
        painter.drawPie(::rect[0], startAngle, spanAngle);
      }
    }
  }
  painter.restore();
}

void
WeatherWidget::paintCurve()
{
  int tempMax = std::numeric_limits<int>::min();
  int tempMin = std::numeric_limits<int>::max();
  int high[6], low[6];
  for (int i = 0; i < 6; ++i) {
    QString temp = forecastHighList[i]->text();
    high[i] = temp.left(temp.length() - 1).toInt();
    tempMax = std::max(tempMax, high[i]);
    tempMin = std::min(tempMin, high[i]);

    temp = forecastLowList[i]->text();
    low[i] = temp.left(temp.length() - 1).toInt();
    tempMax = std::max(tempMax, low[i]);
    tempMin = std::min(tempMin, low[i]);
  }
  int tempAvg = (tempMax + tempMin) / 2;
  int spanSize = 100 / (tempMax - tempMin);

  int pointX[6] = { 35, 105, 175, 245, 315, 385 };
  int pointHY[6], pointLY[6];
  for (int i = 0; i < 6; ++i) {
    pointHY[i] =
      TEMPERATURE_STARTING_COORDINATE - ((high[i] - tempAvg) * spanSize);
    pointLY[i] =
      TEMPERATURE_STARTING_COORDINATE + ((tempAvg - low[i]) * spanSize);
  }

  QPainter painter(ui->lbCurve);
  painter.setRenderHint(QPainter::Antialiasing);
  QPen pen = painter.pen();
  pen.setWidth(1);

  // 高温
  painter.save();
  // 昨天->今天
  pen.setColor(QColor(255, 170, 0));
  pen.setStyle(Qt::DotLine);
  painter.setPen(pen);
  painter.setBrush(QColor(255, 170, 0));
  painter.drawEllipse(QPoint(pointX[0], pointHY[0]), ORIGIN_SIZE, ORIGIN_SIZE);
  painter.drawEllipse(QPoint(pointX[1], pointHY[1]), ORIGIN_SIZE, ORIGIN_SIZE);
  painter.drawLine(pointX[0], pointHY[0], pointX[1], pointHY[1]);
  // 今天->后面几天
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  painter.setPen(pen);
  for (int i = 1; i < 5; ++i) {
    painter.drawEllipse(
      QPoint(pointX[i + 1], pointHY[i + 1]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawLine(pointX[i], pointHY[i], pointX[i + 1], pointHY[i + 1]);
  }
  painter.restore();

  // 低温
  painter.save();
  // 昨天->今天
  pen.setColor(QColor(0, 255, 255));
  pen.setStyle(Qt::DotLine);
  painter.setPen(pen);
  painter.setBrush(QColor(0, 255, 255));
  painter.drawEllipse(QPoint(pointX[0], pointLY[0]), ORIGIN_SIZE, ORIGIN_SIZE);
  painter.drawEllipse(QPoint(pointX[1], pointLY[1]), ORIGIN_SIZE, ORIGIN_SIZE);
  painter.drawLine(pointX[0], pointLY[0], pointX[1], pointLY[1]);
  // 今天->后面几天
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  painter.setPen(pen);
  for (int i = 1; i < 5; ++i) {
    painter.drawEllipse(
      QPoint(pointX[i + 1], pointLY[i + 1]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawLine(pointX[i], pointLY[i], pointX[i + 1], pointLY[i + 1]);
  }
  painter.restore();
}
