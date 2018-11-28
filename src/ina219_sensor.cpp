#include "ina219_sensor.h"

void INA219_Sensor::begin() {
  _ina219.begin();

  Serial.println("INA219 OK");
}

void INA219_Sensor::handle() {
  _shunt_voltage = _ina219.getShuntVoltage_mV();
  _bus_voltage = _ina219.getBusVoltage_V();
  _current = _ina219.getCurrent_mA();
  _power = _ina219.getPower_mW();
  _load_voltage = _bus_voltage + (_shunt_voltage / 1000);
}
