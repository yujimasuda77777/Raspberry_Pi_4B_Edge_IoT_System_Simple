#include "sensor/Dht11Sensor.h"

#include <iostream>

#include <lgpio.h>

/**
 * @brief コンストラクタ
 *
 * @param gpioPin DHT11を接続するGPIO番号
 */
Dht11Sensor::Dht11Sensor(unsigned int gpioPin)
    : m_gpioPin(gpioPin),
      m_gpioHandle(-1),
      m_gpioClaimed(false)
{
}

/**
 * @brief デストラクタ
 */
Dht11Sensor::~Dht11Sensor()
{
    // GPIOを確保している場合は解放する
    if (m_gpioClaimed)
    {
        lgGpioFree(m_gpioHandle, m_gpioPin);
    }

    // GPIOチップのハンドルを閉じる
    if (m_gpioHandle >= 0)
    {
        lgGpiochipClose(m_gpioHandle);
    }
}

/**
 * @brief DHT11を初期化する
 *
 * GPIOチップを開き、指定されたGPIOを入力として確保する。
 *
 * @return true  初期化成功
 * @return false 初期化失敗
 */
bool Dht11Sensor::initialize()
{
    // GPIOチップ0を開く
    m_gpioHandle = lgGpiochipOpen(0);

    if (m_gpioHandle < 0)
    {
        std::cout << "GPIO chip open failed." << std::endl;
        return false;
    }

    // GPIOを入力として確保する
    int result = lgGpioClaimInput(
        m_gpioHandle,
        0,
        m_gpioPin
    );

    if (result < 0)
    {
        std::cout << "GPIO claim failed. GPIO="
                  << m_gpioPin
                  << std::endl;

        lgGpiochipClose(m_gpioHandle);
        m_gpioHandle = -1;

        return false;
    }

    m_gpioClaimed = true;

    std::cout << "DHT11 GPIO initialized. GPIO="
              << m_gpioPin
              << std::endl;

    return true;
}

/**
 * @brief DHT11から温度・湿度を読み取る
 *
 * 現段階ではDHT11の40bit通信はまだ実装しない。
 *
 * @param temperature 読み取った温度
 * @param humidity    読み取った湿度
 *
 * @return false 現段階では未実装
 */
bool Dht11Sensor::read(double& temperature, double& humidity)
{
    temperature = 0.0;
    humidity = 0.0;

    if(!lgGpioClaimed)
    {
        std::std << "GPio is not claimed." << std::endl;
        return false;
    }

    if (lGpioWrite(m_gpioHandle, m_gpioPin, 0) < 0)
    {
        std:cerr << "lgGpioWrite LOW failed." << std::endl;
        return false;

    }

      std::cout << "DHT11 start signal: LOW" << std::endl;

    return true;


}