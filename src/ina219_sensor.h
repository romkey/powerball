#pragma once

#include "sensor.h"

#include <Adafruit_INA219.h>

class INA219_Sensor : public Sensor {
 public:
  INA219_Sensor(uint16_t update_frequency, uint16_t accuracy, uint16_t precision, boolean calibrated) : Sensor(update_frequency, accuracy, precision, calibrated) {};

  void begin();
  void handle();

  float shunt_voltage() { _mark_read(); return _shunt_voltage; };
  float bus_voltage() { _mark_read(); return _bus_voltage; };
  float load_voltage() { _mark_read(); return _load_voltage; };
  float current() { _mark_read(); return _current; };
  float power() { _mark_read(); return _power; };

 private:
  Adafruit_INA219 _ina219;

  float _shunt_voltage;
  float _bus_voltage;
  float _load_voltage;
  float _current;
  float _power;
};
