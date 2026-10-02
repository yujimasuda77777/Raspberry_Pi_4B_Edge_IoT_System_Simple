#include <iostream>
#include "common/SensorData.h"
#include "sensor/Dht11Sensor.h"

int main()
{

    std::cout << "Sensor Process Start" << std::endl;

    Dht11Sensor sensor(14);

    if(!sensor.initialize())
    {
         std::cout << "Initialize failed" << std::endl;
         return 1;
    }

    std::cout << "DHT11 initialize success." << std::endl;

    double temperature = 0.0;
    double humidity = 0.0;

    if(!sensor.read(temperature,humidity))
    {
        std::cerr << "DHT11 read failed."
                  << std::endl;

        return 1;
    }


    std::cout << "Temperature : "
              << temperature
              << std::endl;

    std::cout << "Humidity    : "
              << humidity
              << std::endl;

    return 0;

    

}