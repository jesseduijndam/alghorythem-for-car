#include <ESP8266WiFi.h>

#define triggerPin 0
#define echoPin 2
#define soundSpeed 343.0

/*
 * Webserver setup variables
 */
const char* ssid     = "H369A7B29D6";
const char* password = "632D37DD34D5";

WiFiServer server(80);

String header;

/*
 * Statuses
 */
String motorOneIO1State = "off";
String motorOneIO2State = "off";
String motorTwoIO1State = "off";
String motorTwoIO2State = "off";

String buttonForwardState = "Execute";
String buttonBackwardsState = "Execute";
String buttonLeftState = "Execute";
String buttonRightState = "Execute";

String generalState = "Unknown";
boolean ultrasoonTooClose = false;
boolean autoPilot = false;

/*
 * Set pins
 */
const int motorOneIO1 = 5;        
const int motorOneIO2 = 14;       
const int motorTwoIO1 = 4;        
const int motorTwoIO2 = 12;       
int motor1Speed = 13;
int motor2Speed = 15;

long echoTime = 0;
float distance = 0;

/*
 * Remaining variables
 */
unsigned long currentTime = millis();
unsigned long timerDuration = 0;
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  /*
   * Setting pin modes
   */
  pinMode(motorOneIO1, OUTPUT);
  pinMode(motorOneIO2, OUTPUT);
  pinMode(motorTwoIO1, OUTPUT);
  pinMode(motorTwoIO2, OUTPUT);
  pinMode(motor1Speed, OUTPUT);
  pinMode(motor2Speed, OUTPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  /*
   * Setting digital writes
   */
  digitalWrite(motorOneIO1, LOW);
  digitalWrite(motorOneIO2, LOW);
  digitalWrite(motorTwoIO1, LOW);
  digitalWrite(motorTwoIO2, LOW);
  digitalWrite(motor1Speed, HIGH);
  digitalWrite(motor2Speed, HIGH);

  /*
   * Webserver setup
   */
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  static unsigned long startTime;
  WiFiClient client = server.available();

  /*
   * Start AutoPilot or DrivingAssistant algorithm
   */
  if (autoPilot) {
    autoPilotDriving();
  } else {
    manualDrivingAssistant();
  }
  
  /*
   * Webserver
   */
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            /*
             * Button execution
             */
            if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 on");
              buttonRightState = "Executing";
              buttonLeftState = "Execute";
              buttonBackwardsState = "Execute";
              buttonForwardState = "Execute";
              motorOneIO1State = "on";
              motorOneIO2State = "off";
              motorTwoIO1State = "on";
              motorTwoIO2State = "off";
              digitalWrite(motorOneIO2, LOW);
              digitalWrite(motorTwoIO1, LOW);
              digitalWrite(motorOneIO1, HIGH);
              digitalWrite(motorTwoIO2, HIGH);
            } else if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 off");
              buttonRightState = "Execute";
              motorOneIO1State = "off";
              digitalWrite(motorOneIO1, LOW);
              digitalWrite(motorTwoIO2, LOW);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 on");
              buttonLeftState = "Executing";
              buttonRightState = "Execute";
              buttonBackwardsState = "Execute";
              buttonForwardState = "Execute";
              motorOneIO1State = "off";
              motorOneIO2State = "on";
              motorTwoIO1State = "off";
              motorTwoIO2State = "off";
              digitalWrite(motorOneIO1, LOW);
              digitalWrite(motorTwoIO2, LOW);
              digitalWrite(motorTwoIO1, HIGH);
              digitalWrite(motorOneIO2, HIGH);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 off");
              buttonLeftState = "Execute";
              motorOneIO2State = "off";
              digitalWrite(motorOneIO2, LOW);
              digitalWrite(motorTwoIO1, LOW);
            } else if (header.indexOf("GET /14/off") >= 0) {
              Serial.println("GPIO 14 on");
              buttonBackwardsState = "Executing";
              buttonForwardState = "Execute";
              buttonLeftState = "Execute";
              buttonRightState = "Execute";
              motorOneIO1State = "off";
              motorOneIO2State = "off";
              motorTwoIO1State = "on";
              motorTwoIO2State = "off";
              digitalWrite(motorOneIO2, LOW);
              digitalWrite(motorTwoIO2, LOW);
              digitalWrite(motorTwoIO1, HIGH);
              digitalWrite(motorOneIO1, HIGH);
            } else if (header.indexOf("GET /14/on") >= 0) {
              Serial.println("GPIO 14 off");
              buttonBackwardsState = "Execute";
              motorTwoIO1State = "off";
              digitalWrite(motorTwoIO1, LOW);
              digitalWrite(motorOneIO1, LOW);
            } else if (header.indexOf("GET /12/off") >= 0) {
              Serial.println("GPIO 12 on");
              buttonForwardState = "Executing";
              buttonBackwardsState = "Execute";
              buttonLeftState = "Execute";
              buttonRightState = "Execute";
              motorOneIO1State = "off";
              motorOneIO2State = "on";
              motorTwoIO1State = "off";
              motorTwoIO2State = "on";
              digitalWrite(motorOneIO1, LOW);
              digitalWrite(motorTwoIO2, HIGH);
              digitalWrite(motorTwoIO1, LOW);
              digitalWrite(motorOneIO2, HIGH);
            } else if (header.indexOf("GET /12/on") >= 0) {
              Serial.println("GPIO 12 off");
              buttonForwardState = "Execute";
              motorTwoIO2State = "off";
              motorOneIO1State = "off";
              digitalWrite(motorTwoIO2, LOW);
              digitalWrite(motorOneIO2, LOW);
            } else if (header.indexOf("GET /autoPilot/off") >= 0) {
              Serial.println("Enabling autoPilot driving");
              autoPilot = true;
              toDefaultMode();
            } else if (header.indexOf("GET /autoPilot/on") >= 0) {
              Serial.println("Disabling autoPilot driving");
              autoPilot = false;
              toDefaultMode();
            }
            
            /*
             * Decide general bot status
             */
            if (buttonRightState == "Executing") {
              generalState = "Turning right";
            } else if (buttonLeftState == "Executing") {
              generalState = "Turning left";
            } else if (buttonBackwardsState == "Executing") {
              generalState = "Driving backwards";
            } else if (buttonForwardState == "Executing") {
              generalState = "Driving forward";
            } else if (autoPilot == true) {
              generalState = "Driving on autopilot";
            } else {
              generalState = "Engines off";
            }

            /*
             * Webpage markup
             */
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #327ba8; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #6F91A5;}</style></head>");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button3 { background-color: #5C67A5; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button4 {background-color: #7C82A3;}</style></head>");

            /*
             * Show webpage
             */
            client.println("<body><h1>RescueBot</h1>");
            client.println("<h5>Bot status: \n" + generalState + "</h5>");

            client.println("<h4>Forward</h4>");
            if (buttonForwardState == "Execute") {
              client.println("<p><a href=\"/12/off\"><button class=\"button\">Execute</button></a></p>");
            } else {
              client.println("<p><a href=\"/12/on\"><button class=\"button button2\">Executing</button></a></p>");
            }
            client.println("</body></html>");
            
            client.println("<h4>Backwards</h4>");
            if (buttonBackwardsState == "Execute") {
              client.println("<p><a href=\"/14/off\"><button class=\"button\">Execute</button></a></p>");
            } else {
              client.println("<p><a href=\"/14/on\"><button class=\"button button2\">Executing</button></a></p>");
            }
            client.println("</body></html>");

            client.println("<h4>Left</h4>");
            if (buttonLeftState == "Execute") {
              client.println("<p><a href=\"/4/off\"><button class=\"button\">Execute</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/on\"><button class=\"button button2\">Executing</button></a></p>");
            }
            client.println("</body></html>");
            
            client.println("<h4>Right</h4>");
            if (buttonRightState == "Execute") {
              client.println("<p><a href=\"/5/off\"><button class=\"button\">Execute</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/on\"><button class=\"button button2\">Executing</button></a></p>");
            }
            client.println("</body></html>");

            client.println("<h4>Autopilot</h4>");
            if (autoPilot == false) {
              client.println("<p><a href=\"/autoPilot/off\"><button class=\"button button3\">Off</button></a></p>");
            } else {
              client.println("<p><a href=\"/autoPilot/on\"><button class=\"button button4\">On</button></a></p>");
            }
            client.println("</body></html>");

            client.println("<h5>------------------------------------</h5>");
            client.println("<h5>By Jeroen & Jesse</h5>");

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    client.println("");
  }
}

