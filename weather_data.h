#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QString>

struct Today
{
  QString date = "0000-00-00";
  QString temperature = "null";
  QString city = "null";
  QString humidity = "无数据";
  QString pm25 = "无数据";
  QString quality = "无数据"; // 空气质量
  QString cold = "无数据";    // 感冒指数
  QString windDirect = "无数据";
  QString windPower = "无数据";
  QString type = "无数据"; // 天气
  QString sunrise = "00:00";
  QString sunset = "00:00";
  QString notice = "无数据";
};

struct Forecast
{
  QString date = "00";
  QString week = "无数据";
  QString high = "高温 0.0℃";
  QString low = "低温 0.0℃";
  QString quality = "0";      // 空气质量
  QString type = "undefined"; // 天气
};

#endif