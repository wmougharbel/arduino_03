// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <FastLED.h>
#define NUM_LEDS 64
#define DATA_PIN 5
CRGB leds[NUM_LEDS];

unsigned long previousMillis = 0;           // Stores last time LEDs were updated
int count = 0;                              // Stores count for incrementing up to the NUM_LEDs
int loops = 0;

// Replace with your network credentials
const char* ssid     = "42Wolfsburg_FabLab";
const char* password = "0nly5ky15theL1m17";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputState[64];
String animationState = "off";

// Assign output variables to GPIO pins
const int output5 = 5;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 25000;

void setup() {
  Serial.begin(115200);
  int i = 0;
  while (i < 64)
    outputState[i++] = "off";
  pinMode(output5, OUTPUT);
  digitalWrite(output5, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  FastLED.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
  FastLED.setBrightness(50);
  
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
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
            

            // turn animation on and off
            if (header.indexOf("GET /animate/on") >= 0) {
                Serial.println("animate: on");
                animationState = "on";
                } else if (header.indexOf("GET /animate/off") >= 0) {
                  Serial.println("animate: off");
                  animationState = "off";
                  }

            // turns the LEDs on and off
            if (animationState == "off")
            {
              for (int i = 0; i < 64; i++)
              {
                  if (header.indexOf("GET /" + String(i) + "/on") >= 0) {
                  Serial.println("GPIO" + String(i) + "on");
                  outputState[i] = "on";
                  } else if (header.indexOf("GET /" + String(i) + "/off") >= 0) {
                    Serial.println("GPIO" + String(i) + "off");
                    outputState[i] = "off";
                    }
              }
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html lang=\"en\">");
            client.println("<head>");
            client.println("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("  <link rel=\"icon\" href=\"data:,\">");
            client.println("  <style>");
            client.println("    html { font-family: Helvetica; margin: 10px auto; text-align: center;}");
            client.println("    .button-container { width: 100%; display: flex; flex-wrap: wrap; justify-content: space-around;}");
            client.println("    .button-row { display: flex; justify-content: space-around; width: 100%; margin: 15px}");
            client.println("    .button { display: inline-block; background-color: #195B6A; border: none; color: white;");
            client.println("      padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("    .button2 { display: inline-block; background-color: #77878A;}");
            client.println("  </style>");
            client.println("</head>");

            client.println("<body>");
            client.println("  <h1>THE MACHINE THAT GOES PING</h1>");

            // Wrap the buttons in a container
            client.println("  <div class=\"button-container\">");
            for (int i = 0; i < 64; i++) {
              if (i % 5 == 0) {
                // Start a new row after every 5 buttons
                client.println("    <div class=\"button-row\">");
              }
              //client.println("      <div>GPIO" + String(i) + "- State " + outputState[i] + "</div>");
              // If the output5State is off, it displays the ON button       
              if (outputState[i] == "off") {
                client.println("      <div><a href=\"/" + String(i) + "/on\"><button class=\"button\">" + String(i) + " ON</button></a></div>");
              } else {
                client.println("      <div><a href=\"/" + String(i) + "/off\"><button class=\"button\">" + String(i) + " OFF</button></a></div>");
              }
              if ((i + 1) % 5 == 0 || i == 63) {
                // Close the row after every 5 buttons or at the end
                client.println("    </div>");
              }
            }
            client.println("  </div>");

            //place animation button
            if (animationState == "off") {
                client.println("      <div><a href=\"/animate/on\"><button class=\"button\">Animation: ON</button></a></div>");
              } else {
                client.println("      <div><a href=\"/animate/off\"><button class=\"button\">Animation: OFF</button></a></div>");
              }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();

            // change color of LEDs according to status
            if (animationState == "off")
            {
              for (int i = 0; i < 64; i++)
              {
                if (outputState[i] == "on")
                  leds[i]=CRGB::Blue;
                else if (outputState[i] == "off")
                  leds[i]=CRGB::Black;
                FastLED.show();
              }
            }
            else
            {
              while (loops < 640)
              {
                shootingStarAnimation(255, 255, 255, random(10, 60), random(5, 40), random(2000, 8000), 1);
                loops++;
              }
              loops = 0;
              for (int i = 0; i < 64; i++)
              {
                leds[i]=CRGB::Black;
                FastLED.show();
              }
            }
            
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

void shootingStarAnimation(int red, int green, int blue, int tail_length, int delay_duration, int interval, int direction){
  unsigned long currentMillis = millis();   // Get the time
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;         // Save the last time the LEDs were updated
    count = 0;                              // Reset the count to 0 after each interval
  }
  if (direction == -1) {        // Reverse direction option for LEDs
    if (count < NUM_LEDS) {
      leds[NUM_LEDS - (count % (NUM_LEDS+1))].setRGB(red, green, blue);    // Set LEDs with the color value
      count++;
    }
  }
  else {
    if (count < NUM_LEDS) {     // Forward direction option for LEDs
      leds[count % (NUM_LEDS+1)].setRGB(red, green, blue);    // Set LEDs with the color value
      count++;
    }
  }
  fadeToBlackBy(leds, NUM_LEDS, tail_length);                 // Fade the tail LEDs to black
  FastLED.show();
  delay(delay_duration);                                      // Delay to set the speed of the animation
}
