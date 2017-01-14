#include <Servo.h> 

Servo esc; 
Servo servo;  

const int BUTTON_1_PIN = 3;
const int BUTTON_2_PIN = 2;
const int BUTTON_3_PIN = 5;
const int BUTTON_4_PIN = 4;
const int THROTTLE_PIN = 6;
const int WHEEL_PIN = 7;
const int ESC_PIN = 9;
const int SERVO_PIN = 10;
const int LED_PIN = 13;
const int ESC_ARM = 86;

// Car Modes
// 0 - Startup
// 1 - RC Record Mode
// 2 - Python Playback Mode
// 3 - TensorFlow Drive Mode
// 4 - Stop Button
// 5 - Stop Mode 2 or 3 Timeout
// 6 - RC stop while in mode 2 or 3
volatile int mode = 0;
volatile int button_mode = 0;
volatile long last_command_time = 0;
volatile int serial_steering = 90;
volatile int serial_throttle = 86;

String inputString = "";    
volatile boolean stringComplete = false;

void setup()
{
  Serial.begin(9600);

  pinMode(BUTTON_1_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_2_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_3_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_4_PIN, INPUT_PULLUP); 
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(WHEEL_PIN, INPUT);
  esc.attach(ESC_PIN);
  servo.attach(SERVO_PIN);
  pinMode(LED_PIN, OUTPUT);

  // Reserve 200 bytes for the inputString
  inputString.reserve(200);
  
  // Arm the ESC
  esc.write(ESC_ARM);
  delay(2000);
}

int clamp(int x, int low, int high) {
  if (x < low) return low;
  if (x > high) return high;
  return x;
}

void loop()
{
  long start_time = micros();
  String serial_command = "";

  // serialEvent does not work on the Arduino Micro's 
  // so read the serial input here in the main loop
  while (Serial.available()) {
    // Get the new byte
    char inChar = (char)Serial.read();
    // Add it to the inputString
    inputString += inChar;
    // If the incoming character is a newline, set a flag
    if (inChar == '\n') {
      stringComplete = true;
    }
  }

  if (stringComplete) {
    // Commands look like: T120S120M0 or T045S045M2
    serial_throttle = inputString.substring(1,4).toInt();
    serial_steering = inputString.substring(5,8).toInt();
    int mode_command = inputString.substring(9).toInt();

    // The serial controller can only stop the car, it can not start it
    if (mode_command >= 4) {
      mode = mode_command;
    }

    // Dont save the \n on the end of the serial command
    serial_command = inputString.substring(1,9);
    last_command_time = start_time;

    // Clear the string
    inputString = "";
    stringComplete = false;
  }

  // Read the length of the pulse in microseconds
  int rc_steering = (int)pulseIn(THROTTLE_PIN, HIGH, 25000);
  int rc_throttle = (int)pulseIn(WHEEL_PIN, HIGH, 25000);

  // Observed ranges:
  // - steering: min = 1175, center = 1475, max = 1840
  // - throttle: min = 1010, center = 1465, max = 1975

  // Deliberately use less precision to avoid 16-bit overflow
  const int loss = 5;

  // Map input to: min = 0, center = 90, max = 180
  rc_steering = clamp(90 + (rc_steering - 1475) * (90 / loss) / (300 / loss), 0, 180);
  rc_throttle = clamp(90 + (rc_throttle - 1465) * (90 / loss) / (455 / loss), 0, 180);

  // Prevent rc jitter
  if (rc_steering > 81 && rc_steering < 97) {
    rc_steering = 90;
  }

  // If in a serial driven mode and an extremem rc input is detected stop.
  if (mode == 2 || mode == 3) {
    if (rc_steering == 0 || rc_steering == 180 || rc_throttle == 0 || rc_throttle == 180) {
      mode = 6;
    }
  }

  // Button 4 is the most important as it is stop
  // buttons are read after serial input as they should override
  if (digitalRead(BUTTON_4_PIN) == LOW) {
    mode = 4;
    button_mode = 4;
  } else if (digitalRead(BUTTON_1_PIN) == LOW) {
    digitalWrite(LED_PIN, LOW);
    mode = 1;
    button_mode = 1;
  } else if (digitalRead(BUTTON_2_PIN) == LOW) {
    digitalWrite(LED_PIN, LOW);
    mode = 2;
    button_mode = 2;
    last_command_time = start_time + 15000000;
  } else if (digitalRead(BUTTON_3_PIN) == LOW) {
    digitalWrite(LED_PIN, LOW);
    mode = 3;
    button_mode = 3;
    last_command_time = start_time + 15000000;
  }

  int car_steering = 90; 
  int car_throttle = 86; 
  digitalWrite(LED_PIN, LOW);
  // Mode 4, 5, 6 are the most important as they are stop
  if (mode >= 4) {
    digitalWrite(LED_PIN, HIGH);
  } else if (mode == 1) {
    car_steering = rc_steering;
    car_throttle = rc_throttle - 5; // RC controler middle is 90
  } else if (mode == 2 || mode == 3) {
    if (last_command_time + 15000000 > start_time) {
      car_steering = serial_steering;
      car_throttle = serial_throttle;
    } else {
      mode = 5;
    }
  }
  esc.write(car_throttle);
  servo.write(car_steering);
  
  // Want nice even serial writes out so add necissary time
  long run_time = micros() - start_time;
  if (run_time < 25000) {
    delayMicroseconds(25000 - run_time);
  } else {
    Serial.println("WENT OVER 25ms");
  }

  Serial.print("M");
  Serial.print(mode);
  Serial.print(" T");
  Serial.print(car_throttle);
  Serial.print(" S");
  Serial.print(car_steering);
  Serial.print(" B");
  Serial.print(button_mode);
  Serial.print(" ");
  Serial.println(serial_command);
}
