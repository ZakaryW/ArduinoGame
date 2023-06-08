/***********************************************************************
Author:   Zakary Watts

About:    Basic arudino simon says game. Match outputs with 
          corresponding inputs. 4 different output/ input
          events and modifiable difficulty.
***********************************************************************/
#include <Adafruit_CircuitPlayground.h>
#include <AsyncDelay.h>

//Represents events (input/ output pairs)
enum event : int8_t{
  E_NULL = -1,
  rPress,
  lPress, 
  rTilt,
  lTilt 
  };

volatile int8_t level = 1;         //Level variable (should not exceed 9)
float accelInput;                  //Accelerometer input variable
event userEvent = event::E_NULL;   //User event var
event randomEvent[9];              //Random event array
bool hasWon = false;               //True if the game is won
bool hasLost = false;              //True if the game is lost
bool toCont;                       //Extra loop/ conditional variable
AsyncDelay inputTimer;             //Async timer for timing delays

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  randomSeed(analogRead(0));       //Seeding for random function

  WelcomePattern();
  CircuitPlayground.clearPixels();
}

void loop() 
{
  Serial.println(level);                                                            //Testing, optional inclusion
  int i;                                                                            //Index variable

  delay(100);                                                                       //Various delays for experience "smoothing"

  if (!hasLost && !hasWon && level < 8)                                             //Coditional to run the game loop
  {
    for (i=0; i<9; i++)                                                             //Generate psuedorandom events to be matched
    {
      randomEvent[i] = static_cast<event>(random(0,4));                             //Using the random function to return an int [0,4) and casting for corresponding event assignment
    }
    
    for (i=0; i<level; i++)
    {
      CircuitPlayground.clearPixels();                                              //Displays the generated events to match limited by level
      delay(500);                                                                   //Smoothing delays
      EventLED(randomEvent[i]);
      delay(1000);
    }
    CircuitPlayground.clearPixels();

    PreInput();
    CircuitPlayground.clearPixels();
    delay(100);

    for(i=0; (i<level) && (!hasLost) && (!hasWon); i++)                             //Input loop
    {
      toCont = false;                                                               //Setting relevant variables in preperation for input
      event userEvent = event::E_NULL;
      inputTimer.start(10000, AsyncDelay::MILLIS);                                   //Amount of time to register an input
      //TODO add corresponding sound component
      while(!toCont)                                                                //Event input stream loop, continually checks for input events
      {
        if(CircuitPlayground.rightButton() && !toCont)                              //Right button input
        {
          CircuitPlayground.playTone(100, 150);
          delay(50);
          userEvent = event::rPress;
          EventLED(userEvent);
          delay(100); 
          toCont = true;          
        }
        if(CircuitPlayground.leftButton() && !toCont)                                //Left button input
        {
          CircuitPlayground.playTone(100, 150);
          delay(50);
          userEvent = event::lPress;
          EventLED(userEvent);
          delay(100);
          toCont = true;
        }
        if(RightTilt() && !toCont)                                                   //Right tilt input
        {
          CircuitPlayground.playTone(50, 150);
          delay(50);
          userEvent = event::rTilt;
          EventLED(userEvent);
          delay(100);
          toCont = true;
        }
        if(LeftTilt() && !toCont)                                                    //Left tilt input
        {
          CircuitPlayground.playTone(50, 150);
          delay(50);
          userEvent = event::lTilt;
          EventLED(userEvent);
          delay(100);
          toCont = true;        
        }
        if(inputTimer.isExpired() && !toCont)                                        //If the timer is up and no input has been made, you loose
        {
        hasLost = true;
        toCont = true; 
        } 
        CircuitPlayground.clearPixels();    
      }

      if(userEvent != randomEvent[i])                                                //Checks if the current input matches the corresponding generated input, if not game lost
      hasLost = true; 
    }

    inputTimer.expire();                                                             //Expire the timer and prepare for re-execution of the arduino loop (void)
    if (!hasLost && !hasWon)
    level++;
  }

  else if(level >= 8 && !hasLost)                                                    //Corresponds to the game loop conditional, execution upon respective game state (won or lost)
  {
    CircuitPlayground.clearPixels();
    WinningPattern();
    delay(3000);
    hasLost = true;
    hasWon = false;
    level = 1;
    setup();
    loop();
  }
  else if(hasLost && !hasWon)
  {
    CircuitPlayground.clearPixels();
    LosingPattern();
    delay(3000);
    hasLost = false;
    hasWon = false;
    level = 1;
    setup();
    loop();
  }  
}
//TODO add sound component
//Handles the LED output for events
void EventLED(event dispEvent)
{
  int i;

  switch(dispEvent)
  {
  case rPress:
  CircuitPlayground.playTone(100, 150);
  CircuitPlayground.setPixelColor(7, 255, 0, 0);
  break;
  case lPress:
  CircuitPlayground.playTone(100, 150);
  CircuitPlayground.setPixelColor(2, 0, 0, 255);
  break;
  case rTilt:
  for (i=0; i<5; ++i)
  CircuitPlayground.setPixelColor(i, 255, 0, 0);
  CircuitPlayground.playTone(50, 150);
  break;
  case lTilt:
  for (i=5; i<10; ++i)
  CircuitPlayground.setPixelColor(i, 0, 0, 255);
  CircuitPlayground.playTone(50, 150);
  break;
  default:
  break;
  }

  return;  
}

//Tilt functions adapted from: https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing
//////////////////
//Left tilt check
bool RightTilt()
{
  float sensorX = CircuitPlayground.motionX() / 9.8;
  float sensorZ = CircuitPlayground.motionZ() / 9.8;
  float angle = atan(sensorX / sensorZ) * 180 / PI;

  if (angle > 40.0)
  return true;
  else
  return false; 
}
//Right tilt check
bool LeftTilt()
{
  float sensorX = CircuitPlayground.motionX() / 9.8;
  float sensorZ = CircuitPlayground.motionZ() / 9.8;
  float angle = atan(sensorX / sensorZ) * 180 / PI;

  if ( (angle * -1) > 40.0)
  return true;
  else
  return false;
}

//Losing pattern
void LosingPattern()
{
  int i;

  for(i=0; i<10; ++i)
  CircuitPlayground.setPixelColor(i, 255, 0, 0);

  for(i=10; i>=0; --i)
  {
    if(i % 3 == 0)
    delay(800);
    
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
  }
}
//Winning pattern - Credit to Adafruit for pattern adaptation
void WinningPattern()
{
  int currentSpeed = 100;
  int stripLen = 10;
  uint32_t offset = millis() / currentSpeed;

  for(int i=0; i<10; ++i) 
  CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / stripLen) + offset) & 255));
}
//Welcom pattern upon start-up execution
void WelcomePattern()
{
  int i;

  for(i=0; i<3; ++i)
  CircuitPlayground.setPixelColor(i, 0, 255, 255);
  delay(700);  
  for(i=3; i<6; ++i)
  CircuitPlayground.setPixelColor(i, 0, 255, 255);
  delay(700);
  for(i=6; i<10; ++i)
  CircuitPlayground.setPixelColor(i, 0, 255, 255);
  delay(700);
}

void PreInput()
{
  int i;
  int j;

  for(j=0; j<2; j++)
  {
    for(i=0; i<10; ++i)
    CircuitPlayground.setPixelColor(i, 255, 0, 255);
    delay(400);
    for(i=0; i<10; ++i)
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
    delay(400) ;
  }

}

//Possible TODO: Quiet mode - disable sound with an switch input
