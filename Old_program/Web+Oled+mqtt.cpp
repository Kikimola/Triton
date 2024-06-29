#include "Arduino.h"
#include "EspMQTTClient.h" /* https://github.com/plapointe6/EspMQTTClient */
                           /* https://github.com/knolleary/pubsubclient */
#define PUB_DELAY (5 * 1000) /* 5 seconds */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Import required libraries
#ifdef ESP32
    #include <WiFi.h>
    #include <ESPAsyncWebServer.h>
#else
    #include <Arduino.h>
    #include <ESP8266WiFi.h>
    #include <Hash.h>
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
#endif
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 17
#define ONE_WIRE_BUS 17

EspMQTTClient client(
    "TP-LINK_DA82",
    "06135782",
    "dev.rightech.io",
    "ggggg"
);
////////////////////////////////
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String temperatureF = "";
String temperatureC = "";

// Timer variables
unsigned long lastTime = 0; 
unsigned long timerDelay = 50;

// Replace with your network credentials
const char* ssid = "TP-LINK_DA82";
const char* password = "06135782";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC() {
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures(); 
    float tempC = sensors.getTempCByIndex(0);
    if (tempC == -127.00) {
        Serial.println("Failed to read from DS18B20 sensor");
        return "--";
    } 
    else {
        Serial.print("Temperature Celsius: ");
        Serial.println(tempC); 
    }
    return String(tempC);
}

String readDSTemperatureF() {
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures(); 
    float tempF = sensors.getTempFByIndex(0);
    if (int(tempF) == -196){
        Serial.println("Failed to read from DS18B20 sensor");
        return "--";
    } else {
        Serial.print("Temperature Fahrenheit: ");
        Serial.println(tempF);
    }
    return String(tempF);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
        html
        {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }
        h2 { font-size: 3.0rem; }
        p { font-size: 3.0rem; }
        .units { font-size: 1.2rem; }
        .ds-labels
        {
            font-size: 1.5rem;
            vertical-align:middle;
            padding-bottom: 15px;
        }
    </style>
</head>
<body>
    <h2>ESP DS18B20 Server</h2>
    <p>
        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
        <span class="ds-labels">Temperature Celsius</span> 
        <span id="temperaturec">%TEMPERATUREC%</span>
        <sup class="units">&deg;C</sup>
    </p>
    <p>
        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
        <span class="ds-labels">Temperature Fahrenheit</span>
        <span id="temperaturef">%TEMPERATUREF%</span>
        <sup class="units">&deg;F</sup>
    </p>
</body>
<script>
    setInterval(function()
    {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() 
        {
            if (this.readyState == 4 && this.status == 200) 
            {
                document.getElementById("temperaturec").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/temperaturec", true);
        xhttp.send();
    }, 10000);
    setInterval(function()
    {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() 
        {
            if (this.readyState == 4 && this.status == 200)
            {
                document.getElementById("temperaturef").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/temperaturef", true);
        xhttp.send();
    }, 10000);
</script>
</html>)rawliteral";

// Replaces placeholder with DS18B20 values
String processor(const String& var)
{
    //Serial.println(var);
    if(var == "TEMPERATUREC")
    {
        return temperatureC;
    }
    else if(var == "TEMPERATUREF")
    {
        return temperatureF;
    }
    return String();
}
/////////////////////////////



void setup() 
{
    Serial.begin(115200); 
    Serial.println();

    // Start up the DS18B20 library
    sensors.begin();

    temperatureC = readDSTemperatureC();
    temperatureF = readDSTemperatureF();

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", temperatureC.c_str());
    });
    server.on("/temperaturef", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", temperatureF.c_str());
    });

    // Start server
    server.begin();


    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {   // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
        delay(2000);
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(1, 1);
        // Display static text
        display.println(" ");
        display.display();  
    }
}


void onConnectionEstablished() {
        client.subscribe("base/relay/led1", [] (const String &payload)
        {
            Serial.println(payload);
        });
    }

long last = 0;


void publishTemperature()
{
    long now = millis();
    if (client.isConnected() && (now - last > PUB_DELAY))
    {
        client.publish("base/state/temperature", String(temperatureC));
        client.publish("base/state/humidity", String(random(40, 90)));
        last = now;
    }
}

void loop()
{
    client.loop();
    publishTemperature();
    if ((millis() - lastTime) > timerDelay)
    {
        temperatureC = readDSTemperatureC();
        temperatureF = readDSTemperatureF();
        lastTime = millis();
    }
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    delay(1000);
    display.clearDisplay();
 
    display.setTextSize(2);
    //display.setTextColor(WHITE);
    display.setCursor(0, 20);
    // Display static text
    display.println("TEMP:");
    display.setTextSize(2);
    display.setCursor(0, 40);
    display.print(temperatureC);
    display.setTextSize(2);
    display.print(" C");
    display.display();
}
