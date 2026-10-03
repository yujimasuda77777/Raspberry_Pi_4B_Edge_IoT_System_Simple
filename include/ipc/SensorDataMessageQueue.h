#ifndef SENSOR_DATA_MESSAGE_QUEUE_H
#define SENSOR_DATA_MESSAGE_QUEUE_H

#include "common/SensorData.h"

/**
 * @brief SensorDataをLinux Message Queueで送受信するクラス
 */
class SensorDataMessageQueue
{
public:

    /**
     * @brief コンストラクタ
     */
    SensorDataMessageQueue();

    /**
     * @brief デストラクタ
     */
    ~SensorDataMessageQueue();

    /**
     * @brief Message Queueを作成する
     *
     * @return true  作成成功
     * @return false 作成失敗
     */
    bool open();

    /**
     * @brief Message Queueを閉じる
     */
    void close();

    /**
     * @brief SensorDataをMessage Queueへ送信する
     *
     * @param data 送信するSensorData
     *
     * @return true  送信成功
     * @return false 送信失敗
     */
    bool send(const SensorData& data);

    /**
     * @brief Message QueueからSensorDataを受信する
     *
     * @param data 受信したSensorDataを格納する
     *
     * @return true 受信成功
     * @return false 受信失敗
     */
    bool receive(SensorData& data);

private:

    // Message Queueの名前
    const char* m_queueName;

    // Message Queueの識別子
    int m_queueId;
};

#endif // SENSOR_DATA_MESSAGE_QUEUE_H