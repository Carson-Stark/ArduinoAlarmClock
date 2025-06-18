//lcd battery powered alarm clock
//features:
//custom character 3 line numbers
//put arduino to sleep to save power
//wakes up every minute to update lcd and check alarms
//ability to set 8 different alarms and the days they are active
//ability to set hour and minute
//wake and light up the screan with button press
//enter set mode with button hold

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <LowPower.h>
#include <Wire.h>
#include <RTClibExtended.h>

//custom lcd characters
uint8_t smBlockB[8] = {0x0, 0x0, 0x0, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
uint8_t smBlockT[8] = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0, 0x0, 0x0};
uint8_t halfBlock[8] = {0x0, 0x0, 0x1f, 0x1f, 0x1f, 0x1f, 0x0, 0x0};
uint8_t TRCorner[8] = {0x18, 0x1c, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
uint8_t BLCorner[8] = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0xf, 0x7, 0x3};
uint8_t bellNOW[8] = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4, 0x0};
uint8_t bellNXT[8] = {0x4, 0xa, 0xa, 0xa, 0x1f, 0x0, 0x4, 0x0};

//pins
const byte interruptPinRTC = 2;
const byte interruptPinButton = 3;
const byte buzzerPin = 5;
const byte button1 = 11;
const byte button2 = 10;
const int photo = A0;
const int voltage = A1;
const byte readSwitch = 13;
const byte ledSwitch = 12;

const byte lightUpTime = 5;
const byte snoozeTime = 5;

RTC_DS3231 clock;
LiquidCrystal_I2C lcd(0x27, 20, 4);

//alarms
struct alarm
{
    bool days[7];
    byte hour;
    byte minute;
    byte am;

    alarm() {}
    alarm(int h, int m, int a)
    {
        hour = h;
        minute = m;
        am = a;
    }
};
typedef struct alarm Alarm;
Alarm alarms[8];

Alarm currentAlarm;

//standard global variable
bool lightScreen;
byte lightDisTime;
bool displayTime;
bool alarmFlag = false;
bool snoozing;
int snoozeCount = 0;
bool currentAlarmEnabled = true;
bool displayOff = false;
bool setMode;
const char days[8] = "SMTWTFS";

//variables for set mode
byte menu = 0;
byte subMenu = 0;
byte selected = 1;
int previous = 0;
bool on;

//variables for button hold
bool testingButton1;
bool testingButton2;
byte minReleaseTime;
byte minReleaseTime2;

//is the hour AM or PM ?
bool hourIsAM(int hour)
{
    if (hour < 12 || hour == 24)
        return true;

    return false;
}

//conver 24 hour format to 12
int convertHour(int hour)
{
    if (hour > 11 && hour != 12)
        return hour - 12;

    if (hour == 0)
        return 12;

    return hour;
}

//converts 12 hour format to 24
int hourTo24(int hour, bool am)
{
    if (am)
    {
        if (hour == 12)
            return 0;
        return hour;
    }
    else
    {
        if (hour == 12)
            return hour;
        return hour + 12;
    }
}

//gets the number of digits in a number (for printing the date)
int numOfDigits(int number)
{
    int length = 0;
    int temp = 1;
    while (temp <= number)
    {
        length++;
        temp *= 10;
    }
    return length;
}

//print at number spanning 3 lines on the rtc using custom characters
void printLgNum(int num, int column)
{
    lcd.setCursor(column, 0);
    switch (num)
    {
    case 0:
        lcd.write(255);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 1);
        lcd.write(255);
        lcd.print(" ");
        lcd.write(255);
        lcd.setCursor(column, 2);
        lcd.write(4);
        lcd.write(1);
        lcd.write(255);
        break;

    case 1:
        lcd.write(0);
        lcd.write(3);
        lcd.print(" ");
        lcd.setCursor(column, 1);
        lcd.print(" ");
        lcd.write(255);
        lcd.print(" ");
        lcd.setCursor(column, 2);
        lcd.write(1);
        lcd.write(255);
        lcd.write(1);
        break;

    case 2:
        lcd.write(0);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 1);
        lcd.write(255);
        lcd.write(0);
        lcd.write(0);
        lcd.setCursor(column, 2);
        lcd.write(255);
        lcd.write(1);
        lcd.write(1);
        break;

    case 3:
        lcd.write(0);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 1);
        lcd.write(2);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(column, 2);
        lcd.write(1);
        lcd.write(1);
        lcd.write(255);
        break;

    case 4:
        lcd.write(255);
        lcd.print(" ");
        lcd.write(255);
        lcd.setCursor(column, 1);
        lcd.write(255);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(column, 2);
        lcd.print(" ");
        lcd.print(" ");
        lcd.write(255);
        break;

    case 5:
        lcd.write(255);
        lcd.write(0);
        lcd.write(0);
        lcd.setCursor(column, 1);
        lcd.write(0);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 2);
        lcd.write(1);
        lcd.write(1);
        lcd.write(255);
        break;

    case 6:
        lcd.write(255);
        lcd.write(0);
        lcd.write(0);
        lcd.setCursor(column, 1);
        lcd.write(255);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 2);
        lcd.write(4);
        lcd.write(1);
        lcd.write(255);
        break;

    case 7:
        lcd.write(0);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(column, 1);
        lcd.print(" ");
        lcd.write(255);
        lcd.print(" ");
        lcd.setCursor(column, 2);
        lcd.write(255);
        lcd.print(" ");
        lcd.print(" ");
        break;

    case 8:
        lcd.write(255);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 1);
        lcd.write(255);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(column, 2);
        lcd.write(4);
        lcd.write(1);
        lcd.write(255);
        break;

    case 9:
        lcd.write(255);
        lcd.write(0);
        lcd.write(3);
        lcd.setCursor(column, 1);
        lcd.write(4);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(column, 2);
        lcd.print(" ");
        lcd.print(" ");
        lcd.write(255);
        break;
    }
}

