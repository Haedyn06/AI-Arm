#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

//WIFI
const char* user = "Wifi";   
const char* pw = "Password";    
const char* serverIP = "192.168.1.XXX";  
const int servPort = 1883;       

WiFiClient espClient;
PubSubClient client(espClient);

//SERVER
const char* Global[] = {"global/esp32", "global/pc", "global/rpi"};
const char* Send[] = {"pc/esp32", "rpi/esp32"};
const char* Receive[] = {"esp32/pc", "esp32/rpi"};


// SERVO
Servo J1;
Servo J2;
Servo J3;
Servo J4;

#define Joint1 16
#define Joint2 17
#define Joint3 18
#define Joint4 19

int AngleJ1 = 90; 
int AngleJ2 = 90;
int AngleJ3 = 90;
int AngleJ4 = 90;

int Increments = 25;
int DelayTime = 50;

// Joystick Controller
#define Xpin  35 
#define Ypin  34 
#define Button 32

#define LUhold  1000
#define RDhold  3000

int Xval = 0;
int Yval = 0;
bool stateControlX = true;  // Toggles between J2 and J3 for Left/Right
bool stateControlY = true;  // Toggles between J1 and J4 for Up/Down

//FUNCTION DECLARATIONS
void moveServo(Servo &servo, int &angle, int change);
void printAngles();
void processCommand(String cmd);
void callback(char* topic, byte* payload, unsigned int length);
void msgAck(String message, int timed);

void setup() {
  Serial.begin(115200);


  // WIFI
  WiFi.begin(user, pw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  client.setServer(serverIP, servPort);
  client.setCallback(callback);

  if (client.connect("ESP32Client")) {
    Serial.println("Connected to MQTT");
    for (const char* subTopic : Receive) client.subscribe(subTopic);
  }

  //SERVOS
  J1.attach(Joint1);
  J1.write(AngleJ1);

  J2.attach(Joint2);
  J2.write(AngleJ2);

  J3.attach(Joint3);
  J3.write(AngleJ3);

  J4.attach(Joint4);
  J4.write(AngleJ4);

  pinMode(Button, INPUT_PULLUP);
  processCommand("idle");
}

void loop() {
  client.loop();

  Xval = analogRead(Xpin);
  Yval = analogRead(Ypin);
  bool moved = false;

  // Right/Left controls Joint 2 or Joint 3
  if (Xval < LUhold) {
    Serial.print("Left - ");
    if (stateControlX) {
      Serial.println("Joint 2");
      moveServo(J2, AngleJ2, Increments);
    } else {
      Serial.println("Joint 3");
      moveServo(J3, AngleJ3, -Increments);
    }
    moved = true;
  } else if (Xval > RDhold) {
    Serial.print("Right - ");
    if (stateControlX) {
      Serial.println("Joint 2");
      moveServo(J2, AngleJ2, -Increments);
    } else {
      Serial.println("Joint 3");
      moveServo(J3, AngleJ3, Increments);
    }
    moved = true;
  }

  // Up/Down controls Joint 1 or Joint 4
  if (Yval < LUhold) {
    Serial.print("Up - ");
    if (stateControlY) {
      Serial.println("Joint 1");
      moveServo(J1, AngleJ1, Increments);
    } else {
      Serial.println("Joint 4");
      moveServo(J4, AngleJ4, Increments);
    }
    moved = true;
  } else if (Yval > RDhold) {
    Serial.print("Down - ");
    if (stateControlY) {
      Serial.println("Joint 1");
      moveServo(J1, AngleJ1, -Increments);
    } else {
      Serial.println("Joint 4");
      moveServo(J4, AngleJ4, -Increments);
    }
    moved = true;
  }

  // Button toggles control mode
  if (digitalRead(Button) == LOW) {
    stateControlX = !stateControlX;  // Switch between Joint 2 and Joint 3 for Left/Right
    stateControlY = !stateControlY;  // Switch between Joint 1 and Joint 4 for Up/Down
    Serial.print("Switched Control Mode | Left/Right: ");
    Serial.print(stateControlX ? "Joint 2" : "Joint 3");
    Serial.print(" | Up/Down: ");
    Serial.println(stateControlY ? "Joint 1" : "Joint 4");
    delay(300);  // Debounce
  }

  if (moved) {
    printAngles();
  }

  delay(100);
/*   if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    delay(2500);
    processCommand(cmd);
  }
  delay(500); */
  // processCommand("idle");

}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received message on ");
  Serial.print(topic);
  Serial.print(": ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
      message += (char)payload[i];
  }
  Serial.println(message);

  //Direct Message From PC
  if (String(topic) == "esp32/pc") {
    Serial.println(" Message from PC");
    if (message == "hello") msgAck("wave", 800);
    else if (message == "point") msgAck("point", 1200);
    else if (message == "hold") msgAck("hold", 1200);
    else if (message == "tpose") msgAck("tpose", 1200);
    else if (message == "salute") msgAck("salute", 1200);
    else if (message == "hug") msgAck("hug", 1200);
    else if (message == "attack") msgAck("attack", 1200);
    else if (message == "raise") msgAck("raise", 1200);
    else if (message == "run") msgAck("run", 1200);
    else if (message == "talk") msgAck("talk", 1200);
    else if (message == "sus") msgAck("sus", 1200);
    else if (message == "rando") msgAck("rando", 1200);
  }
  
  //Direct Message From rpi
  if (String(topic) == "esp32/rpi") {
    Serial.println("📩 Message from Raspberry Pi");
    Serial.println(message);
  }
}

