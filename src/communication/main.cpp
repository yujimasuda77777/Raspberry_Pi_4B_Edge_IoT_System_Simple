#include <cstdlib>
#include <iostream>

#include "common/SensorData.h"
#include "communication/CloudflareClient.h"
#include "ipc/SensorDataMessageQueue.h"

/**
 * @brief Communication Process
 */
int main()
{
    std::cout
        << "Communication Process Start"
        << std::endl;

    // ============================================================
    // Cloudflare WorkerのURLを環境変数から取得する
    // ============================================================

    const char* workerUrl =
        std::getenv(
            "CLOUDFLARE_WORKER_URL"
        );

    if (workerUrl == nullptr)
    {
        std::cerr
            << "CLOUDFLARE_WORKER_URL is not set."
            << std::endl;

        return 1;
    }

    std::cout
        << "Cloudflare Worker URL is set."
        << std::endl;

    // ============================================================
    // Message Queueを開く
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
    // Cloudflare Clientを作成する
    // ============================================================

    CloudflareClient cloudflareClient(
        workerUrl
    );

    // ============================================================
    // Message QueueからSensorDataを受信する
    // ============================================================

    SensorData data{};

    std::cout
        << "Waiting for SensorData..."
        << std::endl;

    if (!messageQueue.receive(data))
    {
        std::cerr
            << "SensorData receive failed."
            << std::endl;

        return 1;
    }

    std::cout
        << "SensorData received."
        << std::endl;

    // ============================================================
    // 受信したSensorDataを表示する
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
    // Cloudflare Workerへ送信する
    // ============================================================

    if (!cloudflareClient.post(data))
    {
        std::cerr
            << "Cloudflare POST failed."
            << std::endl;

        return 1;
    }

    std::cout
        << "Cloudflare POST success."
        << std::endl;

    return 0;
}