//print a large colon using custom characters
void printLgColon(int column)
{
    lcd.setCursor(column, 0);
    lcd.write(1);
    lcd.setCursor(column, 2);
    lcd.write(0);
}

//erases a block of text by printing spaces
void eraseBlock(int column, int row, int height, int length)
{
    for (int h = row; h < row + height; h++)
    {
        lcd.setCursor(column, h);
        for (int l = column; l < column + length; l++)
        {
            lcd.print(" ");
        }
    }
}

//prints the hour (centers it based on the number of digits in the hour)
void printHour(int hour)
{
    if (hour < 10)
        printLgNum(hour % 10, 3);
    else
    {
        printLgNum(hour / 10, 1);
        printLgNum(hour % 10, 5);
    }
}

//prints the minute (centers it based on the number of digits in the hour)
void printMinute(int minute, int hour)
{
    if (hour < 10)
    {
        printLgNum(minute / 10, 9);
        printLgNum(minute % 10, 13);
    }
    else
    {
        printLgNum(minute / 10, 11);
        printLgNum(minute % 10, 15);
    }
}

//prints AM or PM (centers it based on the number of digits in the hour)
void printAM(int hour, bool am)
{
    if (hour < 10)
    {
        lcd.setCursor(17, 0);
        lcd.print(am ? "A" : "P");
        lcd.setCursor(17, 1);
    }
    else
    {
        lcd.setCursor(19, 0);
        lcd.print(am ? "A" : "P");
        lcd.setCursor(19, 1);
    }
    lcd.print("M");
}

//prints the colon (centers it based on the number of digits in the hour)
void printColon(int hour)
{
    if (hour < 10)
        printLgColon(7);
    else
        printLgColon(9);
}

//prints the time using previous functions (h:mm AM/PM)
void printTime(bool reset)
{
    int h = convertHour(clock.now().hour());
    int m = clock.now().minute();

    if (reset || ((h == 10 || h == 1) && m == 0))
    {
        eraseBlock(0, 0, 3, 19);
        printColon(h);
    }
    if (reset || m == 0)
        printHour(h);
    if (reset || ((h == 10 || h == 1) && m == 0) || (h == 12 && m == 0))
    {
        eraseBlock(19, 0, 2, 1);
        printAM(h, hourIsAM(clock.now().hour()));
    }

    printMinute(m, h);
}

bool timeEarlierThan(int h1, int m1, int h2, int m2)
{
    if (h1 < h2)
        return true;

    if (h1 == h2 && m1 < m2)
        return true;

    return false;
}

//prints the current alarm ([bell]h:mm AM/PM)
void printAlarm()
{
    eraseBlock(0, 3, 1, 8);
    lcd.setCursor(0, 3);

    if ((currentAlarm.days[clock.now().dayOfTheWeek() < 6 ? clock.now().dayOfTheWeek() + 1 : 0] || (currentAlarm.days[clock.now().dayOfTheWeek()] && timeEarlierThan(clock.now().hour(), clock.now().minute(), hourTo24(currentAlarm.hour, currentAlarm.am == 1 ? true : false), currentAlarm.minute))) && currentAlarm.am > 0)
    {
        if (!currentAlarmEnabled)
        {
            lcd.write(5);
            lcd.print("OFF");
            return;
        }

        if (currentAlarm.days[clock.now().dayOfTheWeek()] && timeEarlierThan(clock.now().hour(), clock.now().minute(), hourTo24(currentAlarm.hour, currentAlarm.am == 1 ? true : false), currentAlarm.minute))
            lcd.write(5);
        else
            lcd.write(6);

        lcd.print(currentAlarm.hour);
        lcd.print(":");
        if (currentAlarm.minute < 10)
            lcd.print(0);
        lcd.print(currentAlarm.minute);
        lcd.print(currentAlarm.am == 1 ? "AM" : "PM");
    }
    else
    {
        if (!currentAlarmEnabled)
        {
            lcd.write(6);
            lcd.print("OFF");
            return;
        }

        lcd.write(6);
        lcd.print("NONE");
    }
}

