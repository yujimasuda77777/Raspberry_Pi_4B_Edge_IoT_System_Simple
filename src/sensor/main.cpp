#include <iostream>
#include "common/SensorData.h"

int main()
{
    SensorData data;

    data.data_id = 1;
    data.temperature = 25.5;
    data.humidity = 10.4;
    data.timestamp = 1234567890;

    std::cout << "Sensor Process Start" << std::endl;
    std::cout << "Data ID :" << data.data_id << std::endl;

    std::cout << "Data Temperature :" << data.temperature << std::endl;
    std::cout << "Data Humidity :" << data.humidity << std::endl;
    std::cout << "Data Timestamp :" << data.timestamp << std::endl;

    return 0;

    

}