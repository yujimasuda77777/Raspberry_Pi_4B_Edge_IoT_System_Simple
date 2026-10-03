#include <chrono>
#include <iostream>

#include "common/SensorData.h"
#include "ipc/SensorDataMessageQueue.h"
#include "sensor/Dht11Sensor.h"

int main()
{
    std::cout
        << "Sensor Process Start"
        << std::endl;

    // ============================================================
    // DHT11センサを作成する
    // ============================================================

    // DHT11はBCM GPIO14に接続されている
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

    // ============================================================
    // Message Queueを作成する
    // ============================================================

    SensorDataMessageQueue messageQueue;

    if (!messageQueue.open())
    {
        std::cerr
            << "Message Queue open failed."
            << std::endl;

        return 1;
    }

    std::cout
        << "Message Queue open success."
        << std::endl;

    // ============================================================
    // DHT11から温度・湿度を読み取る
    // ============================================================

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

    // ============================================================
    // SensorDataを作成する
    // ============================================================

    SensorData data{};

    // データ識別番号
    data.data_id = 1;

    // DHT11から取得した温度
    data.temperature = temperature;

    // DHT11から取得した湿度
    data.humidity = humidity;

    // 現在時刻をUnix時刻で取得する
    data.timestamp =
        std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

    // ============================================================
    // SensorDataの内容を表示する
    // ============================================================

    std::cout
        << "SensorData"
        << std::endl;

    std::cout
        << "  data_id    : "
        << data.data_id
        << std::endl;

    std::cout
        << "  temperature: "
        << data.temperature
        << std::endl;

    std::cout
        << "  humidity   : "
        << data.humidity
        << std::endl;

    std::cout
        << "  timestamp  : "
        << data.timestamp
        << std::endl;

    // ============================================================
    // SensorDataをMessage Queueへ送信する
    // ============================================================

    if (!messageQueue.send(data))
    {
        std::cerr
            << "SensorData send failed."
            << std::endl;

        return 1;
    }

    std::cout
        << "SensorData send success."
        << std::endl;

    return 0;
}
