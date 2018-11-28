#pragma once

#include <Arduino.h>

class Heartbeat {
public:
  Heartbeat(uint8_t reset_pin, uint16_t timeout_ms, int active = LOW, const char* heartbeat_string = "***HEARTBEAT***");

  void handle();
  bool is_heartbeat(char* string);

private:
  bool is_resettable();

  uint8_t _reset_pin;
  uint32_t _timeout_ms;
  int _active;
  const char* _heartbeat_string;

  uint32_t _last_beat_of_my_heart = 0;
  uint32_t _resets = 0;
  uint32_t _last_reset = 0;
};
