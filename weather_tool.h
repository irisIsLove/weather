#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

class WeatherTool
{
public:
  WeatherTool()
  {
    QJsonParseError err;
    QString filename = "city_code.json";
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QByteArray json = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    QJsonArray citys = jsonDoc.array();
    for (const auto& city : citys) {
      QString code = city.toObject().value("city_code").toString();
      QString name = city.toObject().value("city_name").toString();

      if (!code.isEmpty())
        m_mapCity2Code.insert(name, code);
    }
  }

  QString operator[](const QString& city)
  {
    auto it = m_mapCity2Code.find(city);

    if (it != m_mapCity2Code.end())
      return it.value();

    return "000000000";
  }

  static QString getWeather(const QString& type)
  {
    if (type == "多云")
      return "Cloudy";
    else if (type == "雾")
      return "Foggy";
    else if (type == "雪")
      return "Snow";
    else if (type == "晴")
      return "Sunny";
    else if (type == "沙尘暴")
      return "Sandstorm";
    else if (type == "中雨")
      return "ModerateRain";
    else if (type == "中雪")
      return "ModerateSnow";
    else if (type == "雷阵雨")
      return "Thundershower";
    else if (type == "雷阵雨伴有冰雹")
      return "ThundershowerWithHail";
    else if (type == "大雪")
      return "HeavySnow";
    else if (type == "大雨")
      return "HeavyRain";
    else if (type == "冻雨")
      return "FreezingRain";
    else if (type == "小雨")
      return "LightRain";
    else if (type == "雨")
      return "Rain";
    else if (type == "小雪")
      return "LightSnow";
    else if (type == "小到中雨")
      return "LightToModerateRain";
    else if (type == "小到中雪")
      return "LightToModerateSnow";
    else if (type == "中到大雨")
      return "ModerateToHeavyRain";
    else if (type == "中到大雪")
      return "ModerateToHeavySnow";
    else if (type == "阴")
      return "Overcast";
    else if (type == "阵雨")
      return "ShowerRain";
    else if (type == "雨夹雪")
      return "Sleet";
    else if (type == "暴雨")
      return "Storm";
    else if (type == "暴雨到大暴雨")
      return "StormToHeavyStorm";
    else if (type == "阵雪")
      return "SnowFlurry";
    else if (type == "霾")
      return "Haze";
    else if (type == "暴雪")
      return "Snowstorm";
    else if (type == "大到暴雪")
      return "HeavySnowToSnowstorm";
    else if (type == "强沙尘暴")
      return "HeavySandstorm";
    else if (type == "浮尘")
      return "Dust";
    else if (type == "扬沙")
      return "Duststorm";
    else if (type == "大暴雨")
      return "HeavyRainCopy";
    else if (type == "特大暴雨")
      return "SevereStorm";
    else if (type == "大到暴雨")
      return "HeavyToStorm";
    else if (type == "大暴雨到特大暴雨")
      return "HeavyToSevereStorm";
    else
      return type;
  }

private:
  QMap<QString, QString> m_mapCity2Code;
};

#endif