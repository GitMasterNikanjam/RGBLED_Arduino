// LED RGB:
#define LED_RGB_R_PIN                 A8
#define LED_RGB_G_PIN                 A9
#define LED_RGB_B_PIN                 A10

void setup() {
  // put your setup code here, to run once:
pinMode(LED_RGB_R_PIN, OUTPUT);
pinMode(LED_RGB_G_PIN, OUTPUT);
pinMode(LED_RGB_B_PIN, OUTPUT);

digitalWrite(LED_RGB_R_PIN, 1);
digitalWrite(LED_RGB_G_PIN, 1);
digitalWrite(LED_RGB_B_PIN, 1);

}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(LED_RGB_R_PIN, 0);
  digitalWrite(LED_RGB_G_PIN, 1);
  digitalWrite(LED_RGB_B_PIN, 1);

  delay(1000);

  digitalWrite(LED_RGB_R_PIN, 1);
  digitalWrite(LED_RGB_G_PIN, 0);
  digitalWrite(LED_RGB_B_PIN, 1);

  delay(1000);

  digitalWrite(LED_RGB_R_PIN, 1);
  digitalWrite(LED_RGB_G_PIN, 1);
  digitalWrite(LED_RGB_B_PIN, 0);

  delay(1000);

  digitalWrite(LED_RGB_R_PIN, 0);
  digitalWrite(LED_RGB_G_PIN, 0);
  digitalWrite(LED_RGB_B_PIN, 1);

  delay(1000);

  digitalWrite(LED_RGB_R_PIN, 0);
  digitalWrite(LED_RGB_G_PIN, 1);
  digitalWrite(LED_RGB_B_PIN, 0);

  delay(1000);

  digitalWrite(LED_RGB_R_PIN, 1);
  digitalWrite(LED_RGB_G_PIN, 0);
  digitalWrite(LED_RGB_B_PIN, 0);

  delay(1000);

  digitalWrite(LED_RGB_R_PIN, 1);
  digitalWrite(LED_RGB_G_PIN, 1);
  digitalWrite(LED_RGB_B_PIN, 1);

  delay(1000);

}
