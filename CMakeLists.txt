```cmake
cmake_minimum_required(VERSION 3.16)

project(Raspberry_Pi_4B_Edge_IoT_System)

# ============================================================
# C++設定
# ============================================================

# C++17を使用する
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Sensor Process
# ============================================================

add_executable(
    sensor-process

    # Sensor Process
    src/sensor/main.cpp

    # DHT11
    src/sensor/Dht11Sensor.cpp

    # Message Queue
    src/ipc/SensorDataMessageQueue.cpp
)

# includeディレクトリをヘッダファイル検索対象にする
target_include_directories(
    sensor-process
    PRIVATE
    include
)

# Sensor Processで使用するライブラリ
target_link_libraries(
    sensor-process
    PRIVATE

    # GPIO
    lgpio

    # Linux Message Queue
    rt
)

# ============================================================
# Communication Process
# ============================================================

add_executable(
    communication-process

    # Communication Process
    src/communication/main.cpp

    # Message Queue
    src/ipc/SensorDataMessageQueue.cpp

    # Cloudflare
    src/communication/CloudflareClient.cpp
)

# includeディレクトリをヘッダファイル検索対象にする
target_include_directories(
    communication-process
    PRIVATE
    include
)

# Communication Processで使用するライブラリ
target_link_libraries(
    communication-process
    PRIVATE

    # Linux Message Queue
    rt

    # HTTP通信
    curl
)
```
