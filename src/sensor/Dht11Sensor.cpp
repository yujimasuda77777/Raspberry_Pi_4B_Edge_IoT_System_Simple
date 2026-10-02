#include "sensor/Dht11Sensor.h"

#include <chrono>
#include <iostream>
#include <thread>

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

    // GPIOチップを閉じる
    if (m_gpioHandle >= 0)
    {
        lgGpiochipClose(m_gpioHandle);
    }
}

/**
 * @brief DHT11を初期化する
 *
 * @return true  初期化成功
 * @return false 初期化失敗
 */
bool Dht11Sensor::initialize()
{
    // gpiochip0を開く
    m_gpioHandle = lgGpiochipOpen(0);

    if (m_gpioHandle < 0)
    {
        std::cerr << "lgGpiochipOpen failed."
                  << std::endl;

        return false;
    }

    // GPIO14を出力として確保する
    int result = lgGpioClaimOutput(
        m_gpioHandle,
        0,
        m_gpioPin,
        1
    );

    if (result < 0)
    {
        std::cerr << "lgGpioClaimOutput failed."
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
 * 今回はDHT11の応答パルスを確認する。
 *
 * @param temperature 読み取った温度
 * @param humidity    読み取った湿度
 *
 * @return true  通信開始成功
 * @return false 通信開始失敗
 */
bool Dht11Sensor::read(double& temperature, double& humidity)
{
    // 今回はまだ温度・湿度を解析しない
    temperature = 0.0;
    humidity = 0.0;

    // GPIOが確保されているか確認する
    if (!m_gpioClaimed)
    {
        std::cerr << "GPIO is not claimed."
                  << std::endl;

        return false;
    }

    /*
     * --------------------------------------------------
     * 1. DHT11に通信開始を通知する
     * --------------------------------------------------
     */

    // GPIOをLOWにする
    if (lgGpioWrite(m_gpioHandle, m_gpioPin, 0) < 0)
    {
        std::cerr << "lgGpioWrite LOW failed."
                  << std::endl;

        return false;
    }

    std::cout << "DHT11 start signal: LOW"
              << std::endl;

    /*
     * DHT11では、ホスト側がLOWを一定時間維持する。
     *
     * 今回は約18ms待つ。
     */
    std::this_thread::sleep_for(
        std::chrono::milliseconds(18)
    );

    /*
     * --------------------------------------------------
     * 2. GPIOをHIGHにする
     * --------------------------------------------------
     */

    if (lgGpioWrite(m_gpioHandle, m_gpioPin, 1) < 0)
    {
        std::cerr << "lgGpioWrite HIGH failed."
                  << std::endl;

        return false;
    }

    std::cout << "DHT11 start signal: HIGH"
              << std::endl;

    /*
     * --------------------------------------------------
     * 3. GPIOを入力監視に切り替える
     * --------------------------------------------------
     *
     * DHT11はここからGPIOを使って応答してくる。
     *
     * GPIOを出力として確保したままでは、
     * DHT11の信号を受信できない。
     *
     * そのため、一度GPIOを解放する。
     */

    if (lgGpioFree(m_gpioHandle, m_gpioPin) < 0)
    {
        std::cerr << "lgGpioFree failed."
                  << std::endl;

        m_gpioClaimed = false;

        return false;
    }

    m_gpioClaimed = false;

    /*
     * --------------------------------------------------
     * 4. DHT11の応答を監視する
     * --------------------------------------------------
     */

    int result = lgGpioClaimAlert(
        m_gpioHandle,
        0,
        m_gpioPin,
        LG_BOTH_EDGES
    );

    if (result < 0)
    {
        std::cerr << "lgGpioClaimAlert failed."
                  << std::endl;

        return false;
    }

    m_gpioClaimed = true;

    std::cout << "DHT11 GPIO alert started."
              << std::endl;

    /*
     * --------------------------------------------------
     * 5. 少し待ってDHT11の応答を受信する
     * --------------------------------------------------
     *
     * 今回はまだ40bitの解析をしない。
     *
     * 次の段階で、この部分に
     * エッジのタイムスタンプ取得処理を追加する。
     */

    std::this_thread::sleep_for(
        std::chrono::milliseconds(5)
    );

    std::cout << "DHT11 response monitoring finished."
              << std::endl;

    return true;
}