/*
 * Algorithm tools
 */
boolean timer() {
  long startTime = millis();
  while (millis() - startTime < timerDuration) {
    return false;
  }
  return true;
}
 
boolean statusUltrasoon() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  echoTime = pulseInLong(echoPin, HIGH);
  distance = float(echoTime) / 2 * (soundSpeed / 10000.0);
  if (distance == 0) {
    Serial.println("[Warning] Ultrasoon may not be connected properly!");
    toDefaultMode();
    return false;
  } else if (distance < 15) {
    return true;
    driveBackwards();
  } else {
    return false;
  }
}

void toDefaultMode() {
  Serial.println("[INFO] Setting RescueBot in default state");
  buttonBackwardsState = "Execute";
  buttonForwardState = "Execute";
  buttonLeftState = "Execute";
  buttonRightState = "Execute";
  motorOneIO1State = "off";
  motorOneIO2State = "off";
  motorTwoIO1State = "off";
  motorTwoIO2State = "off";
  digitalWrite(motorOneIO2, LOW);
  digitalWrite(motorTwoIO2, LOW);
  digitalWrite(motorTwoIO1, LOW);
  digitalWrite(motorOneIO1, LOW);
}

void turnRight() {
  Serial.println("[INFO] Turning right");
  digitalWrite(motorOneIO2, LOW);
  digitalWrite(motorTwoIO1, LOW);
  digitalWrite(motorOneIO1, HIGH);
  digitalWrite(motorTwoIO2, HIGH);
}

