#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <cstdint>

/**
 * @brief DHT11温湿度センサを扱うクラス
 */
class Dht11Sensor
{
public:

    /**
     * @brief コンストラクタ
     *
     * @param gpioPin DHT11を接続するBCM GPIO番号
     */
    explicit Dht11Sensor(unsigned int gpioPin);

    /**
     * @brief デストラクタ
     */
    ~Dht11Sensor();

    /**
     * @brief DHT11を初期化する
     *
     * @return true  初期化成功
     * @return false 初期化失敗
     */
    bool initialize();

    /**
     * @brief DHT11から温度・湿度を読み取る
     *
     * @param temperature 読み取った温度
     * @param humidity    読み取った湿度
     *
     * @return true  読み取り成功
     * @return false 読み取り失敗
     */
    bool read(double& temperature, double& humidity);

private:

    /**
     * @brief GPIOを出力として確保する
     *
     * @param initialValue 初期出力値
     *
     * @return true  成功
     * @return false 失敗
     */
    bool configureOutput(int initialValue);

    /**
     * @brief GPIOを入力として確保する
     *
     * @return true  成功
     * @return false 失敗
     */
    bool configureInput();

    /**
     * @brief GPIOを解放する
     */
    void releaseGpio();

    /**
     * @brief DHT11から40bitの生データを取得する
     *
     * @param data 取得した5バイトのデータ
     *
     * @return true  成功
     * @return false 失敗
     */
    bool readRawData(std::uint8_t data[5]);

    /**
     * @brief DHT11のチェックサムを確認する
     *
     * @param data DHT11から取得した5バイト
     *
     * @return true  正常
     * @return false 異常
     */
    bool checkChecksum(const std::uint8_t data[5]) const;

private:

    // DHT11を接続するBCM GPIO番号
    unsigned int m_gpioPin;

    // lgpio GPIOチップハンドル
    int m_gpioHandle;

    // GPIOを現在確保しているか
    bool m_gpioClaimed;
};

#endif // DHT11_SENSOR_H