//prints the day on the screen (wd d/m/yy)
void printDay()
{
    eraseBlock(9, 3, 1, 11);

    int totalDigits = numOfDigits(clock.now().month()) + numOfDigits(clock.now().day());
    if (totalDigits == 4)
        lcd.setCursor(9, 3);
    else if (totalDigits == 3)
        lcd.setCursor(10, 3);
    else
        lcd.setCursor(11, 3);

    int day = clock.now().dayOfTheWeek();
    if (day == 1)
        lcd.print("MO");
    else if (day == 2)
        lcd.print("TU");
    else if (day == 3)
        lcd.print("WE");
    else if (day == 4)
        lcd.print("TH");
    else if (day == 5)
        lcd.print("FR");
    else if (day == 6)
        lcd.print("SA");
    else if (day == 0)
        lcd.print("SU");

    lcd.print(" ");
    lcd.print(clock.now().month());
    lcd.print("/");
    lcd.print(clock.now().day());
    lcd.print("/");
    lcd.print(clock.now().year() - 2000);
}

//used for setting the alarms : prints a certain alarm and the days its enabled
//#) h:mm AM/PM/OFF smtwtfs
void printAlarmSet(int row, int alarmIndex)
{
    eraseBlock(0, row, 1, 20);
    Alarm a = alarms[alarmIndex];

    //print number
    lcd.setCursor(0, row);
    lcd.print(alarmIndex + 1);
    lcd.print(") ");

    //print alarm
    lcd.print(a.hour);
    lcd.print(":");
    lcd.print(a.minute / 10);
    lcd.print(a.minute % 10);
    lcd.print(" ");
    if (a.am == 0)
        lcd.print("OFF");
    else if (a.am == 1)
        lcd.print("AM");
    else
        lcd.print("PM");

    //print days to enable on
    lcd.setCursor(13, row);
    for (int i = 0; i < 7; i++)
    {
        if (a.days[i])
            lcd.print(days[i]);
        else
            lcd.write(tolower(days[i]));
    }
}

//set the rtc, clear alarms, reactivate alarm for every minute
void setClock(DateTime datetime)
{
    clock.armAlarm(1, false);
    clock.clearAlarm(1);
    clock.alarmInterrupt(1, false);
    clock.armAlarm(2, false);
    clock.clearAlarm(2);
    clock.alarmInterrupt(2, false);

    clock.adjust(datetime);
    clock.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0);
    clock.alarmInterrupt(2, true);
}

//sets the current alarm to correct one : first searches current day then the next day
void findNextAlarm()
{
    int lowestH = 80;
    int lowestM = 60;
    bool foundNext;

    for (int i = 0; i < 8; i++)
    {
        if (alarms[i].am > 0)
        {
            int h = 100;
            if (alarms[i].days[clock.now().dayOfTheWeek()] && !timeEarlierThan(hourTo24(alarms[i].hour, alarms[i].am == 1 ? true : false), alarms[i].minute, clock.now().hour(), clock.now().minute()))
                h = hourTo24(alarms[i].hour, alarms[i].am == 1 ? true : false);
            else if (alarms[i].days[clock.now().dayOfTheWeek() < 6 ? clock.now().dayOfTheWeek() + 1 : 0])
                h = hourTo24(alarms[i].hour, alarms[i].am == 1 ? true : false) + 24;

            if (h < lowestH || (h == lowestH && alarms[i].minute < lowestM))
            {
                lowestH = h;
                lowestM = alarms[i].minute;
                currentAlarm = alarms[i];
                foundNext = true;
            }
        }
    }

    if (!foundNext)
        currentAlarm = alarms[0];
}

//attached to interrupt by button
void wakeUpButton()
{
}

//attached to interrupt by rtc
void wakeUpRTC()
{
    displayTime = true;
}

//used to check if a certain amount of time has passed since a preious time
//used for blinking text
bool passedInterval(int previous, int interval)
{
    if (previous > millis() % 10000)
        return (10000 - previous) + millis() % 10000 > interval;
    else
        return millis() % 10000 - previous > interval;
}

