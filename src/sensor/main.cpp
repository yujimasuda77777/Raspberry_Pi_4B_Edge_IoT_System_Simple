#include <iostream>

#include "common/SensorData.h"
#include "sensor/Dht11Sensor.h"

int main()
{
    std::cout
        << "Sensor Process Start"
        << std::endl;

    // GPIO14に接続されたDHT11センサを作成する
    Dht11Sensor sensor(14);

    // DHT11を初期化する
    if (!sensor.initialize())
    {
        std::cerr
            << "DHT11 initialize failed."
            << std::endl;

        return 1;
    }

    std::cout
        << "DHT11 initialize success."
        << std::endl;

    // DHT11から温度・湿度を読み取る
    double temperature = 0.0;
    double humidity = 0.0;

    if (!sensor.read(
            temperature,
            humidity))
    {
        std::cerr
            << "DHT11 read failed."
            << std::endl;

        return 1;
    }

    std::cout
        << "Temperature : "
        << temperature
        << std::endl;

    std::cout
        << "Humidity    : "
        << humidity
        << std::endl;

    return 0;
}