void turnLeft() {
  Serial.println("[INFO] Turning left");
  digitalWrite(motorOneIO1, LOW);
  digitalWrite(motorTwoIO2, LOW);
  digitalWrite(motorTwoIO1, HIGH);
  digitalWrite(motorOneIO2, HIGH);
}

void driveBackwards() {
  Serial.println("[INFO] Driving backwards");
  digitalWrite(motorOneIO2, LOW);
  digitalWrite(motorTwoIO2, LOW);
  digitalWrite(motorTwoIO1, HIGH);
  digitalWrite(motorOneIO1, HIGH);
}

/*
 * AutoPilot algorithm
 */
void autoPilotDriving() {
  Serial.println("[INFO] RescueBot is on autopilot. Checking all sensors now.");
  long startTime = millis();
  boolean backToDefault = false;
  if (statusUltrasoon() == true){
    Serial.println("[INFO] Autopilot: Ultrasoon detected an object closer than 15 cm away");
    while (statusUltrasoon() == true) {
      driveBackwards();
      backToDefault = true;
    }
    if (backToDefault == true) {
      while (millis() - startTime > 500) {
        turnRight();
      }
      toDefaultMode();
      backToDefault = false;
    }
  }
}

/*
 * Driving Assistant algorithm
 */
void manualDrivingAssistant() {
  Serial.println("[INFO] You are driving manual with Driving Assistant");
  boolean backToDefault = false;
  if (statusUltrasoon() == true) {
    Serial.println("[INFO] Manual Driving Assistant took over control temporary. RescueBot stopped. Reason: about to hit an object.");
    while (statusUltrasoon() == true) {
      driveBackwards();
      backToDefault = true;
    }
    if (backToDefault == true) {
      timerDuration = 500;
      while (!timer()) {
        turnLeft();
      }
      toDefaultMode();
      backToDefault = false;
    }
  }
}