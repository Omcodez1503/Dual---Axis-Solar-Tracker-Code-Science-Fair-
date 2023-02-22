#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define dc_motor_input1 16
#define dc_motor_input2 17
#define dc_motor_enable 32

#define Bottomservo_pin 18
#define Topservo_pin 19

#define tlLDR_pin 33
#define trLDR_pin 25
#define brLDR_pin 26
#define blLDR_pin 27
#define middleLDR_pin 14

#define panel_voltage_read_pin 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
 
bool dc_motor_open = false;

Servo Bottomservo;
int servob_angle_home = 90;
int servob_angle = 90;
int servob_max = 160;
int servob_min = 20;

Servo Topservo;
int servot_angle_home = 50;
int servot_angle = 50;
int servot_max = 70;
int servot_min = 15;

int init_middleLDR_val = 0;
int middleLDR_val;
int tlLDR_val;
int brLDR_val;
int blLDR_val;
int trLDR_val;
// char buffer[100];

int panel_voltage;

int cur_angle;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  pinMode(dc_motor_input1, OUTPUT);
  pinMode(dc_motor_input2, OUTPUT);
  pinMode(dc_motor_enable, OUTPUT);

  Bottomservo.attach(Bottomservo_pin);
  Topservo.attach(Topservo_pin);
  Topservo.write(servot_angle_home);
  Bottomservo.write(servob_angle_home);

  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  int i = 0;
  while (i < 2000){
    init_middleLDR_val = init_middleLDR_val + analogRead(middleLDR_pin);
    i = i + 1;
  }
  init_middleLDR_val = (init_middleLDR_val/2000) + 600;
}

void loop() {

  //Read LDR Values
  middleLDR_val = analogRead(middleLDR_pin);
  tlLDR_val = analogRead(tlLDR_pin);
  trLDR_val = analogRead(trLDR_pin);
  brLDR_val = analogRead(brLDR_pin);
  blLDR_val = analogRead(blLDR_pin);

  //Read panel voltage
  panel_voltage = analogRead(panel_voltage_read_pin);
  // Serial.println(((panel_voltage / 4095.0) * 3.3) * 4);


  // Check LDR Value And If There Is Light, open panels and start tracking
  if (middleLDR_val > init_middleLDR_val) {

    //update oled screen with ldr voltage
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(27, 0);
    display.println("Voltage");
    display.setTextSize(1);
    display.setCursor(50, 16);
    display.print("M");
    display.print(middleLDR_val);
    display.setTextSize(3);
    display.setCursor(22, 40);
    display.print(((panel_voltage / 4095.0) * 3.3) * 4);
    display.setTextSize(1); display.print(" "); display.setTextSize(3);
    display.print("V");
    // display.display();
    


    // Serial.println("middleLDR_val > 1000");
    if (!dc_motor_open) {
      // Serial.println("Opening DC Motor");
      analogWrite(dc_motor_enable, 150);

      //moves motor ccw
      digitalWrite(dc_motor_input1, HIGH);
      digitalWrite(dc_motor_input2, LOW);
      delay(775);

      //stops motor
      digitalWrite(dc_motor_input1, LOW);
      digitalWrite(dc_motor_input2, LOW);
      delay(1500);
    }
    dc_motor_open = true;

    //Start tracking
    int dtime = 5;
    int tolerance_horiz = 200;
    int tolerance_vert = 300;
    int avg_top = (tlLDR_val + trLDR_val) / 2;
    int avg_bottom = (blLDR_val + brLDR_val) / 2;
    int avg_left = (tlLDR_val + blLDR_val) / 2;
    int avg_right = (trLDR_val + brLDR_val) / 2;
    int diff_vert = avg_top - avg_bottom;
    int diff_horiz = avg_left - avg_right;

    // //Print LDR Values
    // sprintf(buffer, "Middle: %d Diff Vert: %d Diff Horiz: %d Top Avg: %d Bottom Avg: %d Left Avg: %d Right Avg: %d", middleLDR_val, diff_vert, diff_horiz, avg_top, avg_bottom, avg_left, avg_right);
    // Serial.println(buffer);

    //Checks if the difference between the top and bottom ldrs is greater than the tolerance value
    if (abs(diff_vert) > tolerance_vert) {
      //Checks if top value is greater than the bottom value
      if (avg_top > avg_bottom) {
        servot_angle = servot_angle + 1;
        //If angle is greater than max, then stay at max
        if (servot_angle > servot_max) {
          servot_angle = servot_max;
        }
        else{
          display.fillTriangle(50, 25, 45, 35, 55, 35, WHITE);
        }
      }
      //Checks if top value is less than the bottom value
      else if (avg_top < avg_bottom) {
        servot_angle = servot_angle - 1;
        //If angle is less than min, then stay at min
        if (servot_angle < servot_min) {
          servot_angle = servot_min;
        }
        else{
          display.fillTriangle(45, 25, 55, 25, 50, 35, WHITE);
        }
      }
      Topservo.write(servot_angle);
    }


    //Checks if the difference between the left and right ldrs is greater than the tolerance value
    if (abs(diff_horiz) > tolerance_horiz) {
      //Checks if left value is greater than the right value
      if (avg_left > avg_right) {
        servob_angle = servob_angle - 1;
        //If angle is greater than max, then stay at max
        if (servob_angle > servob_max) {
          servob_angle = servob_max;
        }
        else{
          display.fillTriangle(75, 30, 85, 25, 85, 35, WHITE);
        }
      }
      //Checks if left value is less than the right value
      else if (avg_left < avg_right) {
        servob_angle = servob_angle + 1;
        //If angle is less than min, then stay at min
        if (servob_angle < servob_min) {
          servob_angle = servob_min;
        }
        else{
          display.fillTriangle(75, 25, 75, 35, 85, 30, WHITE);
        }
      }
      Bottomservo.write(servob_angle);
    }
    display.display();
    delay(dtime);
  }


  //If LDR value is <= 1000
  else {
    // Serial.println("middleLDR_val <= 1000");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(35, 0);
    display.println("Solar");
    display.setCursor(27, 17);
    display.println("Tracker");
    display.fillCircle(64, 50, 10, WHITE);
    display.fillCircle(59, 45, 5, BLACK);
    display.display();

    //Close panels if open
    if (dc_motor_open == true) {
      // Serial.println("Closing DC Motor");
      analogWrite(dc_motor_enable, 75);

      //moves motor cw
      digitalWrite(dc_motor_input1, LOW);
      digitalWrite(dc_motor_input2, HIGH);
      delay(650);

      //stops motor
      digitalWrite(dc_motor_input1, LOW);
      digitalWrite(dc_motor_input2, LOW);
      delay(1500);
    }
    dc_motor_open = false;

    // Move bottom servo to home
    cur_angle = Bottomservo.read();
    if (abs(cur_angle - servob_angle_home) > 3) {
      if (cur_angle < servob_angle_home) {
        Bottomservo.write(cur_angle + 3);
      } else {
        Bottomservo.write(cur_angle - 3);
      }
    }
    servob_angle = cur_angle;

    // Move top servo to home
    cur_angle = Topservo.read();
    if (abs(cur_angle - servot_angle_home) > 3) {
      if (cur_angle < servot_angle_home) {
        Topservo.write(cur_angle + 3);
      } else {
        Topservo.write(cur_angle - 3);
      }
    }
    servot_angle = cur_angle;

    delay(100);
  }
}