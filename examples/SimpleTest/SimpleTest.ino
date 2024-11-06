#include <RGBLED.h>

// ####################################################################

// LED RGB:
#define LED_RGB_R_PIN                 A8
#define LED_RGB_G_PIN                 A9
#define LED_RGB_B_PIN                 A10

// #####################################################################

RGBLED led;

// #####################################################################

void setup() {

  Serial.begin(115200);

  led.parameters.RED_PIN = LED_RGB_R_PIN;
  led.parameters.GREEN_PIN = LED_RGB_G_PIN; 
  led.parameters.BLUE_PIN = LED_RGB_B_PIN;
  led.parameters.COMMON_STATE = RGBLED_COMMON_CATHODE;

  if(led.init() == false)
  {
    Serial.println(led.errorMessage);
    while(1);
  }

}

// #####################################################################

void loop() {
  // put your main code here, to run repeatedly:

led.red();   // red
delay(1000);
led.green();   // green
delay(1000);
led.blue();   // blue
delay(1000);
led.yellow();   // red + green = yellow
delay(1000);
led.purple();   // red + blue = purple
delay(1000);
led.cyan();   // green + blue = cyan
delay(1000);
led.white();   // red + green + blue = white;
delay(1000);
}
