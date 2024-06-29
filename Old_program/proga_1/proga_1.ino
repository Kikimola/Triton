#include <Bounce2.h>
#include <OneWire.h>
#include <LiquidCrystal.h>                // Подключяем библиотеку
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );    // Указываем порты


OneWire ds(11);                           // Подключение датчика

bool tt=0;
bool t_old=0;

bool _trgt1 = 0;
bool _trgt1I = 0;
bool _trgt2 = 0;
bool _trgt2I = 0;

int PULSE_PIN = 3;
int water_counter = 0; 
bool last_PULSE_PIN_State = false;

Bounce debouncer = Bounce();

 
void setup()
{
    pinMode(11, INPUT); //проверить пин
    pinMode(13, OUTPUT); //проверить пин
    pinMode(12, OUTPUT); //проверить пин
    pinMode(PULSE_PIN, INPUT_PULLUP); //проверить пин??
    digitalWrite(12, 1);
    digitalWrite(13, 1);
    debouncer.attach(PULSE_PIN);
    debouncer.interval(50); // Ignore interval
  

    lcd.begin(16, 2);            // Инициализируем LCD 16x2  
    lcd.clear();
    lcd.setCursor(0,0);          // Установить курсор на первую строку 
    lcd.print("Teplo 1.0");      // Вывести текст
    lcd.setCursor(0,1);          // Установить курсор на вторую строку
    lcd.print("--------");       // Вывести текст
    Serial.begin(9600);          // Включаем последовательный порт
    delay(1000);                 // Проверяем включился ли он
    lcd.clear();
    delay(1000);
}



void loop()
{ 
    int button = button_pressed();  //
    float temp = DS18_read();       //
    float Pres = analogRead(1);     // проверить пин. Сделать переменную
    int V1 = water();               //
    TR1(button);                    //
    TR2(button);                    //


    lcd.setCursor(0,0);             // Установить курсор на первую строку 
    lcd.print("T=");                // Вывести текст
    lcd.setCursor(2,0);             // Установить курсор на вторую строку
    lcd.print(String(temp)+" C"); 

    lcd.setCursor(10,0);            // Установить курсор на первую строку  
    lcd.print("P=");                // Вывести текст
    lcd.setCursor(12,0);            // Установить курсор на вторую строку
    lcd.print(String(Pres)+" Pa"); 

    lcd.setCursor(0,1);             // Установить курсор на первую строку 
    lcd.print("V=");                // Вывести текст
    lcd.setCursor(3,1);             // Установить курсор на вторую строку
    lcd.print(String(V1)+" L");
    delay(100);

    // Задаем номер порта с которого производим считывание
    //lcd.setCursor(0,1);
    // Установить курсор на вторую строку
    //lcd.print (String(button));

}

int button_pressed()
{
    int x,y=0;                      // Создаем переменную x
    x = int(analogRead(0));        //проверить пин

    switch (x / 100)
    {
        case 0: y = 1;      // Если x меньше 100 перейти на следующею строк  //lcd.print ("Right "); 
        case 1: y = 2;      // Если х меньше 200 перейти на следующию строку // lcd.print ("Up  ")
        case 2:             // Если х меньше 400 перейти на следующию строку
        case 3: y = 3;      //lcd.print ("Down ");
        case 4:             // Если х меньше 600 перейти на следующию строку
        case 5: y = 4;      // lcd.print ("Left  ");
        case 6:             // Если х меньше 800 перейти на следующию строку
        case 7: y = 5;      //lcd.print ("Select");         // Вывести текст
    }
    delay(50);
    return y;
    }

void TR1(int button)
{
    bool  _tmp1 = 0;
    if (button == 5)
    {
        _tmp1 =  1;
    } 
    else
    {
        _tmp1 = 0;
    }
    if (_tmp1 && !_trgt1I) _trgt1 = ! _trgt1;
    _trgt1I = _tmp1;
    lcd.setCursor(12,1);  
    if (_trgt1)
    {
        lcd.print ('H');
        digitalWrite(13, 0);
    } 
    else
    {
        lcd.print (' ');
        digitalWrite(13, 1);
    }
}

void TR2(int button)
{
    bool  _tmp=0;
    if (button == 4)
    {
        _tmp = 1;
    }
    else
    {
        _tmp =  0;
    }
    if (_tmp && !_trgt2I) _trgt2 = ! _trgt2;

    _trgt2I = _tmp;
    lcd.setCursor(14,1);  
    
    if (_trgt2)
    {
        lcd.print ('M');
        digitalWrite(12, 0);
    }
    else
    {
        lcd.print (' ');
        digitalWrite(12, 1);
    }
}

float DS18_read()
{
    // Определяем температуру от датчика DS18b20
    byte data[2]; // Место для значения температуры

    ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
    ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
    ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем
                    // - датчик его положит во внутреннюю память

    // delay(1000); // Микросхема измеряет температуру, а мы ждем.  

    ds.reset(); // Теперь готовимся получить значение измеренной температуры
    ds.write(0xCC); 
    ds.write(0xBE); // Просим передать нам значение регистров со значением температуры. // спросить про адреса

    // Получаем и считываем ответ
    data[0] = ds.read(); // Читаем младший байт значения температуры
    data[1] = ds.read(); // А теперь старший

    // Формируем итоговое значение: 
    //    - сперва "склеиваем" значение, 
    //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
    float temperature =  ((data[1] << 8) | data[0]) * 0.0625;

    // Выводим полученное значение температуры в монитор порта
    Serial.println(temperature);  
    return temperature;
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