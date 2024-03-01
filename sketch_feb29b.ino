// Load Wi-Fi library
#include <WiFi.h>
const int trigPin = 5;
const int echoPin = 18;
// Replace with your network credentials
const char* ssid = "MARLUCE_2.4G";
const char* password = "*jose091";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
long duration;
float distanceCm;
float distanceInch;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); 

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
// Define minimum and maximum distances that the sensor can measure (in cm)
const float minDistance = 10.0;  // Example: sensor starts at 10 cm from the top of the water tank
const float maxDistance = 100.0; // Example: sensor ends at 100 cm from the top of the water tank
void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Calculate water level percentage
  float waterLevelPercent = 100.0 - ((distanceCm - minDistance) / (maxDistance - minDistance)) * 100.0;
  // Ensure the water level percentage stays within 0 to 100
  waterLevelPercent = constrain(waterLevelPercent, 0, 100);

  // Prints the water level percentage in the Serial Monitor
  Serial.print("Water Level (%): ");
  Serial.println(waterLevelPercent);
  Serial.println(WiFi.localIP());
  
  delay(1000);
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // Include water level information and battery animation in the HTML response
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons and battery animation

            client.println("<style>html { background-color: #FFFFFF; font-family: Arial, sans-serif; display: flex; justify-content: center; align-items: center; height: 100%; margin: 0; color: #FFFFFF; }");
            client.println(".container { text-align: center; position: relative; }");
            client.println(".background-rect { position: absolute; top: 0; left: 50; width: 100%; height: 50vh; background-color: #1c1c1c; border-radius: 8px;}");
            client.println(".battery { position: relative; width: 100px; height: 70px; background-color: #ddd; border: 1px solid #777; border-radius: 4px; margin-top: 20px; display: inline-block;}");
            client.println(".level { position: absolute; top: 0; left: 0; bottom: 0; width: 0%; background-color: #1DE14C; border-radius: 4px; transition: width 0.5s ease;}");
            client.println(".percentage { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); color: black; font-weight: bold;}");
            client.println(".par { position: relative; color: #FFFFFF; }");
            client.println(".title { position: relative; color: #0BEBB9; }");

            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body><div class=\"container\">");
            
            // Background rectangle
            client.println("<div class=\"background-rect\"></div>");
            
            // Heading
            client.println("<h1 class=\"title\">Aquamind - <br> Monitoramento <br> Hidrico <br> </h1>");
            
            // Display water level percentage
            client.print("<p class=\"par\">Nivel aquifero: ");
            client.print(waterLevelPercent);
            client.println("%</p>");

            // Display battery animation
            client.print("<div class=\"battery\"><div class=\"level\" style=\"width: ");
            client.print(waterLevelPercent);
            client.println("%\"></div><div class=\"percentage\">");
            client.print(waterLevelPercent);
            client.println("%</div></div>");

            client.println("</div></body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  
  }
}
