#include "sensor/Dht11Sensor.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

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
        m_gpioClaimed = false;
    }

    // GPIOチップを閉じる
    if (m_gpioHandle >= 0)
    {
        lgGpiochipClose(m_gpioHandle);
        m_gpioHandle = -1;
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

    /*
     * GPIO14を出力として確保する。
     *
     * 初期値はHIGHにする。
     */
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
 * @brief GPIOエッジ通知コールバック
 *
 * DHT11からGPIOの状態変化が発生すると、
 * lgpioからこの関数が呼び出される。
 *
 * @param numAlerts 通知されたエッジ数
 * @param alerts    エッジ情報
 * @param userdata  Dht11Sensor自身へのポインタ
 */
void Dht11Sensor::alertCallback(
    int numAlerts,
    lgGpioAlert_p alerts,
    void* userdata)
{
    // userdataからDht11Sensorを取得する
    Dht11Sensor* sensor =
        static_cast<Dht11Sensor*>(userdata);

    if (sensor == nullptr)
    {
        return;
    }

    /*
     * 通知されたエッジを順番に処理する。
     */
    for (int i = 0; i < numAlerts; ++i)
    {
        const lgGpioAlert_t& alert = alerts[i];

        /*
         * 今回はGPIO14のエッジだけを表示する。
         */
        if (alert.report.gpio != sensor->m_gpioPin)
        {
            continue;
        }

        /*
         * timestampはエッジが発生した時刻。
         *
         * DHT11では、このtimestampの差を利用して
         * パルス幅を測定する。
         */
        std::cout
            << "DHT11 edge"
            << " GPIO=" << alert.report.gpio
            << " level=" << alert.report.level
            << " timestamp=" << alert.report.timestamp
            << std::endl;
    }
}

/**
 * @brief DHT11から温度・湿度を読み取る
 *
 * 今回はDHT11から返ってくるGPIOエッジを
 * lgpioで受信するところまで実装する。
 *
 * @param temperature 読み取った温度
 * @param humidity    読み取った湿度
 *
 * @return true  通信開始成功
 * @return false 通信開始失敗
 */
bool Dht11Sensor::read(
    double& temperature,
    double& humidity)
{
    // 今回はまだデータ解析をしない
    temperature = 0.0;
    humidity = 0.0;

    /*
     * GPIOが確保されているか確認する。
     */
    if (!m_gpioClaimed)
    {
        std::cerr << "GPIO is not claimed."
                  << std::endl;

        return false;
    }

    /*
     * ================================================
     * 1. DHT11への開始信号
     * ================================================
     */

    // GPIO14をLOWにする
    if (lgGpioWrite(
            m_gpioHandle,
            m_gpioPin,
            0) < 0)
    {
        std::cerr << "lgGpioWrite LOW failed."
                  << std::endl;

        return false;
    }

    std::cout
        << "DHT11 start signal: LOW"
        << std::endl;

    /*
     * DHT11への開始信号として
     * 約18ms LOWを維持する。
     */
    std::this_thread::sleep_for(
        std::chrono::milliseconds(18)
    );

    /*
     * ================================================
     * 2. GPIOをHIGHにする
     * ================================================
     */

    if (lgGpioWrite(
            m_gpioHandle,
            m_gpioPin,
            1) < 0)
    {
        std::cerr << "lgGpioWrite HIGH failed."
                  << std::endl;

        return false;
    }

    std::cout
        << "DHT11 start signal: HIGH"
        << std::endl;

    /*
     * ================================================
     * 3. GPIOを一度解放する
     * ================================================
     *
     * 今まではGPIOを「出力」として使用していた。
     *
     * ここからはDHT11がGPIOを操作するので、
     * Raspberry Pi側の出力設定を解除する。
     */

    if (lgGpioFree(
            m_gpioHandle,
            m_gpioPin) < 0)
    {
        std::cerr << "lgGpioFree failed."
                  << std::endl;

        m_gpioClaimed = false;

        return false;
    }

    m_gpioClaimed = false;

    /*
     * ================================================
     * 4. GPIOエッジ通知コールバックを登録
     * ================================================
     *
     * DHT11のGPIO14の状態変化を受け取る。
     */
    int result = lgGpioSetAlertsFunc(
        m_gpioHandle,
        m_gpioPin,
        Dht11Sensor::alertCallback,
        this
    );

    if (result < 0)
    {
        std::cerr << "lgGpioSetAlertsFunc failed."
                  << std::endl;

        return false;
    }

    /*
     * ================================================
     * 5. GPIOを両エッジ監視として確保
     * ================================================
     *
     * LG_BOTH_EDGES:
     *
     *   HIGH → LOW
     *   LOW  → HIGH
     *
     * の両方を監視する。
     */
    result = lgGpioClaimAlert(
        m_gpioHandle,
        0,
        LG_BOTH_EDGES,
        m_gpioPin,
        -1
    );

    if (result < 0)
    {
        std::cerr << "lgGpioClaimAlert failed."
                  << std::endl;

        lgGpioSetAlertsFunc(
            m_gpioHandle,
            m_gpioPin,
            nullptr,
            nullptr
        );

        return false;
    }

    m_gpioClaimed = true;

    std::cout
        << "DHT11 GPIO alert started."
        << std::endl;

    /*
     * ================================================
     * 6. DHT11の応答を待つ
     * ================================================
     *
     * DHT11はこの間に応答する。
     *
     * コールバック関数が呼び出され、
     * GPIOのエッジ情報が表示される。
     */
    std::this_thread::sleep_for(
        std::chrono::milliseconds(5)
    );

    /*
     * ================================================
     * 7. GPIO監視を終了
     * ================================================
     */

    lgGpioSetAlertsFunc(
        m_gpioHandle,
        m_gpioPin,
        nullptr,
        nullptr
    );

    std::cout
        << "DHT11 response monitoring finished."
        << std::endl;

    return true;
}