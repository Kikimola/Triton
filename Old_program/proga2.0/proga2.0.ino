#include <Bounce2.h>
#include <OneWire.h>
#include "./LiquidCrystal.h"
#include <DallasTemperature.h>

#define PULSE_PIN   3
#define RELAY1_PIN  13
#define RELRY2_PIN  12
#define TERM_PIN    1
#define PRESS_PIN   15

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);    // Указываем порты. Плата дисплея
#define BUTTONS_LCD_PIN 14


OneWire oneWire(TERM_PIN);                  // Инициализация термодатчика
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1Address = {
  0x28, 0xEA, 0xA1, 0x87, 0x00, 0x00, 0x00, 0x59               // Специальный объект для хранения адреса устройства
};


bool tt = false;
bool t_old = false;

bool _trgt1 = false;
bool _trgt1I = false;

bool _trgt2 = false;
bool _trgt2I = false;

int water_counter = 0;
bool last_PULSE_PIN_State = false;

Bounce debouncer = Bounce();        // Счетчик литража

void setup()
{
    pinMode(TERM_PIN, INPUT);
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELRY2_PIN, OUTPUT);
    pinMode(PULSE_PIN, INPUT_PULLUP);

    digitalWrite(RELRY2_PIN, 1);
    digitalWrite(RELAY1_PIN, 1);

    debouncer.attach(PULSE_PIN);        // Инициализация счетчика воды
    debouncer.interval(50);             // Ignore interval

    Serial.begin(9600);                 // Включаем последовательный порт
    Serial.println("Канал открыт!\n");  // Проверяем подключение
    Serial.println("Канал открыт!\n");  // Адрес термометра
    printAddress(sensor1Address);

    lcd.begin(16, 2);                   // Инициализируем LCD 16x2  
    lcd.clear();
    lcd.setCursor(0,0);                 // Установить курсор на первую строку 
    lcd.print("Teplo 1.0");             // Вывести текст
    lcd.setCursor(0,1);                 // Установить курсор на вторую строку
    lcd.print("--------");              // Вывести текст
    delay(1000);                        // Проверяем включился ли он
    lcd.clear();
    delay(100);
}

void loop()
{ 
    int button = button_pressed();          // Событие нажатия кнопки
    int temp = sensors.getTempC(sensor1Address); // считывание температуры
//    float temp = DS18_read();               // считывание температуры
    float Pres = analogRead(PRESS_PIN);     // считывание давления
    int V1 = water();                       // считывание литража
    TR1(button);                            // Проверка нажатия кнопки справа
    TR2(button);                            // Проверка нажатия кнопки слева

    Serial.print("Temp C: ");
    Serial.println(temp);

    lcd.setCursor(0,0);                 // Установить курсор на первую строку 
    lcd.print("T=");                    // Вывести текст
    lcd.setCursor(2,0);                 // Установить курсор на вторую строку
    lcd.print(String(temp) + "C"); 

    lcd.setCursor(9,0);                 // Установить курсор на первую строку  
    lcd.print("P=");                    // Вывести текст
    lcd.setCursor(11,0);                // Установить курсор на вторую строку
    lcd.print(String(Pres/1000)+" Pa"); 

    lcd.setCursor(0,1);                 // Установить курсор на первую строку 
    lcd.print("V=");                    // Вывести текст
    lcd.setCursor(3,1);                 // Установить курсор на вторую строку
    lcd.print(String(V1)+" L");
    Serial.println("Цикл закончен.");
    delay(100);

    // Задаем номер порта с которого производим считывание
    //lcd.setCursor(0,1);
    // Установить курсор на вторую строку
    //lcd.print (String(button));
}

// Вспомогательная функция для отображения адреса датчика ds18b20
void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

int button_pressed() // спросить что делает функция
// {
//     int x, y = 0;          // Создаем переменную x
//     x = int(analogRead(BUTTONS_LCD_PIN));   //

