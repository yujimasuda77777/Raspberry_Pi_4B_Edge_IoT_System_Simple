cmake_minimum_required(VERSION 3.16)

project(Raspberry_Pi_4B_Edge_IoT_System)

# C++17を使用する
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Sensor Process
# ============================================================

add_executable(
    sensor-process

    # Sensor Processのmain
    src/sensor/main.cpp

    # DHT11センサ
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

# lgpioライブラリをリンクする
target_link_libraries(
    sensor-process
    PRIVATE
    lgpio
)

# Linux Message Queueで使用するライブラリ
target_link_libraries(
    sensor-process
    PRIVATE
    rt
)
