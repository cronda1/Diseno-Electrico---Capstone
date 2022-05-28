#include <ezButton.h>

#define DEBOUNCE_TIME 50



struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

ezButton boton1(19);
ezButton boton2(18);
ezButton boton3(5);
ezButton boton4(23);

Button button1 = {19, 0, false};
Button button2 = {18, 0, false};
Button button3 = {5, 0, false};
Button button4 = {23, 0, false};

void IRAM_ATTR isr1() {
  button1.numberKeyPresses += 1;
  button1.pressed = true;
}
void IRAM_ATTR isr2() {
  button2.numberKeyPresses += 1;
  button2.pressed = true;
}
void IRAM_ATTR isr3() {
  button3.numberKeyPresses += 1;
  button3.pressed = true;
}
void IRAM_ATTR isr4() {
  button4.numberKeyPresses += 1;
  button4.pressed = true;
}



void setup() {
  Serial.begin(115200);
  
  pinMode(button1.PIN, INPUT_PULLUP);
  pinMode(button2.PIN, INPUT_PULLUP);
  pinMode(button3.PIN, INPUT_PULLUP);
  pinMode(button4.PIN, INPUT_PULLUP);
  
  attachInterrupt(button1.PIN, isr1, FALLING);
  attachInterrupt(button2.PIN, isr2, FALLING);
  attachInterrupt(button3.PIN, isr3, FALLING);
  attachInterrupt(button4.PIN, isr4, FALLING);

  boton1.setDebounceTime(DEBOUNCE_TIME);
  boton2.setDebounceTime(DEBOUNCE_TIME);
  boton3.setDebounceTime(DEBOUNCE_TIME);
  boton4.setDebounceTime(DEBOUNCE_TIME);
}

void loop() {
  boton1.loop();
  boton2.loop();
  boton3.loop();
  boton4.loop();
  if (button1.pressed) {
    if (boton1.isPressed()){
      Serial.println("Button 1 has been pressed");
    }
    if (boton1.isReleased()){
        button1.pressed = false;
    }      
  }

  if (button2.pressed) {
    if (boton2.isPressed()){
      Serial.println("Button 2 has been pressed");
    }
    if (boton2.isReleased()){
        button2.pressed = false;
    }      
  }

  if (button3.pressed) {
    if (boton3.isPressed()){
      Serial.println("Button 3 has been pressed");
    }
    if (boton3.isReleased()){
        button3.pressed = false;
    }      
  }

  if (button4.pressed) {
    if (boton4.isPressed()){
      Serial.println("Button 4 has been pressed");
    }
    if (boton4.isReleased()){
        button4.pressed = false;
    }      
  }

  }

 
