#include "communication/CloudflareClient.h"

#include <curl/curl.h>

#include <cstdio>
#include <cstring>
#include <iostream>

/**
 * @brief コンストラクタ
 *
 * @param workerUrl Cloudflare WorkerのURL
 */
CloudflareClient::CloudflareClient(
    const char* workerUrl)
    : m_workerUrl(workerUrl)
{
    // JSONバッファを初期化する
    std::memset(
        m_jsonBuffer,
        0,
        sizeof(m_jsonBuffer)
    );

    // libcurlを初期化する
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

/**
 * @brief デストラクタ
 */
CloudflareClient::~CloudflareClient()
{
    // libcurlを終了する
    curl_global_cleanup();
}

/**
 * @brief SensorDataをJSON文字列に変換する
 *
 * @param data SensorData
 *
 * @return JSON文字列
 */
const char* CloudflareClient::createJson(
    const SensorData& data)
{
    /*
     * Cloudflare Workerへ送信するJSON。
     *
     * 例:
     *
     * {
     *   "data_id":1,
     *   "temperature":26.6,
     *   "humidity":61.9,
     *   "timestamp":1790415121
     * }
     */
    std::snprintf(
        m_jsonBuffer,
        sizeof(m_jsonBuffer),
        "{"
        "\"data_id\":%d,"
        "\"temperature\":%.1f,"
        "\"humidity\":%.1f,"
        "\"timestamp\":%lld"
        "}",
        data.data_id,
        data.temperature,
        data.humidity,
        static_cast<long long>(
            data.timestamp)
    );

    return m_jsonBuffer;
}

/**
 * @brief Cloudflare WorkerへSensorDataを送信する
 *
 * @param data 送信するSensorData
 *
 * @return true  送信成功
 * @return false 送信失敗
 */
bool CloudflareClient::post(
    const SensorData& data)
{
    if (m_workerUrl == nullptr)
    {
        std::cerr
            << "Worker URL is null."
            << std::endl;

        return false;
    }

    /*
     * JSONを作成する。
     */
    const char* json =
        createJson(data);

    std::cout
        << "POST JSON: "
        << json
        << std::endl;

    /*
     * libcurlのハンドルを作成する。
     */
    CURL* curl = curl_easy_init();

    if (curl == nullptr)
    {
        std::cerr
            << "curl_easy_init failed."
            << std::endl;

        return false;
    }

    /*
     * HTTPヘッダを作成する。
     */
    struct curl_slist* headers = nullptr;

    headers = curl_slist_append(
        headers,
        "Content-Type: application/json"
    );

    /*
     * HTTP POSTの設定。
     */
    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        m_workerUrl
    );

    curl_easy_setopt(
        curl,
        CURLOPT_HTTPHEADER,
        headers
    );

    curl_easy_setopt(
        curl,
        CURLOPT_POST,
        1L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_POSTFIELDS,
        json
    );

    /*
     * 通信タイムアウト。
     *
     * 今回は複雑なリトライ処理は行わない。
     */
    curl_easy_setopt(
        curl,
        CURLOPT_TIMEOUT,
        10L
    );

    /*
     * HTTP POSTを実行する。
     */
    const CURLcode result =
        curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cerr
            << "curl_easy_perform failed. "
            << curl_easy_strerror(result)
            << std::endl;

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return false;
    }

    /*
     * HTTPステータスコードを取得する。
     */
    long httpStatus = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &httpStatus
    );

    std::cout
        << "HTTP Status: "
        << httpStatus
        << std::endl;

    /*
     * 後片付け。
     */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    /*
     * 200番台なら成功とする。
     */
    if (httpStatus < 200 ||
        httpStatus >= 300)
    {
        std::cerr
            << "Cloudflare Worker returned error."
            << std::endl;

        return false;
    }

    return true;
}
