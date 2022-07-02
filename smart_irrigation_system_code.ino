#include <Arduino_FreeRTOS.h>
#include<LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Wire.h>


#define echo 9
#define trigger 10
#define tank_pump 4
#define watering_pump 13
#define moisture_sensor A0
long duration = 0;
int distance = 0;
int moisture_value = 0;
int distance_percent = 0;
int moist_percent = 0;
String ip_stat = "";
String wtp_stat = "";

SoftwareSerial SIM900(2, 3);
LiquidCrystal lcd(12, 11, 8, 7, 6, 5);

//////////////////////////////////////////////////////////////////////////////////
void lcd_msg( void *pvParameters );
void check_moist( void *pvParameters );
void check_water( void *pvParameters );
void irrigation_pump( void *pvParameters );
void water_pump( void *pvParameters );

TaskHandle_t TaskHandle_3;
TaskHandle_t TaskHandle_4;
//////////////////////////////////////////////////////////////////////////////////

void setup () {

  lcd.begin(20, 4);
  SIM900.begin(9600);
  Serial.begin(9600);
  pinMode(echo, INPUT);
  pinMode(moisture_sensor, INPUT);
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger, LOW);
  pinMode(watering_pump, OUTPUT);
  pinMode(tank_pump, OUTPUT);
  digitalWrite(watering_pump, LOW);
  digitalWrite(tank_pump, LOW);

  //////////////////////////////////////////////////////////////////////////////////
  xTaskCreate(check_moist, NULL, 150, NULL, 3, NULL);
  xTaskCreate(check_water, NULL, 150, NULL, 3, NULL);
  xTaskCreate(irrigation_pump, NULL, 150, NULL, 4, &TaskHandle_3);
  xTaskCreate(water_pump, NULL, 150, NULL, 4, &TaskHandle_4);
  //xTaskCreate(lcd_msg, NULL, 150, NULL, 7, NULL);

  vTaskStartScheduler();
  //////////////////////////////////////////////////////////////////////////////////
}

void loop() {
  
}

//////////////////////////////////////////////////////////////////////////////////

void check_moist(void* pvParam) {
  (void) pvParam;
  for (;;) {
    moisture_value = analogRead(moisture_sensor);
    moist_percent = map(moisture_value, 0, 1023, 0, 100);
    
    SIM900.print("moist_percent: ");
    SIM900.println(moist_percent);
    vTaskDelay(110/portTICK_PERIOD_MS);
  }
}

void check_water(void* pvParam) {
  (void) pvParam;
  for (;;) {
    //WATER LEVEL SENSOR
    digitalWrite(trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    duration = pulseIn(echo, HIGH);
    distance = duration * 0.017;
    distance_percent = map( distance, 0, 1023, 0, 100);
    lcd.clear();
    lcd.print("distance_percent: ");
    lcd.print(distance_percent);

    vTaskDelay(110/portTICK_PERIOD_MS);
  }
}

void irrigation_pump(void* pvParam) {
  (void) pvParam;
  for (;;) {
    if (moist_percent > 85) {
      digitalWrite(watering_pump, LOW);
    }
    else {
      digitalWrite(watering_pump, HIGH);
    }

    vTaskDelay(110/portTICK_PERIOD_MS);
  }
}

void water_pump(void* pvParam) {
  (void) pvParam;
  for (;;) {
    if (distance_percent > 65) {
      digitalWrite(tank_pump, LOW);
      vTaskPrioritySet(TaskHandle_3,4);
    }
    else {
      digitalWrite(tank_pump, HIGH);
      vTaskPrioritySet(TaskHandle_3,5);
    }

    vTaskDelay(110/portTICK_PERIOD_MS);
  }
}

void lcd_msg(void* pvParam) {
  (void) pvParam;
  for (;;) {
    int w = digitalRead(watering_pump);
    int t = digitalRead(tank_pump);
    if (w == 0) {
      wtp_stat = " off";
    } else {
      wtp_stat = " on";
    }
    if (t == 0) {
      ip_stat = " off";
    } else {
      ip_stat = " on";
    }

    SIM900.println("WATER TANK PUMP IS");
    SIM900.println(wtp_stat);
    SIM900.println((char)26); // End AT command with a Upper Case Z, which is ASCII code 26
    SIM900.println();

    SIM900.println("IRRIGATION PUMP IS");
    SIM900.println(ip_stat);
    SIM900.println((char)26); // End AT command with a Upper Case Z, which is ASCII code 26
    SIM900.println();

    //LCD();

    delay(100);
  }
}
//////////////////////////////////////////////////////////////////////////////////


void LCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WATER TANK LEVEL=");
  lcd.print(distance_percent);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("SOIL MOISTURE= ");
  lcd.print(moist_percent);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("IRRIGATION PUMP");
  lcd.print(ip_stat);
  lcd.setCursor(0, 3);
  lcd.print("WATER TANK PUMP");
  lcd.print(wtp_stat);
}