void setup()
{
    pinMode(interruptPinRTC, INPUT_PULLUP);
    pinMode(interruptPinButton, INPUT_PULLUP);
    pinMode(button1, INPUT_PULLUP);
    pinMode(button2, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    pinMode(photo, INPUT_PULLUP);
    //pinMode(voltage, INPUT);
    pinMode(readSwitch, OUTPUT);

    //signals successful start
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5000);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    Wire.begin();
    clock.begin();
    lcd.init();

    //custom characters for lcd
    lcd.createChar(0, smBlockT);
    lcd.createChar(1, smBlockB);
    lcd.createChar(2, halfBlock);
    lcd.createChar(3, TRCorner);
    lcd.createChar(4, BLCorner);
    lcd.createChar(5, bellNOW);
    lcd.createChar(6, bellNXT);
    lcd.home();

    //sets time to upload time
    clock.writeSqwPinMode(DS3231_OFF);
    //setClock(DateTime(__DATE__, __TIME__));

    for (int i = 0; i < 8; i++)
    {
        alarms[i] = Alarm(8, 0, 0);
    }
    currentAlarm = alarms[0];
    findNextAlarm(); //<-- sets the current alarm to correct one

    printTime(true);
    printAlarm();
    printDay();
}

//puts arduino to sleep and attaches correct interrupts
void sleep()
{
    attachInterrupt(0, wakeUpRTC, LOW);
    attachInterrupt(1, wakeUpButton, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    Serial.print("wake");
    detachInterrupt(0);
    detachInterrupt(1);
    clock.clearAlarm(2);
    if (!displayTime)
    {
        digitalWrite(readSwitch, HIGH);
        delay(10);

        if (displayOff)
        {
            displayOff = false;
            lcd.display();
        }

        if (analogRead(photo) > 900)
        {
            int sec = clock.now().second();
            lightScreen = true;
            lightDisTime = sec + lightUpTime < 60 ? sec + lightUpTime : sec + lightUpTime - 60;
            lcd.backlight();
        }

        if (analogRead(voltage) * (3.3 / 1023) < 0.25)
        {
            lcd.clear();
            lcd.setCursor(1, 4);
            lcd.print("LOW BATTERY:");
            lcd.setCursor(2, 5);
            lcd.print(analogRead(voltage) * (3.33 / 1023) * 2);
            lcd.print("V ");
            lcd.print(analogRead(voltage) * (3.33 / 1023) / 2.25 * 100);
            lcd.print("%");

            delay(3000);

            printTime(true);
            printAlarm();
            printDay();
        }

        delay(10);
        digitalWrite(readSwitch, LOW);
    }
}

//variables for loop()
bool buttonHeld;
bool buttonHeld2;
byte newHour;
byte newMinute;
bool am;
byte newMonth;
byte newDay;
byte newYear;
bool ready;
int lastInteraction;
bool tempDis;

void loop()
{
    //called every minute
    if (displayTime && !setMode)
    {
        Serial.print("minute passed");

        digitalWrite(readSwitch, HIGH);
        delay(10);

        if (analogRead(photo) > 900 && !displayOff)
        {
            lcd.noDisplay();
            displayOff = true;
        }
        else if (displayOff)
        {
            lcd.display();
            displayOff = false;
        }

        delay(10);
        digitalWrite(readSwitch, LOW);

        printTime(false);
        displayTime = false;

        //updates date at 12:00am
        if (clock.now().hour() == 0 && clock.now().minute() == 0)
        {
            printDay();
            printAlarm();
        }

        if (!lightScreen)
            delay(50);
    }

    //sound alarm
    if (currentAlarmEnabled && currentAlarm.days[clock.now().dayOfTheWeek()] && currentAlarm.am > 0 && convertHour(clock.now().hour()) == currentAlarm.hour && clock.now().minute() == currentAlarm.minute && (hourIsAM(clock.now().hour()) ? 1 : 2) == currentAlarm.am)
    {
        if (!alarmFlag && !tempDis)
        {
            alarmFlag = true;
            snoozing = false;
        }
    }
    else if (alarmFlag || tempDis)
    {
        snoozing = false;
        alarmFlag = false;
        tempDis = false;
        currentAlarmEnabled = true;
        findNextAlarm();
        printAlarm();
    }

    if (alarmFlag)
    {
        if (passedInterval(previous, 100))
        {
            if (on)
                digitalWrite(buzzerPin, HIGH);
            else
                analogWrite(buzzerPin, 0);
            on = !on;
            previous = millis() % 10000;
        }

        if (!snoozing && (digitalRead(button1) == LOW || digitalRead(button2) == LOW))
        {
            digitalWrite(buzzerPin, 0);
            snoozing = true;
            alarmFlag = false;
            snoozeCount++;

            if (currentAlarm.minute + 5 < 60)
                currentAlarm.minute += 5;
            else
            {
                if (currentAlarm.hour + 1 > 12)
                {
                    currentAlarm.hour = 1;
                    currentAlarm.am = currentAlarm.am == 1 ? 2 : 1;
                }
                else
                    currentAlarm.hour++;

                currentAlarm.minute = currentAlarm.minute + 5 - 60;
            }
        }
    }

    if (!setMode && !testingButton2 && digitalRead(button2) == 0)
    {
        testingButton2 = true;
        int sec = clock.now().second();
        minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
    }

    if (lightScreen)
    {
        if (clock.now().second() >= lightDisTime)
        {
            lightScreen = false;
            lcd.noBacklight();
        }
    }

    //start checking for hold if we press button one
    if (!setMode && !alarmFlag && !testingButton1 && digitalRead(button1) == 0)
    {
        testingButton1 = true;
        int sec = clock.now().second();
        minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
        minReleaseTime2 = sec + 4 < 60 ? sec + 4 : sec + 4 - 60;
    }
    else if (setMode)
    {
        //nothing selected
        if (menu == 0)
        {
            switch (selected)
            {
            //blink time
            case 1:
                if (passedInterval(previous, 500))
                {
                    if (on)
                        eraseBlock(0, 0, 3, 20);
                    else
                        printTime(true);
                    on = !on;
                    previous = millis() % 10000;
                }
                break;

            //blink alarm
            case 2:
                if (passedInterval(previous, 500))
                {
                    if (on)
                        eraseBlock(0, 3, 1, 8);
                    else
                        printAlarm();
                    on = !on;
                    previous = millis() % 10000;
                }
                break;

            //blink day
            case 3:
                if (passedInterval(previous, 500))
                {
                    if (on)
                        eraseBlock(8, 3, 1, 12);
                    else
                        printDay();
                    on = !on;
                    previous = millis() % 10000;
                }
                break;
            }

            //cycle what is selected
            if (digitalRead(button1) == LOW)
            {
                if (!testingButton1)
                {
                    //start checking for hold in case we want to exit set mode
                    testingButton1 = true;
                    ready = true;
                    int sec = clock.now().second();
                    minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
                }
            }
            else if (ready)
            { //called after we release the button
                ready = false;
                selected++;
                //we have to reprint what we have selected in case it was blinked off
                if (selected == 2)
                    printTime(true);
                else if (selected == 3)
                    printAlarm();
                else
                {
                    printDay();
                    selected = 1;
                }

                lastInteraction = clock.now().second();
            }

            //select selected
            if (digitalRead(button2) == LOW)
            {
                menu = selected;
                if (selected == 1)
                {
                    //prepare to set time
                    printTime(true);
                    newHour = convertHour(clock.now().hour());
                    newMinute = clock.now().minute();
                    am = hourIsAM(clock.now().hour());
                }
                else if (selected == 2)
                {
                    //prints new screen : shows alarms to set
                    lcd.clear();
                    printAlarmSet(0, 0);
                    printAlarmSet(1, 1);
                    printAlarmSet(2, 2);
                    printAlarmSet(3, 3);
                    subMenu = 0;
                }
                else
                {
                    //changes format of date and removes day of the week (WD d/m/yy --> dd/mm/yy)
                    newMonth = clock.now().month();
                    newDay = clock.now().day();
                    newYear = clock.now().year() - 2000;
                    lcd.setCursor(9, 3);
                    lcd.print("   ");
                    lcd.print(newMonth);
                    lcd.setCursor(14, 3);
                    lcd.print("/");
                    lcd.print(newDay);
                    lcd.setCursor(17, 3);
                    lcd.print("/");
                    lcd.print(newYear);
                }
                selected = 1;
                lastInteraction = clock.now().second();
                delay(500);
            }
        }
        //time selcted
        else if (menu == 1)
        {
            switch (selected)
            {
            //blink hours
            case 1:
                if (passedInterval(previous, 500))
                {
                    if (on)
                    {
                        if (newHour < 10)
                            eraseBlock(0, 0, 3, 6);
                        else
                            eraseBlock(0, 0, 3, 8);
                    }
                    else
                        printHour(newHour);
                    on = !on;
                    previous = millis() % 10000;
                }
                break;

            //blink minutes
            case 2:
                if (passedInterval(previous, 500))
                {
                    if (on)
                    {
                        if (newHour < 10)
                            eraseBlock(9, 0, 3, 7);
                        else
                            eraseBlock(11, 0, 3, 7);
                    }
                    else
                        printMinute(newMinute, newHour);
                    on = !on;
                    previous = millis() % 10000;
                }
                break;

            //blink am/pm
            case 3:
                if (passedInterval(previous, 500))
                {
                    if (on)
                    {
                        if (newHour < 10)
                            eraseBlock(17, 0, 2, 1);
                        else
                            eraseBlock(19, 0, 2, 1);
                    }
                    else
                        printAM(newHour, am);
                    on = !on;
                    previous = millis() % 10000;
                }
                break;
            }

            //cycle menu
            if (digitalRead(button1) == LOW)
            {
                if (!testingButton1)
                {
                    testingButton1 = true;
                    ready = true;
                    int sec = clock.now().second();
                    minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
                }
                lastInteraction = clock.now().second();
            }
            else if (ready)
            {
                ready = false;
                selected++;
                if (selected == 2)
                    printHour(newHour);
                else if (selected == 3)
                    printMinute(newMinute, newHour);
                else
                {
                    printAM(newHour, am);
                    selected = 1;
                }
            }

            //increment selected
            if (digitalRead(button2) == LOW)
            {
                if (selected == 1)
                {
                    newHour++;
                    if (newHour > 12)
                        newHour = 1;
                    if (newHour == 10 || newHour == 1)
                    {
                        eraseBlock(0, 0, 3, 20);
                        printMinute(newMinute, newHour);
                        printColon(newHour);
                        printAM(newHour, am);
                    }

                    previous = -500;
                    printHour(newHour);
                }
                else if (selected == 2)
                {
                    newMinute++;
                    if (newMinute >= 60)
                        newMinute = 0;
                    printMinute(newMinute, newHour);
                }
                else
                    am = !am;

                lastInteraction = clock.now().second();
                delay(150);
            }
        }
        //alarm selected
        else if (menu == 2)
        {
            //haven't selected an alarm to edit yet
            if (subMenu == 0)
            {
                //blink alarm thats selected
                if (passedInterval(previous, 500))
                {
                    if (on)
                        printAlarmSet(selected < 4 ? selected - 1 : selected - 5, selected - 1);
                    else
                        eraseBlock(0, selected < 4 ? selected - 1 : selected - 5, 1, 20);
                    on = !on;
                    previous = millis() % 10000;
                }

                //cycle menu
                if (digitalRead(button1) == LOW)
                {
                    if (!testingButton1)
                    {
                        testingButton1 = true;
                        ready = true;
                        int sec = clock.now().second();
                        minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
                    }
                }
                else if (ready)
                {
                    ready = false;
                    if (selected == 4)
                    {
                        printAlarmSet(0, 4);
                        printAlarmSet(1, 5);
                        printAlarmSet(2, 6);
                        printAlarmSet(3, 7);
                    }
                    else if (selected == 8)
                    {
                        selected = 0;
                        printAlarmSet(0, 0);
                        printAlarmSet(1, 1);
                        printAlarmSet(2, 2);
                        printAlarmSet(3, 3);
                    }
                    else
                        printAlarmSet(selected < 4 ? selected - 1 : selected - 5, selected - 1);

                    lastInteraction = clock.now().second();
                    selected++;
                }

                //select selected
                if (digitalRead(button2) == LOW)
                {
                    subMenu = selected;
                    printAlarmSet(selected - 1 < 4 ? selected - 1 : selected - 5, selected - 1);
                    selected = 1;
                    lastInteraction = clock.now().second();
                    delay(500);
                }
            }
            //we have selected a certain alarm slot to edit
            else
            {
                Alarm a = alarms[subMenu - 1];                       //stores the alarm we've chosen to edit
                int r = subMenu - 1 < 4 ? subMenu - 1 : subMenu - 5; //stores the row to print on
                switch (selected)
                {
                //blink alarm hour
                case 1:
                    if (passedInterval(previous, 500))
                    {
                        if (on)
                        {
                            if (a.hour < 10)
                                eraseBlock(3, r, 1, 1);
                            else
                                eraseBlock(3, r, 1, 2);
                        }
                        else
                        {
                            lcd.setCursor(3, r);
                            lcd.print(a.hour);
                        }
                        on = !on;
                        previous = millis() % 10000;
                    }
                    break;

                //blink alarm minute
                case 2:
                    if (passedInterval(previous, 500))
                    {
                        if (on)
                        {
                            if (a.hour < 10)
                                eraseBlock(5, r, 1, 2);
                            else
                                eraseBlock(6, r, 1, 2);
                        }
                        else
                        {
                            if (a.hour < 10)
                                lcd.setCursor(5, r);
                            else
                                lcd.setCursor(6, r);

                            lcd.print(a.minute / 10);
                            lcd.print(a.minute % 10);
                        }
                        on = !on;
                        previous = millis() % 10000;
                    }
                    break;

                //blink alarm AM/PM/OFF
                case 3:
                    if (passedInterval(previous, 500))
                    {
                        if (on)
                            eraseBlock(8, r, 1, 5);
                        else
                        {
                            if (a.hour < 10)
                                lcd.setCursor(8, r);
                            else
                                lcd.setCursor(9, r);

                            if (a.am == 0)
                                lcd.print("OFF");
                            else if (a.am == 1)
                                lcd.print("AM");
                            else
                                lcd.print("PM");
                        }

                        on = !on;
                        previous = millis() % 10000;
                    }
                    break;

                //blinks the letter/day thats selected
                default:
                    if (passedInterval(previous, 500))
                    {
                        lcd.setCursor(13 + (selected - 4), r);
                        if (on)
                            lcd.print(" ");
                        else
                        {
                            if (a.days[selected - 4])
                                lcd.print(days[selected - 4]);
                            else
                                lcd.write(tolower(days[selected - 4]));
                        }
                        on = !on;
                        previous = millis() % 10000;
                    }
                    break;
                }

                //cycle selection
                if (digitalRead(button1) == LOW)
                {
                    if (!testingButton1)
                    {
                        testingButton1 = true;
                        ready = true;
                        int sec = clock.now().second();
                        minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
                    }
                }
                else if (ready)
                {
                    ready = false;
                    selected++;
                    lastInteraction = clock.now().second();
                    if (selected > 10)
                    {
                        selected = subMenu;
                        subMenu = 0;
                    }
                    printAlarmSet(r, subMenu - 1);
                }

                //increment selection
                if (digitalRead(button2) == LOW)
                {
                    if (selected == 1)
                    {
                        alarms[subMenu - 1].hour++;
                        if (alarms[subMenu - 1].hour > 12)
                            alarms[subMenu - 1].hour = 1;

                        if (alarms[subMenu - 1].hour == 10 || alarms[subMenu - 1].hour == 1)
                            printAlarmSet(r, subMenu - 1);

                        lcd.setCursor(3, r);
                        lcd.print(alarms[subMenu - 1].hour);
                        previous = millis() + 500;
                    }
                    else if (selected == 2)
                    {
                        alarms[subMenu - 1].minute++;
                        if (alarms[subMenu - 1].minute >= 60)
                            alarms[subMenu - 1].minute = 0;

                        if (a.hour < 10)
                            lcd.setCursor(5, r);
                        else
                            lcd.setCursor(6, r);

                        lcd.print(a.minute / 10);
                        lcd.print(a.minute % 10);

                        previous = millis() + 500;
                    }
                    else if (selected == 3)
                    {
                        alarms[subMenu - 1].am++;
                        if (alarms[subMenu - 1].am > 2)
                            alarms[subMenu - 1].am = 0;

                        if (a.hour < 10)
                            lcd.setCursor(8, r);
                        else
                            lcd.setCursor(9, r);

                        if (a.am == 0)
                            lcd.print("OFF");
                        else if (a.am == 1)
                            lcd.print("AM");
                        else
                            lcd.print("PM");

                        previous = millis() + 500;
                    }
                    else
                    {
                        alarms[subMenu - 1].days[selected - 4] = !a.days[selected - 4];
                        printAlarmSet(r, subMenu - 1);
                    }

                    lastInteraction = clock.now().second();
                    delay(150);
                }
            }
        }
        //date selcted
        else if (menu == 3)
        {
            switch (selected)
            {
            //blink the month
            case 1:
                if (passedInterval(previous, 500))
                {
                    lcd.setCursor(12, 3);
                    if (on)
                        lcd.print(newMonth);
                    else
                        lcd.print("  ");
                    on = !on;
                    previous = millis() % 10000;
                }
                break;

            //blink the day
            case 2:
                if (passedInterval(previous, 500))
                {
                    lcd.setCursor(15, 3);
                    if (on)
                        lcd.print(newDay);
                    else
                        lcd.print("  ");
                    on = !on;
                    previous = millis() % 10000;
                }
                break;

            //blink the year
            case 3:
                if (passedInterval(previous, 500))
                {
                    lcd.setCursor(18, 3);
                    if (on)
                        lcd.print(newYear);
                    else
                        lcd.print("  ");
                    on = !on;
                    previous = millis() % 10000;
                }
                break;
            }

            //cycle menu
            if (digitalRead(button1) == LOW)
            {
                if (!testingButton1)
                {
                    testingButton1 = true;
                    ready = true;
                    int sec = clock.now().second();
                    minReleaseTime = sec + 2 < 60 ? sec + 2 : sec + 2 - 60;
                }
            }
            else if (ready)
            {
                ready = false;
                selected++;
                if (selected == 2)
                {
                    lcd.setCursor(12, 3);
                    lcd.print(newMonth);
                }
                else if (selected == 3)
                {
                    lcd.setCursor(15, 3);
                    lcd.print(newDay);
                }
                else
                {
                    lcd.setCursor(18, 3);
                    lcd.print(newYear);
                    selected = 1;
                }
                lastInteraction = clock.now().second();
            }

            //increment selected
            if (digitalRead(button2) == LOW)
            {
                previous = millis() + 500;
                if (selected == 1)
                {
                    newMonth++;
                    if (newMonth > 12)
                        newMonth = 1;
                    lcd.setCursor(12, 3);
                    lcd.print(newMonth);
                }
                else if (selected == 2)
                {
                    newDay++;
                    if (newDay > 31)
                        newDay = 1;
                    lcd.setCursor(15, 3);
                    lcd.print(newDay);
                }
                else
                {
                    newYear++;
                    if (newYear > 99)
                        newYear = 0;
                    lcd.setCursor(18, 3);
                    lcd.print(newYear);
                }

                lastInteraction = clock.now().second();
                delay(150);
            }
        }

        if (lastInteraction > clock.now().second() ? 60 - lastInteraction + clock.now().second() > 30 : clock.now().second() - lastInteraction > 20)
        {
            setMode = false;
            lcd.clear();
            printTime(true);
            printDay();
            printAlarm();
            menu = 0;
            selected = 1;
        }
    }

    if (snoozing && !setMode)
    {
        lcd.setCursor(1, 3);
        lcd.print("SNZ");
        lcd.print(snoozeCount);
        lcd.print(":");
        int remaining = currentAlarm.hour != convertHour(clock.now().hour()) ? currentAlarm.minute + 60 - clock.now().minute() : currentAlarm.minute - clock.now().minute();
        lcd.print(remaining);
        lcd.print("m");
    }

    //if we are currently testing button 1 for a hold
    if (testingButton1)
    {
        if (clock.now().second() == minReleaseTime && !buttonHeld)
        {
            //we've held the button if we haven't released and we've passed the minimum release time
            analogWrite(buzzerPin, 150);
            delay(100);
            analogWrite(buzzerPin, 0);
            buttonHeld = true;
        }

        if (clock.now().second() == minReleaseTime2 && !buttonHeld2)
        {
            analogWrite(buzzerPin, 150);
            delay(100);
            analogWrite(buzzerPin, 0);
            buttonHeld2 = true;
        }

        if (digitalRead(button1) == HIGH)
        {
            //when we release the button:
            //if we have haven't held it for long enough, stop the test
            //if we have held it either exit set mode and set the new tine/day or enter set mode
            if (buttonHeld)
            {
                if (buttonHeld2 && !setMode)
                {
                    digitalWrite(readSwitch, HIGH);
                    delay(10);

                    eraseBlock(0, 3, 1, 20);
                    lcd.setCursor(0, 3);
                    lcd.print("BATTERY: ");
                    lcd.print(round(analogRead(voltage) * (3.3 / 1023) / 2.25 * 100));
                    lcd.print("% ");
                    lcd.print(analogRead(voltage) * (3.3 / 1023) * 2);
                    lcd.print("V");

                    delay(10);
                    digitalWrite(readSwitch, LOW);

                    delay(5000);

                    eraseBlock(0, 3, 1, 20);
                    printAlarm();
                    printDay();
                }
                else if (setMode)
                {
                    if (menu == 1)
                    {
                        setClock(DateTime(clock.now().year(), clock.now().month(), clock.now().day(), hourTo24(newHour, am), newMinute, 0));
                        printTime(true);
                        findNextAlarm();
                        printAlarm();
                    }
                    else if (menu == 2)
                    {
                        findNextAlarm();
                        lcd.clear();
                        printTime(true);
                        printDay();
                        findNextAlarm();
                        printAlarm();
                    }
                    else if (menu == 3)
                    {
                        setClock(DateTime(newYear, newMonth, newDay, clock.now().hour(), clock.now().minute(), clock.now().second()));
                        printDay();
                        findNextAlarm();
                        printAlarm();
                    }
                    else
                    {
                        printTime(true);
                        printAlarm();
                        printDay();
                        printAlarm();
                    }
                    selected = 1;
                    menu = 0;
                    setMode = false;
                }
                else
                {
                    lastInteraction = clock.now().second();
                    setMode = true;
                }

                delay(500);
            }

            testingButton1 = false;
            buttonHeld = false;
            buttonHeld2 = false;
        }
    }

    if (testingButton2)
    {
        if (clock.now().second() == minReleaseTime && !buttonHeld)
        {
            //we've held the button if we haven't released and we've passed the minimum release time
            analogWrite(buzzerPin, 150);
            delay(100);
            analogWrite(buzzerPin, 0);
            buttonHeld = true;
        }

        if (digitalRead(button2) == HIGH)
        {
            //when we release the button:
            //if we have haven't held it for long enough, stop the test
            if (buttonHeld)
            {
                if (alarmFlag || snoozing)
                {
                    alarmFlag = false;
                    findNextAlarm();
                    printAlarm();
                    if (currentAlarm.days[clock.now().dayOfTheWeek()] && currentAlarm.am > 0 && convertHour(clock.now().hour()) == currentAlarm.hour && clock.now().minute() == currentAlarm.minute && (hourIsAM(clock.now().hour()) ? 1 : 2) == currentAlarm.am)
                        tempDis = true;
                    snoozing = false;
                }
                else
                {
                    analogWrite(buzzerPin, 150);
                    delay(100);
                    analogWrite(buzzerPin, 0);
                    delay(100);
                    analogWrite(buzzerPin, 150);
                    delay(100);
                    analogWrite(buzzerPin, 0);
                    delay(100);
                    analogWrite(buzzerPin, 150);
                    delay(100);
                    analogWrite(buzzerPin, 0);

                    currentAlarmEnabled = !currentAlarmEnabled;
                    printAlarm();
                }

                delay(500);
            }

            testingButton2 = false;
            buttonHeld = false;
        }
    }

    if (!alarmFlag && !lightScreen && !testingButton1 && !setMode)
    {
        Serial.print("sleep");
        delay(50);
        sleep();
    }
}
