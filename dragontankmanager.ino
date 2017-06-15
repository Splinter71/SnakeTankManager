//##########################################################
//
//  DDDD    RRRR       AAA       GGG       OOO      N   N
//  D   D   R   R     A   A     G   G     O   O     N   N
//  D   D   R  R      A   A     G         O   O     NN  N
//  D   D   RRR       AAAAA     G         O   O     N N N
//  D   D   R  R      A   A     G  GG     O   O     N  NN
//  D   D   R   R     A   A     G   G     O   O     N   N
//  DDDD    R    R    A   A      GGG       OOO      N   N
//
//##########################################################
//
// v1.4
//
// This #include statement was automatically added by the Particle IDE.
#include <clickButton.h>
#include <DailyTimerSpark.h>
#include "thingspeak.h"
#include "DS18B20.h"
#include "LiquidCrystal_I2C_Spark.h"

//##########################################################
// User configurable values
//##########################################################
bool DebugModeOn = false;

double Sensor1OnTmp = 36.000; // Tank warm side
double Sensor2OnTmp = 35.000; // Tank cool side
double Sensor3OnTmp = 40.000; //  Internal Pi temp
int Relay1SwitchTime = 300; //5 minutes ie. Only switch heat-lamp1 back on after 5 min of being off.

DailyTimer timer1(7, 30,  19, 30, EVERY_DAY);
DailyTimer timer2(7, 30,  19, 30, EVERY_DAY);
DailyTimer timer3(7, 30,  19, 30, EVERY_DAY);
DailyTimer timer4(7, 30,  19, 30, EVERY_DAY);


//##########################################################
// Hardware and Software addresses
//##########################################################
uint8_t LCD_Address = 0x27;

ThingSpeakLibrary::ThingSpeak thingspeak ("JWMS8QQGQJIEVGAZ");

char Sensor1Addy[16];
char Sensor2Addy[16];
char Sensor3Addy[16];

double Sensor1TempC;
double Sensor2TempC;
double Sensor3TempC;
char *Sensor1Addr = "28-0316a33e4fff"; // Tank warm side
char *Sensor2Addr = "28-0316a30c67ff"; // Tank cool side
char *Sensor3Addr = "28-0416a2dee0ff"; // Internal PI enclosure temperature

// the Button
const int buttonPin1 = D16;
ClickButton button1(buttonPin1, LOW, CLICKBTN_PULLUP);
int buttonFunction = 0;
//##########################################################

bool timer1_LastState = false;
bool timer2_LastState = false;
bool timer3_LastState = false;
bool timer4_LastState = false;

int Relay1On = 0;
int Relay2On = 0;
int Relay3On = 0;
int Relay4On = 0;

LiquidCrystal_I2C *lcd;

int relay1 = D7;
int relay2 = D6;
int relay3 = D5;
int relay4 = D4;

int lastSecond = 0;
bool pauseClock = false;
bool BackLightOn = false;

time_t currTime = Time.now();
time_t relay1SwitchTime = Time.now();
int relay1ElapsedTime = 0;        

// Custom Characters
uint8_t degC[8] = {0x8,0xf4,0x8,0x43,0x4,0x4,0x43,0x0};  // degree c
uint8_t testChar2[8] = { 0b10101,0b10101,0b10101,0b10101,0b11111,0b00100,0b01010,0b10101 };
uint8_t bell[8] = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t note[8] = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t _clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t duck[8] = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t retarrow[8] = { 0x1,0x1,0x5,0x9,0x1f,0x8,0x4};



void handler(const char *topic, const char *data) {
    Particle.publish("handler", "received " + String(topic) + ": " + String(data), PRIVATE);
}