void moveServo(Servo &servo, int &angle, int change) {
  angle = constrain(angle + change, 0, 180);
  servo.write(angle);
}

void printAngles() {
  Serial.print("J1: "); Serial.print(AngleJ1);
  Serial.print(" | J2: "); Serial.print(AngleJ2);
  Serial.print(" | J3: "); Serial.print(AngleJ3);
  Serial.print(" | J4: "); Serial.println(AngleJ4);
}

void servosPos(int a, int b, int c, int d, int delayTime = 0) {
  J1.write(a);
  J2.write(b);
  J3.write(c);
  J4.write(d);
  if (delayTime > 0) delay(delayTime);
}

/* 
IDLE = 0 X 180 0
WAVE 75 X 80 0 > 75 X 80 180
POINT 100 X 180 0
HOLD 0 X 180 175
TPOSE 0 X 80 0
SALUTE 125 X 180 0
SHAKE 0 X 105 75
DONT KNOW 75 X 105 75
TALK  0 X 180 55 > 0 X 130 55
ATTACK 0 X 105 100 > 100 X 180 0 
SUS 0 X 180 180 > 0 X 180 50
RUN 100 X 180 180 > 0 155 180
RANDO 0-180

*/

void processCommand(String cmd) {
  cmd.toLowerCase();

  if(cmd.equalsIgnoreCase("idle")) servosPos(5, 105, 105, 180); // IDLE 5 105 105 180 
  else if(cmd.equalsIgnoreCase("point")) servosPos(100, 125, 155, 100); // Point 100 105 75 155 
  else if(cmd.equalsIgnoreCase("hold")) servosPos(75, 150, 155, 100); // Hold 0 80 50 75 
  else if(cmd.equalsIgnoreCase("tpose")) servosPos(0, 55, 125, 180); // T Pose 0 55 125 180 
  else if(cmd.equalsIgnoreCase("salute")) servosPos(100, 125, 155, 100); // Salute 150 180 90 155
  else if (cmd.equalsIgnoreCase("hug")) servosPos(75, 105, 25, 155); // Hug 75 105 25 155

  else if(cmd.equalsIgnoreCase("wave")) { // Wave 180 80 100 180 <==> 180 80 0 180
    for (int i = 0; i <= 2; i++) {
      servosPos(180, 80, 100, 180, 350);
      servosPos(180, 80, 0, 180, 350);
    }
  } else if (cmd.equalsIgnoreCase("attack")) { // Attack 180 130 90 90 > 0 130 90 180
    servosPos(180, 130, 90, 90, 350);
    servosPos(0, 130, 90, 180, 350);
  } else if (cmd.equalsIgnoreCase("raise")) { // Hand Up 180 105 105 180
    servosPos(180, 105, 105, 180);
  } else if (cmd.equalsIgnoreCase("run")) { // Run 0 125 80 80 > 75 130 80 80
    for (int i = 0; i <= 6; i++) {
      servosPos(75, 130, 80, 80, 250);
      servosPos(0, 125, 80, 80, 250);
    }
  } else if (cmd.equalsIgnoreCase("talk")) { // Talk 50 175 105 75 > 50 105 105 105 75
    for (int i = 0; i <= 3; i++) {
      servosPos(50, 175, 105, 75, 500);
      servosPos(50, 105, 105, 75, 500);
    }
  } else if (cmd.equalsIgnoreCase("sus")) { // Sus 25 105 75 50 > 25 105 75 125
    for (int i = 0; i <= 3; i++) {
      servosPos(25, 105, 75, 50, 600);
      servosPos(25, 105, 75, 160, 600);
    }
    for (int i = 0; i <= 4; i++) {
      servosPos(25, 105, 75, 50, 400);
      servosPos(25, 105, 75, 160, 400);
    }
    for (int i = 0; i <= 6; i++) {
      servosPos(25, 105, 75, 50, 200);
      servosPos(25, 105, 75, 160, 200);
    }
    for (int i = 0; i <= 10; i++) { 
      servosPos(25, 105, 75, 50, 100);
      servosPos(25, 105, 75, 160, 100);
    }
  } else if (cmd.equalsIgnoreCase("rando")) { // Random movements
    for (int i = 0; i <= 12; i++) servosPos(random(0, 180), random(0, 80), random(25, 150), random(50, 180), 800);
    servosPos(5, 105, 105, 180);
  } else Serial.println("Unknown command. Try: wave, idle, point, hold, tpose, salute, attack, raise, run, hug, talk, sus, rando.");
}

void msgAck(String message, int timed) {
  processCommand(message);
  delay(timed);
  processCommand("idle");
}