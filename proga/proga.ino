#include <Bounce2.h>
#include <OneWire.h>
#include "./LiquidCrystal.h"
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>

// Пины для подключенных устройств
#define PULSE_PIN 17
#define RELAY1_PIN 19
#define RELAY2_PIN 18
#define TERM_PIN 16
#define PRESS_PIN 15

// Настройка LCD дисплея
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define BUTTONS_LCD_PIN 14

// Настройка датчика температуры
OneWire oneWire(TERM_PIN);
DallasTemperature sensors(&oneWire);

DeviceAddress MotorAddress = { 0x28, 0xEA, 0xA1, 0x87, 0x00, 0x00, 0x00, 0x59 };
DeviceAddress SystemAddress = { 0x28, 0xFF, 0x18, 0xE8, 0x84, 0x16, 0x05, 0xDA };

// Настройки Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 137, 137);  //10, 6, 130, 66//192, 168, 137, 137
EthernetServer server(80);

bool _trgt1 = 0;
bool _trgt1I = 0;
bool _trgt2 = 0;
bool _trgt2I = 0;

// Настройка debounce для счетчика воды
Bounce debouncer = Bounce();
int water_counter = 0;
bool last_PULSE_PIN_State = false;

void setup() {
  // Настройка пинов
  pinMode(TERM_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(PULSE_PIN, INPUT_PULLUP);

  digitalWrite(RELAY1_PIN, _trgt1);
  digitalWrite(RELAY2_PIN, _trgt2);

  // Инициализация debounce для счетчика воды
  debouncer.attach(PULSE_PIN);
  debouncer.interval(50);

  // Инициализация Serial и LCD
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Teplo 1.0");
  lcd.setCursor(0, 1);
  lcd.print("--------");
  delay(900);
  lcd.clear();

  // Инициализация Ethernet
  Ethernet.begin(mac, ip);
  server.begin();
}

// Функция для считывания температуры
int readTemperature(DeviceAddress addr) {
  sensors.requestTemperaturesByAddress(addr);
  return sensors.getTempC(addr);
}

// Функция для считывания нажатий кнопок
int button_pressed() {
  int x = analogRead(BUTTONS_LCD_PIN);
  if (x < 100) return 1;
  else if (x < 200) return 2;
  else if (x < 350) return 3;
  else if (x < 600) return 4;
  else if (x < 800) return 5;
  return 0;
}

// Функция для управления реле
void TR1(int button) {
  bool _tmp1 = 0;
  if (button == 4) _tmp1 = 1;
  else _tmp1 = 0;
  if (_tmp1 && !_trgt1I) _trgt1 = !_trgt1;
  _trgt1I = _tmp1;
  lcd.setCursor(9, 1);
  lcd.print("H=");
  if (_trgt1) {
    lcd.print('I');
    digitalWrite(RELAY2_PIN, LOW);
  } else {
    lcd.print(' ');
    digitalWrite(RELAY2_PIN, HIGH);
  }
}

void TR2(int button) {
  bool _tmp2 = 0;
  if (button == 5) _tmp2 = 1;
  else _tmp2 = 0;
  if (_tmp2 && !_trgt2I) _trgt2 = !_trgt2;
  _trgt2I = _tmp2;
  lcd.setCursor(13, 1);
  lcd.print("M=");
  if (_trgt2) {
    lcd.print('I');
    digitalWrite(RELAY1_PIN, LOW);
  } else {
    lcd.print(' ');
    digitalWrite(RELAY1_PIN, HIGH);
  }
  //tt != tt;
  //Serial.print("System temp tt: " + String(tt));
}

// Функция для считывания расхода воды
int water() {
  debouncer.update();
  int value = debouncer.read();
  if (value == LOW && !last_PULSE_PIN_State) {
    water_counter++;
    Serial.println("Water consumption: " + String(water_counter * 10) + " l.");
    last_PULSE_PIN_State = true;
  } else if (value == HIGH) {
    last_PULSE_PIN_State = false;
  }
  return water_counter * 10;
}

void loop() {
  // Считывание данных
  int button = button_pressed();
  float temp = readTemperature(SystemAddress);
  float temp2 = readTemperature(MotorAddress);
  float Pres = analogRead(PRESS_PIN) / 1.023;
  int V1 = water();

  // Управление реле
  TR1(button);
  TR2(button);

  // Вывод данных в Serial
  Serial.print("System temp C: ");
  Serial.println((temp));
  Serial.print("Motor temp C: ");
  Serial.println(temp2);
  Serial.print("System P: ");
  Serial.println(Pres);
  Serial.print("Motor Litr: ");
  Serial.println(V1);

  // Обновление дисплея
  lcd.setCursor(0, 0);
  lcd.print("T=");
  lcd.print(temp2);
  lcd.setCursor(6, 0);
  lcd.print("C");

  lcd.setCursor(8, 0);
  lcd.print("P=");
  lcd.print(Pres);
  lcd.setCursor(13, 0);
  lcd.print("kPa");

  lcd.setCursor(0, 1);
  lcd.print("V=");
  lcd.print(V1);
  lcd.setCursor(6, 1);
  lcd.print("L");

  // Обработка подключений клиентов к серверу
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 1");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<title>Teplo 1.0 Data</title>");
          client.println("<style>");
          client.println("body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }");
          client.println("h1 { color: #333; }");
          client.println("p { background: #fff; padding: 10px; border: 1px solid #ddd; border-radius: 5px; margin-bottom: 10px; }");
          client.println("</style>");
          // output the value of each analog input pin
          float t1 = temp;
          float t2 = temp2;
          float Pressure = Pres;
          float Litr = V1;
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>Teplo 1.0 Data</h1>");
          client.print("<p>System temp C: ");
          client.print(t1);
          client.println("</p>");
          client.print("<p>Motor temp C: ");
          client.print(t2);
          client.println("</p>");
          client.print("<p>System P: ");
          client.print(Pres);
          client.println("</p>");
          client.print("<p>Motor Litr: ");
          client.print(V1);
          client.println("</p>");
          client.println("</body>");
          client.println("</html>");
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
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

  delay(500);  // Задержка перед следующим циклом
}