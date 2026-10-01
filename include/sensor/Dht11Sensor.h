#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

class Dht11Sensor
{
public:

    explicit Dht11Sensor(unsigned int gpioPin);

    bool initialize();

    bool read(double& temperature,double& humidity);

private:

    unsigned int m_gpioPin;

    int m_gpioHandle;



};

#endif