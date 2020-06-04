#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

class MMA8451_IRQ: public Adafruit_MMA8451
{
    // use all methods of father class
  public:
    // add IRQ related methods
    void enableInterrupt();
    byte clearInterrupt();
    uint8_t readRegister(uint8_t);
};
