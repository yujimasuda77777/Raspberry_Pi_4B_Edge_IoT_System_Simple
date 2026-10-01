#include <iostream>
#include "common/SensorData.h"
#include "sensor/Dht11Sensor.h"

int main()
{

    std::cout << "Sensor Process Start" << std::endl;

    Dht11Sensor sensor(14);

    if(!sensor.initialize)
    {
         std::cout << "Initialize failed" << std::endl;
         return 1;
    }

    std::cout << "DHT11 initialize success." << std::endl;

    SensorData data;

    data.data_id = 1;
    data.temperature = 25.5;
    data.humidity = 10.4;
    data.timestamp = 1234567890;


    std::cout << "Data ID :" << data.data_id << std::endl;

    std::cout << "Data Temperature :" << data.temperature << std::endl;
    std::cout << "Data Humidity :" << data.humidity << std::endl;
    std::cout << "Data Timestamp :" << data.timestamp << std::endl;

    return 0;

    

}