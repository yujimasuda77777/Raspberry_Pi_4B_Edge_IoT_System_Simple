#ifndef CLOUDFLARE_CLIENT_H
#define CLOUDFLARE_CLIENT_H

#include "common/SensorData.h"

/**
 * @brief Cloudflare WorkerへSensorDataを送信するクラス
 */
class CloudflareClient
{
public:

    /**
     * @brief コンストラクタ
     *
     * @param workerUrl Cloudflare WorkerのURL
     */
    explicit CloudflareClient(const char* workerUrl);

    /**
     * @brief デストラクタ
     */
    ~CloudflareClient();

    /**
     * @brief Cloudflare WorkerへSensorDataを送信する
     *
     * @param data 送信するSensorData
     *
     * @return true  送信成功
     * @return false 送信失敗
     */
    bool post(const SensorData& data);

private:

    /**
     * @brief SensorDataをJSON文字列に変換する
     *
     * @param data SensorData
     *
     * @return JSON文字列
     */
    const char* createJson(
        const SensorData& data);

private:

    // Cloudflare WorkerのURL
    const char* m_workerUrl;

    // JSON文字列を保持するバッファ
    char m_jsonBuffer[512];
};

#endif // CLOUDFLARE_CLIENT_H
