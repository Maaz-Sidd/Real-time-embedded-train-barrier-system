#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Servo.h>
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
LiquidCrystal lcd(1, 2, 4, 5, 6, 7); // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7) 

Servo myservo;


// Pin configuration for ultrasonic sensor
const int trigPin = 49;
const int echoPin = 3;
unsigned long CurrTime, prevTime = 0;
volatile float prevDist = 0;
volatile float distance = 0;
float velocity = 0;
volatile int pos = 40;
unsigned long train_time = 0;


TaskHandle_t distanceTask;
TaskHandle_t LED_Blink;
TaskHandle_t BarDown;
TaskHandle_t BarUp;
TaskHandle_t buzzer;

int counter = 0;

typedef struct sensorInfo{
  float SensorDist;
  unsigned long time;
};

// Function prototypes
void ultrasonicTask(void *pvParameters);
void ledBlink(void *pvParameters);
void bar_down(void *pvParameters);
void bar_up(void *pvParameters);
void Buzzer(void *pvParameters);
float getDistance();

void setup() {
  Serial.begin(9600);
  myservo.write(40);
  lcd.begin(16,2);

 
  xTaskCreate(ultrasonicTask, "UltrasonicTask", 1000, NULL, 2, &distanceTask);
  xTaskCreate(ledBlink, "LEDBlinkTask", 1000, NULL, 1, &LED_Blink);
  xTaskCreate(bar_down, "ServoTask", 1000, NULL, 2, &BarDown);
  xTaskCreate(bar_up, "ServoTask", 1000, NULL, 1, &BarUp);
  xTaskCreate(Buzzer, "Buzzertask", 1000, NULL, 1, &buzzer);

  vTaskSuspend(LED_Blink);
  vTaskSuspend(BarDown);
  vTaskSuspend(BarUp);
  vTaskSuspend(buzzer);
  myservo.attach(9);

  pinMode(52, OUTPUT);
  vTaskStartScheduler();
}

void loop() {
  // Empty, as tasks will be running
}

void ultrasonicTask(void *pvParameters) {
  (void) pvParameters;

  sensorInfo Ultrasonic;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  while (1) {

  //for (int i = 0; i <5; i++){
    digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  // Speed of sound is 343 meters/second or 0.0343 cm/microsecond
  distance = (duration * 0.0343) / 2;

  CurrTime = millis();

  float deltaDist = prevDist - distance;
  float deltatime = prevTime CurrTime;

  velocity = deltaDist/deltatime;

  if ( distance < 75 || ((distance+100)/velocity > 6)){
    train_time = millis();
    vTaskResume(LED_Blink);
    vTaskResume(buzzer);
    vTaskSuspend(NULL);
  }

  prevTime = CurrTime;
  prevDist = distance;


    // Delay for a short time before the next measurement
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void ledBlink(void *pvParameters) {
  (void) pvParameters;
while(1){
  unsigned long time_pass = millis();
  digitalWrite(33, HIGH);  // turn the LED on (HIGH is the voltage level)
  vTaskDelay(pdMS_TO_TICKS(250));                      // wait for a second
  digitalWrite(33, LOW);   // turn the LED off by making the voltage LOW
  vTaskDelay(pdMS_TO_TICKS(250)); 
  if ((time_pass-train_time) > 1000){
    vTaskResume(BarDown);
  }


  if((time_pass - train_time) > 6000){
    counter ++;
    if (counter == 2){
      vTaskResume(distanceTask);
      vTaskSuspend(buzzer);
      digitalWrite(52, LOW);
      vTaskResume(BarUp);
      vTaskSuspend(NULL);
      distance = 100;
      velocity = 1;
      counter = 0;
    }
    Serial.println(time_pass - train_time);
  }
  vTaskDelay(pdMS_TO_TICKS(500));   
}
}

void bar_down(void *pvParameters) {
  (void) pvParameters;
while(1){
   while(pos <= 145) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    pos += 2;
    vTaskDelay(pdMS_TO_TICKS(50));                       // waits 15ms for the servo to reach the position
  }
vTaskSuspend(NULL);
}
}


void bar_up(void *pvParameters) {
  (void) pvParameters;
while(1){
   while(pos >= 40) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    pos -= 2;
    vTaskDelay(pdMS_TO_TICKS(50));                       // waits 15ms for the servo to reach the position
  }
vTaskSuspend(NULL);
}
}

void Buzzer(void *pvParameters){
  (void) pvParameters;
while(1){
  digitalWrite(52, HIGH);
  vTaskDelay(pdMS_TO_TICKS(500)); 
  digitalWrite(52, LOW);
  vTaskDelay(pdMS_TO_TICKS(500)); 
}
}