void setup(void)
{

    Publish("setup","Start");
    
    strcpy(Sensor1Addy, Sensor1Addr);
    strcpy(Sensor2Addy, Sensor2Addr);
    strcpy(Sensor3Addy, Sensor3Addr);
    
    Particle.function("backlightOn", fnBacklightOn);
    Particle.function("backlightOff", fnBacklightOff);

    Particle.function("SwitchDebug", fnSwitchDebugMode);

    Particle.variable("Sensor1TempC", Sensor1TempC);
    Particle.variable("Sensor2TempC", Sensor2TempC);
    Particle.variable("Sensor3TempC", Sensor3TempC);
    Particle.variable("Sensor1OnTmp", Sensor1OnTmp);
    Particle.variable("Sensor2OnTmp", Sensor2OnTmp);
    Particle.variable("Sensor3OnTmp", Sensor3OnTmp);
    
    Particle.variable("Relay1On", Relay1On);
    Particle.variable("Relay2On", Relay2On);
    Particle.variable("Relay3On", Relay3On);
    Particle.variable("Relay4On", Relay4On);
    

    pinMode(relay1, OUTPUT);
    digitalWrite(relay1,HIGH);
    pinMode(relay2, OUTPUT);
    digitalWrite(relay2,HIGH);
    pinMode(relay3, OUTPUT);
    digitalWrite(relay3,HIGH);
    pinMode(relay4, OUTPUT);
    digitalWrite(relay4,HIGH);
    
    pinMode(D16, INPUT_PULLUP);
    // Setup button timers (all in milliseconds / ms)
    // (These are default if not set, but changeable for convenience)
    button1.debounceTime   = 20;   // Debounce timer in ms
    button1.multiclickTime = 250;  // Time limit for multi clicks
    button1.longClickTime  = 1000; // time until "held-down clicks" register

    timer1.begin();
    timer2.begin();
    timer3.begin();
    timer4.begin();

    Particle.process();
 
    lcd = new LiquidCrystal_I2C(LCD_Address, 16, 2);
    lcd->init();
    //lcd->backlight();
    lcd->noBacklight();
    lcd->clear();

  //lcd->createChar(0, degC);
  //lcd->createChar(1, testChar2);
  //lcd->createChar(2, bell);
  //lcd->createChar(3, note);
  //lcd->createChar(4, _clock);
  //lcd->createChar(5, heart);
  //lcd->createChar(6, duck);
  //lcd->createChar(7, check);
  //lcd->createChar(8, cross);
  //lcd->createChar(9, retarrow);
  

}