//     if (x < 100)
//         y = 1;  // Если x меньше 100 перейти на следующею строк  //lcd.print ("Right "); 
//     else if (x < 200)
//         y = 2;  // Если х меньше 200 перейти на следующию строку // lcd.print ("Up  ")
//     else if (x < 400)
//         y = 3;   // Если х меньше 400 перейти на следующию строку
//     else if (x < 600)
//         y = 4;  // Если х меньше 600 перейти на следующию строку
//     else if (x < 800)    // Если х меньше 800 перейти на следующию строку
//         y = 5;  //lcd.print ("Select");         // Вывести текст
//     delay(50);
//     return y;
// }
{
    int x, y = 0;                                  // Создаем переменную x
    x = analogRead(BUTTONS_LCD_PIN); //проверить пин

    if (x < 100) {                          // Если x меньше 100 перейти на следующею строк
        y = 1; //lcd.print ("Right "); 
    }
    else if (x < 200) {           // Если х меньше 200 перейти на следующию строку
        y = 2;// lcd.print ("Up  ");   
    }
    else if (x < 400){                      // Если х меньше 400 перейти на следующию строку                
        y = 3; //lcd.print ("Down ");
    }
    else if (x < 600) {                      
        y = 4; // lcd.print ("Left  ");         
    }
    else if (x < 800) {                      // Если х меньше 800 перейти на следующию строку
        y = 5; //lcd.print ("Select");                   // Вывести текст
    }
    delay(50);
    return y;
}

void TR1(int button)
{
    bool  _tmp1 = false;
    if (button == 5)
    {
        _tmp1 = true;
    }
    if (_tmp1 && !_trgt1I) _trgt1 = ! _trgt1;
    _trgt1I = _tmp1;
    lcd.setCursor(12,1);  
    if (_trgt1)
    {
        lcd.print ('H');
        digitalWrite(RELAY1_PIN, 0);
    } 
    else
    {
        lcd.print (' ');
        digitalWrite(RELAY1_PIN, 1);
    }
}

void TR2(int button)
{
    bool  _tmp = false;
    if (button == 4)
    {
        _tmp = true;
    }
    if (_tmp && !_trgt2I) _trgt2 = ! _trgt2;
    
    _trgt2I = _tmp;
    lcd.setCursor(14,1);  
    
    if (_trgt2)
    {
        lcd.print ('M');
        digitalWrite(RELRY2_PIN, 0);
    }
    else
    {
        lcd.print (' ');
        digitalWrite(RELRY2_PIN, 1);
    }
}

int water()
{
    debouncer.update();
 
    // Read Reed Switch state
    int value = debouncer.read();
 
    // Now all processes are finished and we know exactly the state of the Reed Switch
    if ( value == LOW )
    {
        if (last_PULSE_PIN_State == false)
        {
            water_counter++;
            Serial.println("Water consumption: " + (String)(water_counter*10) + " l.");
        }
        last_PULSE_PIN_State = true;
    }
    else
    {
        last_PULSE_PIN_State = false;
    }
    int wc = water_counter*10;
    return wc;
}



// float DS18_read()
// {
//     // Определяем температуру от датчика DS18b20
//     byte data[2]; // Место для значений теомометра

//     DalTemp.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
//     DalTemp.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
//     DalTemp.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем
//                     // - датчик его положит во внутреннюю память

//     delay(800); // Микросхема измеряет температуру, а мы ждем.  

//     DalTemp.reset(); // Теперь готовимся получить значение измеренной температуры
//     DalTemp.write(0xCC); 
//     DalTemp.write(0xBE); // Просим передать нам значение регистров со значением температуры. // спросить про эти символы

//     // Получаем и считываем ответ
//     data[0] = DalTemp.read(); // Читаем младший байт значения температуры
//     data[1] = DalTemp.read(); // А теперь старший

//     // Формируем итоговое значение: 
//     //    - сперва "склеиваем" значение, 
//     //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
//     float temperature =  ((data[1] << 8) | data[0]) * 0.0625;

//     // Выводим полученное значение температуры в монитор порта
//     Serial.println(temperature);  
//     return temperature;
// }





/*
float readDSTemperatureC() {
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures(); 
    float tempC = sensors.getTempCByIndex(0);
    Serial.println(tempC);
    return tempC;
}*/

/*float DS18_read()
{
    // Определяем температуру от датчика DS18b20
    byte data[2]; // Место для значения температуры

    DalTemp.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
    DalTemp.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
    DalTemp.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем
                    // - датчик его положит во внутреннюю память

    // delay(1000); // Микросхема измеряет температуру, а мы ждем.  

    DalTemp.reset(); // Теперь готовимся получить значение измеренной температуры
    DalTemp.write(0xCC); 
    DalTemp.write(0xBE); // Просим передать нам значение регистров со значением температуры. // спросить про эти символы

    // Получаем и считываем ответ
    data[0] = DalTemp.read(); // Читаем младший байт значения температуры
    data[1] = DalTemp.read(); // А теперь старший

    // Формируем итоговое значение: 
    //    - сперва "склеиваем" значение, 
    //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
    float temperature =  ((data[1] << 8) | data[0]) * 0.0625;

    // Выводим полученное значение температуры в монитор порта
    Serial.println(temperature);  
    return temperature;
}*/
