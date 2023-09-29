// Define ultrasonic sensor
#define TRIGGER_PIN 11
#define ECHO_PIN 10

// Bluetooth module RX and TX pins are attached to D2 and D4, so this carries out Serial communication

// Define motor control pins
const int leftMotorPin1 = 3;
const int leftMotorPin2 = 5;
const int rightMotorPin1 = 9;
const int rightMotorPin2 = 10;

// Define servo pin
const int servo = 6;

// Global variables
bool isMoving = false;
bool isDrawing = false;

// Servo motor settings
int servoNeutralPosition = 90;                    // The neutral position of the servo
int servoDrawingPosition = 45;                    // The angle for drawing
int currentServoPosition = servoNeutralPosition;  // Current servo position

// Motor speed settings (0 to 255, where 0 is stopped and 255 is maximum speed)
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;

// Variables for mobile sensor data
int mobileSensorX = 0;
int mobileSensorY = 0;
int mobileSensorZ = 0;

// Constants for control modes
const byte JOYSTICK_CONTROL = 0;
const byte MOBILE_SENSOR_CONTROL = 1;

// Control mode (JOYSTICK_CONTROL or SENSOR_CONTROL)
byte controlMode = JOYSTICK_CONTROL;  // Default to joystick control

void setup() {
  // Initialize servo, ultrasonic sensor
  pinMode(servo, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


  // Initialize motor control pins as outputs
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);

  Serial1.begin(9600);

  // Set the servo to its neutral position
  servo_write(servoNeutralPosition);
}

void loop() {
  // Check for incoming Bluetooth commands
  char cmd = 'N';  // Initialising a default command of choice
  if (Serial1.available() > 0) {
    char command = Serial1.read();
    cmd = command;
    processCommand(command);
  }

  // Continuously monitor obstacles
  int distance = measureDistance();

  // Send ultrasonic sensor data to the app
  sendSensorData(distance);

  // Receive the data from the app.
  processCommand(cmd);
}

// Function to measure distance using ultrasonic sensor
int measureDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration / 58;
  return distance;
}

// Function to send sensor data via Bluetooth
void sendSensorData(int distance) {
  // The sensor data is used within the app to create a movement graph, analyze the environment around the bot, possibly integrated AR experience, and so on.
  Serial1.print("D");         // Identifier for sensor data
  Serial1.println(distance);  // Send the distance data
}

void processCommand(char command) {

  int leftSpeed, rightSpeed;

  switch (command) {
    case 'U':
      // Raise drawing mechanism by setting a constant angle
      adjustServoAngle(servoDrawingPosition);
      break;

    case 'C':
      // Lower drawing mechanism by setting a constant angle
      adjustServoAngle(servoNeutralPosition);
      break;

    case 'B':
      // Reverse movement command
      leftSpeed = Serial1.parseInt();   // Read left motor speed from Bluetooth
      rightSpeed = Serial1.parseInt();  // Read right motor speed from Bluetooth
      controlMotors(-leftSpeed, -rightSpeed);
      break;

    case 'F':
      // Set custom motor speeds (forwards)
      leftSpeed = Serial1.parseInt();
      rightSpeed = Serial1.parseInt();
      setMotorSpeeds(leftSpeed, rightSpeed);
      break;

    case 'N':
      // Set custom motor speeds (neutral)
      setMotorSpeeds(0, 0);  // Stop
      break;

    case 'J':
      // Switch to joystick control mode
      controlMode = JOYSTICK_CONTROL;
      break;

    case 'S':
      // Switch to sensor control mode
      controlMode = MOBILE_SENSOR_CONTROL;
      break;

    case 'A':
      // Adjust servo angle
      int newServoAngle = Serial1.parseInt();  // Read new servo angle from Bluetooth
      // Error handling: Ensure valid servo angle values (0 to 180)
      if (newServoAngle >= 0 && newServoAngle <= 180) {
        adjustServoAngle(newServoAngle);
      }
      break;
  }
}

void controlMotors(int leftSpeed, int rightSpeed) {
  // Check the current control mode (joystick or sensor)
  if (leftSpeed >= -255 && leftSpeed <= 255 && rightSpeed >= -255 && rightSpeed <= 255) {
    if (controlMode == MOBILE_SENSOR_CONTROL) {
      analogWrite(leftMotorPin1, map(leftSpeed, -90, 90, -255, 255));
      digitalWrite(leftMotorPin2, LOW);
      analogWrite(rightMotorPin1, map(rightSpeed, -90, 90, -255, 255));
      digitalWrite(rightMotorPin2, LOW);

    } else if (controlMode == JOYSTICK_CONTROL) {
      analogWrite(leftMotorPin1, leftSpeed);
      digitalWrite(leftMotorPin2, LOW);
      analogWrite(rightMotorPin1, rightSpeed);
      digitalWrite(rightMotorPin2, LOW);
    }
  } else {
    // Handle invalid speed values (stop motors)
    setMotorSpeeds(0, 0);
  }
}

void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  // Set custom motor speeds for forward motion
  leftMotorSpeed = leftSpeed;
  rightMotorSpeed = rightSpeed;
}

bool servo_write(int deg)
{ // Custom function to set servo angle
  const int MIN_PULSE_WIDTH = 544;
  const int MAX_PULSE_WIDTH = 2200;

  if (deg < 0 || deg > 180) {
    // Handle invalid degree value
    return false;
  }

  int time = 0;  //where "pin" is the pin number of the gpio conneted to the output of the servo.
  time = map(deg, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);

  digitalWrite(servo, HIGH);
  delayMicroseconds(time);
  digitalWrite(servo, LOW);
  delay(20);

  return true;  // Successful servo movement
}

void adjustServoAngle(int newAngle) {
  // Adjust the servo angle based on the input
  // Ensure the new angle is within a valid range (0 to 180 degrees)
  if (newAngle >= 0 && newAngle <= 180) {
    currentServoPosition = newAngle;
    // Move the servo to the new angle
    servo_write(currentServoPosition);
  }
}