void loop(void)
{
    try 
    {
      currTime = Time.now();
      relay1ElapsedTime = currTime - relay1SwitchTime;

      int intr;
      bool timerState;
       
      if (Time.second() != lastSecond)
        if (!pauseClock)
          {
            lcd->createChar(1, degC);
            lcd->setCursor(0,0);
            lcd->print("1:" + String(Sensor1TempC,0) + ((char)1) + " 2:" + String(Sensor2TempC,0) + ((char)1) + " 3:" + String(Sensor3TempC,0));

            lcd->setCursor(0,1);
            lcd->print(Time.hour() < 10? "0" : "");
            lcd->print(Time.hour());
            lcd->print(Time.minute() < 10? ":0": ":");
            lcd->print(Time.minute());
            lcd->print(Time.second() < 10? ":0": ":");
            lcd->print(Time.second());
            lcd->print("    ");
            
            lcd->setCursor(12,1);
            if (Relay1On==1)
                { lcd->print("1"); }
            else
                { lcd->print(" "); }

            if (Relay2On==1)
                { lcd->print("2"); }
            else
                { lcd->print(" "); }
            
            if (Relay3On==1)
                { lcd->print("3"); }
            else
                { lcd->print(" "); }

            if (Relay4On==1)
                { lcd->print("4"); }
            else
                { lcd->print(" "); }
            
            
            lastSecond = Time.second();
            
            
            if (lastSecond==30)
            {
                  Particle.process();
                  Sensor3TempC = 80;
                  
                  DS18B20 w1Device1 (Sensor1Addy);
                  Sensor1TempC = w1Device1.getTemp();
                  Sensor1TempC = round(Sensor1TempC*10)/10;

                  DS18B20 w1Device2 (Sensor2Addy);
                  Sensor2TempC = w1Device2.getTemp();
                  Sensor2TempC = round(Sensor2TempC*10)/10;

                  DS18B20 w1Device3 (Sensor3Addy);
                  Sensor3TempC = w1Device3.getTemp();
                  Sensor3TempC = round(Sensor3TempC*10)/10;
    

                  Publish("Tank Warm Side",String(Sensor1TempC,0));
                  delay(1000);
                  Publish("Warm Side Switch Elapsed", String(relay1ElapsedTime));
                  delay(1000);
                  Publish("Tank Cool Side", String(Sensor2TempC,0));
                  delay(1000);
                  Publish("Pi Internal Temp", String(Sensor3TempC,0));

                  timerState = timer1.isActive();  //State Change method this block
                  //if(timerState != timer1_LastState)
                  //{
                        if(timerState)
                        {
                            if (Sensor1TempC <= Sensor1OnTmp)
                            {
                              if (Relay1On == 0 && relay1ElapsedTime > Relay1SwitchTime)
                              {
                                intr = fnRelay1On("");          
                              }
                            }
                            else
                            {
                              if (Relay1On == 1)
                              {
                                intr = fnRelay1Off("");    
                              }
                            }

                        }
                        else
                        {
                            if (Relay1On == 1)
                                { 
                                    intr = fnRelay1Off("");  
                                    Publish("DailyTimer", "Relay 1 Off");
                                }
                            
                            
                        }
                        timer1_LastState = timerState;
                  //}


                  //if (Sensor2TempC <= Sensor2OnTmp)
                  //{
                    //  if (Relay2On == 0)
                     // {
                      //  intr = fnRelay2On("");
                        //intr = fnRelay3On("");          
                     // }
                  //}
                  //else
                  //{
                    //  if (Relay2On == 1)
                      //{
                        //intr = fnRelay2Off("");
                        //intr = fnRelay3Off("");    
                      //}
                 // }
                  
                  if (Sensor3TempC >= Sensor3OnTmp)
                  {
                      Particle.publish("WARNING", String(Sensor3TempC), PRIVATE);
                  }

                  // Send data back to ThingSpeak
                  bool valSet = thingspeak.recordValue(1, String(Sensor1TempC,0));
                  valSet = thingspeak.recordValue(2, String(Sensor2TempC,0));
                  valSet = thingspeak.recordValue(3, String(Sensor3TempC,0));
                  valSet = thingspeak.recordValue(4, String(Relay1On));
                  valSet = thingspeak.recordValue(5, String(Relay2On));
                  valSet = thingspeak.recordValue(6, String(Relay3On));
                  valSet = thingspeak.recordValue(7, String(Relay4On));
                  bool valsSent = thingspeak.sendValues();

            }
          }
         else
         {
             lcd->setCursor(0,1);
             lcd->print((char)4);
         }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Background Timer code
        ////////////////////////////////////////////////////////////////////////////////////////////
       

        
        timerState = timer2.isActive();  //State Change method this block
        if(timerState != timer2_LastState)
        {
            if(timerState)
            {
                if (Relay2On == 0)
                    { intr = fnRelay2On("");  }
                
                Publish("DailyTimer", "Relay 2 On");
            }
            else
            {
                if (Relay2On == 1)
                    { intr = fnRelay2Off("");  }
                
              Publish("DailyTimer", "Relay 2 Off");
            }
            timer2_LastState = timerState;
        }
        
        timerState = timer3.isActive();  //State Change method this block
        if(timerState != timer3_LastState)
        {
            if(timerState)
            {
                if (Relay3On == 0)
                    { intr = fnRelay3On("");  }
                
                Publish("DailyTimer", "Relay 3 On");
            }
            else
            {
                if (Relay3On == 1)
                    { intr = fnRelay3Off("");  }
                
              Publish("DailyTimer", "Relay 3 Off");
            }
            timer3_LastState = timerState;
        }


        timerState = timer4.isActive();  //State Change method this block
        if(timerState != timer4_LastState)
        {
        if(timerState)
        {
            if (Relay4On == 0)
                { intr = fnRelay4On("");  }
            
            Publish("DailyTimer", "Relay 4 On");
        }
        else
        {
            if (Relay4On == 1)
                { intr = fnRelay4Off("");  }
            
            Publish("DailyTimer", "Relay 4 Off");
        }
        timer4_LastState = timerState;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Push button code
        ////////////////////////////////////////////////////////////////////////////////////////////
        // Update button state
        button1.Update();
        
        // Save click codes, as click codes are reset at next Update()
        if(button1.clicks != 0) buttonFunction = button1.clicks;
        
        if(buttonFunction == 1) 
        // SINGLE CLICK
        {   // Toggle the BackLight
            if(BackLightOn)
                { intr = fnBacklightOff(""); }
            else
                { intr = fnBacklightOn(""); }
        }

        if(buttonFunction == 2) Publish("buttonFunction", "DOUBLE click"); 
        //Publish();
        if(buttonFunction == 3) Publish("buttonFunction", "TRIPLE click"); 
        
        if(buttonFunction == -1) Publish("buttonFunction", "SINGLE LONG click");
        
        if(buttonFunction == -2) Publish("buttonFunction", "DOUBLE LONG click");
        
        if(buttonFunction == -3) Publish("buttonFunction", "TRIPLE LONG click");
        
        buttonFunction = 0;
        delay(5);

    } 
    catch (const char* msg) 
    {
          Publish("loop-error", String(msg));
    }
            
}

int fnBacklightOn(String command)
{
    lcd->backlight();
    BackLightOn = true;
    return 1;
}

int fnBacklightOff(String command)
{
    lcd->noBacklight();
    BackLightOn = false;
    return 1;
}

int fnPauseClock(String command)
{
    pauseClock = true;
    return 1;
}

int fnResumeClock(String command)
{
    pauseClock = false;
    return 1;
}

int fnRelay1On(String command)
{
    Relay1On = 1;
    pauseClock = true;
    digitalWrite(relay1, LOW);
    relay1SwitchTime = Time.now();
    lcd->setCursor(0,1);
    lcd->print("HEAT LAMP 1 ON..");
    delay(2000);
    Publish("Heat Lamp 1", "Heat Lamp 1 ON");
    pauseClock = false;
    return 1;
}

int fnRelay2On(String command)
{
    Relay2On = 1;
    pauseClock = true;
    digitalWrite(relay2, LOW);
    lcd->setCursor(0,1);
    lcd->print("HEAT LAMP 2 ON..");
    delay(2000);
    pauseClock = false;
    return 1;
}

int fnRelay3On(String command)
{
    Relay3On = 1;
    pauseClock = true;
    digitalWrite(relay3, LOW);
    lcd->setCursor(0,1);
    lcd->print("LIGHT ON..");
    delay(2000);
    pauseClock = false;
    return 1;
}

int fnRelay4On(String command)
{
    Relay4On = 1;
    pauseClock = true;
    digitalWrite(relay4, LOW);
    lcd->setCursor(0,1);
    lcd->print("HEAT MAT ON.....");
    delay(2000);
    pauseClock = false;
    return 1;
}

int fnRelay1Off(String command)
{
    Relay1On = 0;
    pauseClock = true;
    digitalWrite(relay1, HIGH);
    relay1SwitchTime = Time.now();
    lcd->setCursor(0,1);
    lcd->print("HEAT LAMP 1 OFF.");
    delay(2000);
    Publish("Heat Lamp 1", "Heat Lamp 1 OFF");
    pauseClock = false;
    return 1;
}

int fnRelay2Off(String command)
{
    Relay2On = 0;
    pauseClock = true;
    digitalWrite(relay2, HIGH);
    lcd->setCursor(0,1);
    lcd->print("HEAT LAMP 1 OFF.");
    delay(2000);
    pauseClock = false;
    return 1;
}

int fnRelay3Off(String command)
{
    Relay3On = 0;
    pauseClock = true;
    digitalWrite(relay3, HIGH);
    lcd->setCursor(0,1);
    lcd->print("HEAT LAMP 2 OFF.");
    delay(2000);
    pauseClock = false;
    return 1;
}

int fnRelay4Off(String command)
{
    Relay4On = 0;
    pauseClock = true;
    digitalWrite(relay4, HIGH);
    lcd->setCursor(0,1);
    lcd->print("HEAT MAT OFF....");
    delay(2000);
    pauseClock = false;
    return 1;
}

int fnShowMessage(String message)
{
    pauseClock = true;
    lcd->setCursor(0,1);
    lcd->print(message);
    delay(3000);
    pauseClock = false;
    return 1;
}

int fnSwitchDebugMode(String command)
{
    DebugModeOn = !DebugModeOn;
    return DebugModeOn;
}

int Publish(String section, String message)
{
    if (DebugModeOn) { Particle.publish(section, message, PRIVATE); }
}
//int showIPAddress(String command)
//{
//  lcd->clear();
//  IPAddress myIP = WiFi.localIP();
//  String ipStr = "IP:"+String(myIP[0])+"."+String(myIP[1])+"."+String(myIP[2])+"."+String(myIP[3]);
//  Spark.publish("LocalIP", ipStr, 60,PRIVATE);
//  lcd->print(ipStr);
//  delay(5000); //30 sec
//
//    return 1;
//}
