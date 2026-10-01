#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <cstdint>

struct SENSOR_DATA_H
{
    int data_id;
    double temperature;  //温度
    double humidity;        //湿度
    std::int64_t timestamp;        //time


}

#endif