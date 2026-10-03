```cpp
#include "ipc/SensorDataMessageQueue.h"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

/**
 * @brief Message Queueの最大メッセージ数
 */
constexpr long MESSAGE_QUEUE_MAX_MESSAGES = 10;

/**
 * @brief コンストラクタ
 */
SensorDataMessageQueue::SensorDataMessageQueue()
    : m_queueName("/raspberry_pi_edge_iot_sensor_data"),
      m_queueId(-1)
{
}

/**
 * @brief デストラクタ
 */
SensorDataMessageQueue::~SensorDataMessageQueue()
{
    close();
}

/**
 * @brief Message Queueを作成する
 *
 * @return true  作成成功
 * @return false 作成失敗
 */
bool SensorDataMessageQueue::open()
{
    /*
     * Message Queueの属性を設定する。
     */
    struct mq_attr attributes{};

    // 最大メッセージ数
    attributes.mq_maxmsg =
        MESSAGE_QUEUE_MAX_MESSAGES;

    // 1メッセージの最大サイズ
    attributes.mq_msgsize =
        sizeof(SensorData);

    /*
     * Message Queueを作成して開く。
     *
     * O_CREAT:
     *   存在しなければ作成する
     *
     * O_RDWR:
     *   読み書き両方可能にする
     */
    m_queueId = mq_open(
        m_queueName,
        O_CREAT | O_RDWR,
        0666,
        &attributes
    );

    if (m_queueId == -1)
    {
        std::cerr
            << "mq_open failed. "
            << std::strerror(errno)
            << std::endl;

        return false;
    }

    std::cout
        << "Message Queue opened."
        << std::endl;

    return true;
}

/**
 * @brief Message Queueを閉じる
 */
void SensorDataMessageQueue::close()
{
    /*
     * Message Queueが開かれていない場合は
     * 何もしない。
     */
    if (m_queueId == -1)
    {
        return;
    }

    mq_close(m_queueId);

    m_queueId = -1;

    std::cout
        << "Message Queue closed."
        << std::endl;
}

/**
 * @brief SensorDataをMessage Queueへ送信する
 *
 * @param data 送信するSensorData
 *
 * @return true  送信成功
 * @return false 送信失敗
 */
bool SensorDataMessageQueue::send(
    const SensorData& data)
{
    if (m_queueId == -1)
    {
        std::cerr
            << "Message Queue is not open."
            << std::endl;

        return false;
    }

    /*
     * SensorDataをMessage Queueへ送信する。
     */
    const int result = mq_send(
        m_queueId,
        reinterpret_cast<const char*>(&data),
        sizeof(SensorData),
        0
    );

    if (result == -1)
    {
        std::cerr
            << "mq_send failed. "
            << std::strerror(errno)
            << std::endl;

        return false;
    }

    return true;
}

/**
 * @brief Message QueueからSensorDataを受信する
 *
 * @param data 受信したSensorDataを格納する
 *
 * @return true 受信成功
 * @return false 受信失敗
 */
bool SensorDataMessageQueue::receive(
    SensorData& data)
{
    if (m_queueId == -1)
    {
        std::cerr
            << "Message Queue is not open."
            << std::endl;

        return false;
    }

    /*
     * Message QueueからSensorDataを受信する。
     */
    const ssize_t result = mq_receive(
        m_queueId,
        reinterpret_cast<char*>(&data),
        sizeof(SensorData),
        nullptr
    );

    if (result == -1)
    {
        std::cerr
            << "mq_receive failed. "
            << std::strerror(errno)
            << std::endl;

        return false;
    }

    /*
     * 受信サイズがSensorDataと一致するか確認する。
     */
    if (result != sizeof(SensorData))
    {
        std::cerr
            << "Invalid message size."
            << std::endl;

        return false;
    }

    return true;
}
```
