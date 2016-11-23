// ESP8266 plus WEMOS SHT30-D Sesnor with a Temperature and Humidity Web Server
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <WEMOS_SHT3X.h>

SHT3X sht30(0x45);        // SHT30 object to enable readings
String version = "1.0";   // Version of this program

WiFiServer server(80);    // Start server on port 80 (default for a web-browser

void setup() {
  Serial.begin(115200);
  //WiFiManager intialisation. Once completed there is no need to repeat the process on the current board
  WiFiManager wifiManager;
  // New OOB ESP8266 has no Wi-Fi credentials so will connect and not need the next command to be uncommented and compiled in, a used one with incorrect credentials will
  // Then restart the ESP8266 and connect your PC to the wireless access point called 'ESP8266_AP' or whatever you call it below
  // wifiManager.resetSettings(); // Command to be included if needed, then connect to http://192.168.4.1/ and follow instructions to make the WiFi connection
  // Set a timeout until configuration is turned off, useful to retry or go to sleep in n-seconds
  wifiManager.setTimeout(180);
  //fetches ssid and password and tries to connect, if connections succeeds it starts an access point with the name called "ESP8266_AP" and waits in a blocking loop for configuration
  if(!wifiManager.autoConnect("ESP8266_AP")) {
    Serial.println("failed to connect and timeout occurred");
    delay(3000);
    ESP.reset(); //reset and try again
    delay(5000);
  }
  // At this stage the WiFi manager will have successfully connected to a network, or if not will try again in 180-seconds
  //----------------------------------------------------------------------
  Serial.println("WiFi connected..");
  server.begin(); Serial.println("Webserver started..."); // Start the webserver
  Serial.print("Use this URL to connect: http://");// Print the IP address
  Serial.print(WiFi.localIP());Serial.println("/");
}
 
void loop() {
  String webpage = "";  
  sht30.get(); // Provides temp = sht30.ctemp or sht30.ftemp and sht30.humidity
  // Dew point and Heat Index are derived values from temperature and humidity
  float dew_point = 243.04*(log(sht30.humidity/100)+((17.625*sht30.fTemp)/(243.04+sht30.fTemp)))/(17.625-log(sht30.humidity/100)-((17.625*sht30.fTemp)/(243.04+sht30.fTemp)));
  float RHx       = sht30.humidity;
  float T         = sht30.fTemp;
  float heat_index;
  heat_index =(-42.379+(2.04901523*T)+(10.14333127*RHx)-(0.22475541*T*RHx)-(0.00683783*T*T)-(0.05481717*RHx*RHx)+(0.00122874*T*T*RHx)+(0.00085282*T*RHx*RHx)-(0.00000199*T*T*RHx*RHx)-32)*5/9;
  if ((sht30.cTemp <= 26.66) || (sht30.humidity <= 40)) heat_index = sht30.cTemp;
  WiFiClient client = server.available();// Now check if a client has connected
  if (client) { // has connected 
    boolean currentLineIsBlank = true; //an http request ends with a blank line
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if the end of a line (received a newline character) and the line is blank, then the http request has ended, so send a reply
        if (c == '\n' && currentLineIsBlank) {
          // now send a correctly formatted and standard http response
          webpage += "<!DOCTYPE html><html><head>";
          webpage += "<meta http-equiv=\"refresh\" content=\"10\">";
          webpage += "<meta content=\"text/html;charset=utf-8\">";
          webpage += "<title>ESP8266 Sensor Readings</title>";
          webpage += "<style>";
          webpage += "#header  {background-color:#6A6AE2; font-family:tahoma; width:1280px; padding:10px; color:white; text-align:center; }";
          webpage += "#section {background-color:#E6E6FA; font-family:tahoma; width:1280px; padding:10px; color:blue;  font-size:22px; text-align:center;}";
          webpage += "#footer  {background-color:#6A6AE2; font-family:tahoma; width:1280px; padding:10px; color:white; font-size:14px; clear:both; text-align:center;}";
          webpage += "</style></head><body>";
          webpage += "<div id=\"header\"><h1>ESP8266 Sensor Readings "+version+"</h1></div>";
          webpage += "<div id=\"section\"><h2>SHT30-D Temperature and Humidity Readings</h2>"; 
          webpage += "Temperature [" + String(sht30.cTemp,1)+ "&deg;C]" + "&nbsp&nbsp&nbsp";
          webpage += "Humidity ["    + String(sht30.humidity,1)+"%]"  + "&nbsp&nbsp&nbsp";
          webpage += "Dew Point ["   + String(dew_point,1)+"&deg;C]"  + "&nbsp&nbsp&nbsp";
          webpage += "Heat Index ["  + String(heat_index,1)+"&deg;C]";
          webpage += "</div></body>"; 
          webpage += "<div id=\"footer\">Copyright &copy; D L Bird 2016</div>"; 
          client.println(webpage);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    client.stop(); // close the connection:
  }
}

