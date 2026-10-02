#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <lgpio.h>

/**
 * @brief DHT11温湿度センサを扱うクラス
 */
class Dht11Sensor
{
public:

    /**
     * @brief コンストラクタ
     *
     * @param gpioPin DHT11を接続するGPIO番号
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
     * @brief GPIOのエッジ通知を受け取るコールバック
     *
     * @param numAlerts 通知されたエッジ数
     * @param alerts    エッジ情報
     * @param userdata  Dht11Sensor自身へのポインタ
     */
    static void alertCallback(
        int numAlerts,
        lgGpioAlert_p alerts,
        void* userdata
    );

    // DHT11を接続するGPIO番号
    unsigned int m_gpioPin;

    // GPIOチップのハンドル
    int m_gpioHandle;

    // GPIOを確保しているか
    bool m_gpioClaimed;
};

#endif // DHT11_SENSOR_H