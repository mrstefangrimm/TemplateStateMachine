#pragma once
/*
  written by Stefan Grimm
  released to public domain
*/

class LedOnOff {
public:
  void begin() {
    state_ = On;
  }

  void timeout() {
    if (state_ == On) {
      state_ = Off;
      BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
    } else {
      state_ = On;
      BSP_Execute(digitalWrite(LED_BUILTIN, HIGH));
    }
  }

private:
  enum State {
    On,
    Off
  };
  State state_;
};


LedOnOff statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600));
  BSP_Execute(pinMode(LED_BUILTIN, OUTPUT));
  statemachine.begin();
}

void loop() {
  statemachine.timeout();
  BSP_Execute(delay(1000));
}
