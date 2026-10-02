#include "sensor/Dht11Sensor.h"

#include <lgpio.h>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>

/**
 * @brief GPIO状態変化情報
 */
struct GpioTransition
{
    // GPIOレベル
    // 0 = LOW
    // 1 = HIGH
    int level;

    // 状態変化が発生した時刻[us]
    std::uint64_t timestamp;
};

/**
 * @brief 現在時刻をマイクロ秒で取得する
 *
 * DHT11のパルス幅測定に使用する。
 *
 * @return 現在時刻[us]
 */
static std::uint64_t getMonotonicTimeUs()
{
    struct timespec timeSpec{};

    clock_gettime(
        CLOCK_MONOTONIC,
        &timeSpec
    );

    return
        static_cast<std::uint64_t>(
            timeSpec.tv_sec
        ) * 1000000ULL
        +
        static_cast<std::uint64_t>(
            timeSpec.tv_nsec
        ) / 1000ULL;
}

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
    // GPIOを解放する
    releaseGpio();

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
    /*
     * gpiochip0を開く。
     *
     * Raspberry Pi 4BのBCM GPIO14は
     * gpiochip0に存在する。
     */
    m_gpioHandle = lgGpiochipOpen(0);

    if (m_gpioHandle < 0)
    {
        std::cerr
            << "lgGpiochipOpen failed. "
            << "result="
            << m_gpioHandle
            << std::endl;

        return false;
    }

    /*
     * DHT11の信号線は通常HIGHで待機する。
     */
    if (!configureOutput(1))
    {
        return false;
    }

    /*
     * センサが安定するまで少し待つ。
     */
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100)
    );

    /*
     * GPIOを一度解放する。
     *
     * 実際の読み取りはreadRawData()で
     * 改めてGPIOを確保する。
     */
    releaseGpio();

    std::cout
        << "DHT11 initialized. GPIO="
        << m_gpioPin
        << std::endl;

    return true;
}

/**
 * @brief GPIOを出力として確保する
 *
 * @param initialValue 初期出力値
 *
 * @return true  成功
 * @return false 失敗
 */
bool Dht11Sensor::configureOutput(int initialValue)
{
    if (m_gpioHandle < 0)
    {
        return false;
    }

    /*
     * すでにGPIOを確保している場合は、
     * 一度解放する。
     */
    releaseGpio();

    const int result = lgGpioClaimOutput(
        m_gpioHandle,
        0,
        m_gpioPin,
        initialValue
    );

    if (result < 0)
    {
        std::cerr
            << "lgGpioClaimOutput failed. "
            << "GPIO="
            << m_gpioPin
            << " result="
            << result
            << std::endl;

        return false;
    }

    m_gpioClaimed = true;

    return true;
}

/**
 * @brief GPIOを入力として確保する
 *
 * @return true  成功
 * @return false 失敗
 */
bool Dht11Sensor::configureInput()
{
    if (m_gpioHandle < 0)
    {
        return false;
    }

    /*
     * 出力として確保されていた場合は
     * 一度解放する。
     */
    releaseGpio();

    const int result = lgGpioClaimInput(
        m_gpioHandle,
        0,
        m_gpioPin
    );

    if (result < 0)
    {
        std::cerr
            << "lgGpioClaimInput failed. "
            << "GPIO="
            << m_gpioPin
            << " result="
            << result
            << std::endl;

        return false;
    }

    m_gpioClaimed = true;

    return true;
}

/**
 * @brief GPIOを解放する
 */
void Dht11Sensor::releaseGpio()
{
    if (m_gpioHandle < 0)
    {
        return;
    }

    if (m_gpioClaimed)
    {
        lgGpioFree(
            m_gpioHandle,
            m_gpioPin
        );

        m_gpioClaimed = false;
    }
}

/**
 * @brief DHT11から40bitの生データを取得する
 *
 * @param data 取得した5バイトのデータ
 *
 * @return true  成功
 * @return false 失敗
 */
