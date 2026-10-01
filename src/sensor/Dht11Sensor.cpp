#include "sensor/Dht11Sensor.h"

Dht11Sensor::Dht11Sensor(unsigned int gpioPin)
    :m_gpioPin(gpioPin),
     m_gpioHandle(-1)

{

}

bool Dht11Sensor::initialize()
{
    return true;    
}

bool Dht11Sensor::read(double& temperature,double& humidity)
{
    (void)temperature;
    (void)humidity;

    return false;

}