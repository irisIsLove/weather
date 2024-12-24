cmake_minimum_required(VERSION 3.15.0)
project(WeatherApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options("/utf-8")

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_AUTOUIC TRUE)
set(CMAKE_AUTORCC TRUE)

set(ResouceFile icons/icons.qrc images/images.qrc weatherIcons/weatypeico.qrc)

add_executable(WeatherApp main.cpp weather_widget.cpp ${ResouceFile})

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(WeatherApp PRIVATE Qt5::Widgets)