bool Dht11Sensor::readRawData(
    std::uint8_t data[5])
{
    if (data == nullptr)
    {
        return false;
    }

    /*
     * 5バイトを初期化する。
     */
    for (int i = 0; i < 5; ++i)
    {
        data[i] = 0;
    }

    /*
     * ================================================
     * 1. GPIOを出力LOWにする
     * ================================================
     */

    if (!configureOutput(0))
    {
        return false;
    }

    /*
     * ================================================
     * 2. LOWを18ms保持する
     * ================================================
     *
     * これがDHT11への開始信号。
     */
    std::this_thread::sleep_for(
        std::chrono::milliseconds(18)
    );

    /*
     * ================================================
     * 3. GPIOを入力に切り替える
     * ================================================
     *
     * まず出力GPIOを解放する。
     */
    releaseGpio();

    /*
     * GPIOを入力として再確保する。
     */
    if (!configureInput())
    {
        return false;
    }

    /*
     * ================================================
     * 4. GPIO状態を高速ポーリングする
     * ================================================
     *
     * DHT11から返ってくる信号を
     * lgGpioRead()で連続的に確認する。
     */

    std::vector<GpioTransition> transitions;

    transitions.reserve(100);

    /*
     * 読み取り開始時刻。
     */
    const std::uint64_t startTime =
        getMonotonicTimeUs();

    /*
     * 最大250ms監視する。
     */
    constexpr std::uint64_t TIMEOUT_US =
        250000ULL;

    /*
     * DHT11は待機時HIGHなので、
     * 最初はHIGHとする。
     */
    int previousLevel = 1;

    while (true)
    {
        const std::uint64_t currentTime =
            getMonotonicTimeUs();

        /*
         * タイムアウト確認。
         */
        if ((currentTime - startTime)
            >= TIMEOUT_US)
        {
            break;
        }

        /*
         * GPIOの現在状態を取得する。
         */
        const int level =
            lgGpioRead(
                m_gpioHandle,
                m_gpioPin
            );

        if (level < 0)
        {
            std::cerr
                << "lgGpioRead failed. "
                << "result="
                << level
                << std::endl;

            releaseGpio();

            return false;
        }

        /*
         * GPIO状態が変化した場合だけ
         * その時刻を記録する。
         */
        if (level != previousLevel)
        {
            GpioTransition transition{};

            transition.level = level;
            transition.timestamp = currentTime;

            transitions.push_back(
                transition
            );

            previousLevel = level;

            /*
             * DHT11では約80個程度のエッジが
             * 発生する。
             *
             * 100個取得したら終了する。
             */
            if (transitions.size() >= 100)
            {
                break;
            }
        }
    }

    /*
     * GPIOを解放する。
     */
    releaseGpio();

    /*
     * エッジが少なすぎる場合は失敗。
     */
    if (transitions.size() < 2)
    {
        std::cerr
            << "DHT11 timing timeout."
            << std::endl;

        return false;
    }

    /*
     * ================================================
     * 5. HIGHパルス幅を取得する
     * ================================================
     *
     * DHT11のデータは、
     *
     * LOW  約50us
     *
     * HIGH 約26～28us → 0
     * HIGH 約70us     → 1
     *
     * という形式。
     */
    std::vector<std::uint64_t> highPulses;

    highPulses.reserve(50);

    bool highActive = false;

    std::uint64_t highStart = 0;

    for (const auto& transition : transitions)
    {
        /*
         * LOW → HIGH
         */
        if (transition.level == 1)
        {
            highActive = true;

            highStart =
                transition.timestamp;
        }
        /*
         * HIGH → LOW
         */
        else if (transition.level == 0)
        {
            if (highActive)
            {
                const std::uint64_t pulseWidth =
                    transition.timestamp
                    - highStart;

                highPulses.push_back(
                    pulseWidth
                );

                highActive = false;
            }
        }
    }

    /*
     * 40bit分のHIGHパルスが必要。
     */
    if (highPulses.size() < 40)
    {
        std::cerr
            << "DHT11 timing timeout. "
            << "HIGH pulses="
            << highPulses.size()
            << std::endl;

        return false;
    }

    /*
     * ================================================
     * 6. 最後の40個を使用する
     * ================================================
     */
    const std::size_t startIndex =
        highPulses.size() - 40;

    /*
     * ================================================
     * 7. 40bitを5バイトへ変換する
     * ================================================
     */
    for (int bitIndex = 0;
         bitIndex < 40;
         ++bitIndex)
    {
        const std::uint64_t pulseWidth =
            highPulses[
                startIndex
                + static_cast<std::size_t>(
                    bitIndex)
            ];

        /*
         * 40usを境界値として、
         *
         * 26～28us → 0
         * 約70us    → 1
         *
         * と判定する。
         */
        const bool bitValue =
            pulseWidth > 40;

        const int byteIndex =
            bitIndex / 8;

        const int bitPosition =
            7 - (bitIndex % 8);

        if (bitValue)
        {
            data[byteIndex] |=
                static_cast<std::uint8_t>(
                    1U << bitPosition
                );
        }
    }

    return true;
}

/**
 * @brief DHT11のチェックサムを確認する
 *
 * @param data DHT11から取得した5バイト
 *
 * @return true  正常
 * @return false 異常
 */
bool Dht11Sensor::checkChecksum(
    const std::uint8_t data[5]) const
{
    const std::uint8_t checksum =
        static_cast<std::uint8_t>(
            data[0]
            + data[1]
            + data[2]
            + data[3]
        );

    return checksum == data[4];
}

/**
 * @brief DHT11から温度・湿度を取得する
 *
 * @param temperature 温度
 * @param humidity 湿度
 *
 * @return true  読み取り成功
 * @return false 読み取り失敗
 */
bool Dht11Sensor::read(
    double& temperature,
    double& humidity)
{
    /*
     * DHT11から受信する5バイト。
     *
     * data[0] : 湿度整数部
     * data[1] : 湿度小数部
     * data[2] : 温度整数部
     * data[3] : 温度小数部
     * data[4] : checksum
     */
    std::uint8_t data[5] = {};

    /*
     * 40bitを取得する。
     */
    if (!readRawData(data))
    {
        return false;
    }

    /*
     * チェックサムを確認する。
     */
    if (!checkChecksum(data))
    {
        std::cerr
            << "DHT11 checksum error."
            << std::endl;

        return false;
    }

    /*
     * 湿度を計算する。
     */
    humidity =
        static_cast<double>(data[0])
        +
        static_cast<double>(data[1])
        / 10.0;

    /*
     * 温度を計算する。
     */
    temperature =
        static_cast<double>(data[2])
        +
        static_cast<double>(data[3])
        / 10.0;

    /*
     * 温度の範囲を確認する。
     */
    if (temperature < -40.0 ||
        temperature > 80.0)
    {
        std::cerr
            << "DHT11 temperature range error."
            << std::endl;

        return false;
    }

    /*
     * 湿度の範囲を確認する。
     */
    if (humidity < 0.0 ||
        humidity > 100.0)
    {
        std::cerr
            << "DHT11 humidity range error."
            << std::endl;

        return false;
    }

    return true;
}