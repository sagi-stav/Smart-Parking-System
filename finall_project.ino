#include <Servo.h>
#include <SoftwareSerial.h>

Servo entranceServo;
Servo exitServo;

#define BUZZER 6 // Single buzzer for both entrance and exit alerts

const int buttonPin = 5; // Button pin for exit
const int entranceTrigPin = 13;
const int entranceEchoPin = 12;
const int exitTrigPin = 4;
const int exitEchoPin = 3;
const int entranceRedLEDPin = 7; // Entrance Red LED pin
const int entranceGreenLEDPin = 8; // Entrance Green LED pin
const int maxSpaces = 5; // Total parking spaces

int sensorPin = A0; // Select the input pin for LDR
int sensorValue = 0; // Variable to store the value coming from the sensor

int availableSpaces = maxSpaces;
int buttonState = 0; // Variable for reading the pushbutton status

float duration;
float distance;

SoftwareSerial Bluetooth(10, 11); // RX, TX

void setup() {
  entranceServo.attach(9); // Entrance servo on pin 9
  exitServo.attach(2); // Exit servo on pin 2
  
  entranceServo.write(0); // Initial position of the entrance servo
  exitServo.write(0); // Initial position of the exit servo
  
  Serial.begin(9600); // Serial communication for debugging
  Bluetooth.begin(9600); // Bluetooth communication
  
  pinMode(BUZZER, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  pinMode(entranceTrigPin, OUTPUT);
  pinMode(entranceEchoPin, INPUT);
  pinMode(exitTrigPin, OUTPUT);
  pinMode(exitEchoPin, INPUT);
  
  pinMode(entranceRedLEDPin, OUTPUT);
  pinMode(entranceGreenLEDPin, OUTPUT);
  
  digitalWrite(entranceRedLEDPin, LOW); // Initially, turn off LEDs
  digitalWrite(entranceGreenLEDPin, LOW);
}

void loop() {      
  // Check for vehicle at entrance
  float entranceDistance = getDistance(entranceTrigPin, entranceEchoPin);
  // Check for vehicle at exit
  float exitDistance = getDistance(exitTrigPin, exitEchoPin);
  
  sensorValue = analogRead(sensorPin);
  if (sensorValue >= 500) { 
    if (Bluetooth.available()) {
      char command = Bluetooth.read();
      // Entrance logic
      if (command == '1' && entranceDistance < 50 && availableSpaces > 0) {
        entranceServo.write(90); // Open entrance gate
        digitalWrite(entranceGreenLEDPin, HIGH); // Turn on green LED
        Bluetooth.println("Vehicle entering");
        Serial.println("Vehicle entering");
        tone(BUZZER, 2000); // Activate buzzer
        delay(3000); // Wait for the vehicle to pass
        entranceServo.write(0); // Close entrance gate
        digitalWrite(entranceGreenLEDPin, LOW); // Turn off green LED
        noTone(BUZZER); // Deactivate buzzer
        availableSpaces--; // Decrease available spaces
        Bluetooth.print("Spaces left: ");
        Bluetooth.println(availableSpaces);
        Serial.print("Spaces left: ");
        Serial.println(availableSpaces);
      } else if (command == '1' && entranceDistance >= 50) {
        Bluetooth.println("No vehicle detected at entrance");
        Serial.println("No vehicle detected at entrance");
        digitalWrite(entranceRedLEDPin, HIGH); // Turn on red LED
        tone(BUZZER, 2000); // Activate buzzer
        delay(1000);
        digitalWrite(entranceRedLEDPin, LOW); // Turn off red LED
        noTone(BUZZER); // Deactivate buzzer
      } else if (command == '1' && availableSpaces <= 0) {
        Bluetooth.println("No available spaces");
        Serial.println("No available spaces");
        digitalWrite(entranceRedLEDPin, HIGH); // Turn on red LED
        tone(BUZZER, 2000); // Activate buzzer
        delay(1000);
        digitalWrite(entranceRedLEDPin, LOW); // Turn off red LED
        noTone(BUZZER); // Deactivate buzzer
      }
    }

    // Exit logic
    buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH && exitDistance < 50) {
      exitServo.write(90); // Open exit gate
      Bluetooth.println("Vehicle exiting");
      Serial.println("Vehicle exiting");
      tone(BUZZER, 2000); // Activate buzzer
      delay(3000); // Wait for the vehicle to pass
      exitServo.write(0); // Close exit gate
      noTone(BUZZER); // Deactivate buzzer
      availableSpaces++; // Increase available spaces
      if (availableSpaces > maxSpaces) {
        availableSpaces = maxSpaces; // Prevent available spaces from exceeding maxSpaces
      }
      Bluetooth.print("Spaces left: ");
      Bluetooth.println(availableSpaces);
      Serial.print("Spaces left: ");
      Serial.println(availableSpaces);
    } else if (buttonState == HIGH && exitDistance >= 50) {
      Bluetooth.println("No vehicle detected at exit");
      Serial.println("No vehicle detected at exit");
      tone(BUZZER, 2000); // Activate buzzer
      delay(1000);
      noTone(BUZZER); // Deactivate buzzer
    }
  } else {
    Serial.print("sensorValue: ");
    Serial.print(sensorValue);
    Serial.println(" - It's dark, the parking lot is only open during the day.");
  }

  delay(500); // Delay for stability
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}
