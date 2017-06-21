//##########################################################
//
// Veggie Irrigation System
//
//##########################################################
// v1.0
//

// This #include statement was automatically added by the Particle IDE.
#include <DailyTimerSpark.h>
#include "thingspeak.h"


//##########################################################
// User configurable values
//##########################################################

bool DebugModeOn = false;

int Timer1StartHour = 9;
int Timer1StartMinute = 30;
int Timer1StopHour = 10;
int Timer1StopMinute = 0;


//##########################################################
// Hardware and Software addresses
//##########################################################
ThingSpeakLibrary::ThingSpeak thingspeak ("2ZB278OS8VLX5BN3");

//##########################################################
// Module level variables
//##########################################################
int Relay1On = 0;

int relay1 = D6;

bool timer1_LastState = false;

int lastSecond = 0;

DailyTimer timer1(Timer1StartHour, Timer1StartMinute, Timer1StopHour, Timer1StopMinute, EVERY_DAY);

//##########################################################
void setup(void)
{

    Publish("setup","Start");
    

    Particle.function("SwitchDebug", fnSwitchDebugMode);
    Particle.function("WaterOn", fnRelay1On);
    Particle.function("WaterOff", fnRelay1Off);

    Particle.variable("IsWaterOn", Relay1On);

    pinMode(relay1, OUTPUT);
    digitalWrite(relay1,HIGH);

    timer1.begin();

    Particle.process();
 
}

void loop(void)
{
    bool timerState = false;
     
    try 
    {

      int intr;
      
      if (Time.second() != lastSecond)
      {
             delay(10000);
            ////////////////////////////////////////////////////////////////////////////////////////////
            // Background Timer code
            ////////////////////////////////////////////////////////////////////////////////////////////
            Publish("loop","Heartbeat " + String(Time.second()) + " " + String(lastSecond));
            
            timerState = timer1.isActive();  //State Change method this block
            if(timerState != timer1_LastState)
            {
                if(timerState)
                {
                    if (Relay1On == 0)
                        { intr = fnRelay1On("");  }
                    
                    Publish("DailyTimer","Water ON");
                }
                else
                {
                    if (Relay1On == 1)
                        { intr = fnRelay1Off("");  }
                    
                    Publish("DailyTimer","Water OFF");
                }
                
                timer1_LastState = timerState;
            }
            
            // Send data back to ThingSpeak
            double startTime = double(Timer1StartHour) + (double(Timer1StartMinute) / double(60));
            double endTime = double(Timer1StopHour) + (double(Timer1StopMinute) / double(60));
            double currTime = double(Time.hour()) + (double(Time.minute()) / double(60));
            Publish("loop",String(startTime));
            Publish("loop",String(endTime));
            bool valSet = thingspeak.recordValue(1, String(Relay1On));
            valSet = thingspeak.recordValue(2, String(startTime));
            valSet = thingspeak.recordValue(3, String(endTime));
            valSet = thingspeak.recordValue(4, String(currTime));
            bool valsSent = thingspeak.sendValues();

      }
    
       
    } 
    catch (const char* msg) 
    {
          Publish("loop-error", String(msg));
    }
    
    
            
}


int fnRelay1On(String command)
{
    Relay1On = 1;
    digitalWrite(relay1, LOW);
    delay(2000);
    Publish("Relay 1", "Water ON");
    return 1;
}


int fnRelay1Off(String command)
{
    Relay1On = 0;
    digitalWrite(relay1, HIGH);
    delay(2000);
    Publish("Relay 1", "Water OFF");
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
    return 1;
}
