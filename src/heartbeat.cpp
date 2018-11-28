#include <Arduino.h>

#include "heartbeat.h"

// Heartbeat::Heartbeat(uint8_t reset_pin, uint16_t timeout_ms, int active = LOW, const char* heartbeat_string = "***HEARTBEAT***") : _reset_pin(reset_pin), _timeout_ms(timeout_ms), _active(active), _heartbeat_string(heartbeat_string) {
Heartbeat::Heartbeat(uint8_t reset_pin, uint16_t timeout_ms, int active, const char* heartbeat_string) : _reset_pin(reset_pin), _timeout_ms(timeout_ms), _active(active), _heartbeat_string(heartbeat_string) {
  if(is_resettable()) {
    pinMode(_reset_pin, OUTPUT);
    digitalWrite(_reset_pin, !active);
  }   
}

void Heartbeat::handle() {
  if(millis() - _last_beat_of_my_heart > _timeout_ms)
    digitalWrite(_reset_pin, _active);
}

bool Heartbeat::is_heartbeat(char *str) {
 if(strcmp(str, _heartbeat_string) == 0) {
   _last_beat_of_my_heart = millis();
   return true;
 } else
   return false;
}

bool Heartbeat::is_resettable() {
  return _reset_pin >= 0;
}
