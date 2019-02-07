/****************************************************/
/* environment constants */
/****************************************************/
const unsigned int btnPin = 2;
const unsigned int potPin = A0;
const unsigned int actionLed = 13;
const int num_pins = 10;

/* helper functions */

// discreet values of the pins positions
int pinPosition(int index)
{
  return 12 - num_pins + index + 1;
}

// get potentiometer valuefrom 1 to num_pins
int getPot()
{
  int valPot = analogRead(potPin);
  return map(valPot, 0, 1023, num_pins, 1);
}

// get tick value from potentiometer
long getTick()
{
  int valPot = analogRead(potPin);
  return map(long(valPot), 0, 1023, 10, 1000);
}

// digitalWrite a state mapped in bits
void digitalWriteState(int state)
{
  for (int i = 0; i < num_pins; i++)
  {
    int pin = pinPosition(i);
    int bit = (1 << i);
    bool on = (bit & state) == bit;
    digitalWrite(pin, on);
  }
}

// state of push btn
int buttonState = 0;

// listen for changes in btn state
bool hasButtonPressed()
{
  int newButtonState = digitalRead(btnPin);
  digitalWrite(actionLed, newButtonState);
  // only once per press / release
  if (newButtonState == buttonState)
    return false;

  buttonState = newButtonState;
  
  // Delay a little bit to avoid bouncing
  delay(50);
  return newButtonState == HIGH;
}

// light 0 to value
int drawScale(int value)
{
  if (value == 1)
    return 1;
  return pow(2, value);
}

/****************************************************/
/* environment variables */
/****************************************************/

// time unit
unsigned int t = 0;

// time unit value in ms
long tick;

unsigned long lastTick;

// shape of drawing
unsigned int drawingType = 5;

// show pot until
unsigned long drawScaleUntil;

// state of pot btn
int pot;

/****************************************************/
/* setup */
/****************************************************/
void setup()
{
  // initialize digital pins as an output.
  for (int i = 0; i < num_pins; i++)
  {
    int pin = pinPosition(i);
    pinMode(pin, OUTPUT);
  }

  // action led 
  pinMode(actionLed, OUTPUT);

  // button pin as input
  pinMode(btnPin, INPUT_PULLUP);

  //pot pin as input
  pinMode(potPin, INPUT);

  // get the pot value and set init state:
  pot = getPot();

  // get the corresponding tick value
  tick = getTick();

  // set the first last tick value
  lastTick = millis();
}

/****************************************************/
/* drawing */
/****************************************************/
// pos 0 to 9
int showBit()
{
  int pos = t % num_pins;
  return 1 << pos;
}

// pos 9 to 0 then to 9 => abs((0 to 18) - 9)
int upDownBit()
{
  int maxpos = (num_pins - 1);
  int pos = abs((int(t) % (maxpos * 2)) - maxpos);
  return 1 << pos;
}

// go to center and back
int toCenter()
{
  int pos = t % num_pins;
  int pos2 = (num_pins - 1 - pos);
  return (1 << pos) | (1 << pos2);
}

// go to center and back filled
int toCenterFilled()
{
  int pos = t % num_pins;
  int pos2 = (num_pins - pos);
  return 
    // way in
    (drawScale(pos) & drawScale(pos) << pos2) | 
    // way out
    (drawScale(pos2) & drawScale(pos2) << pos);
}

// 0 to max in binary
int counter()
{
  return (t % int(pow(2, num_pins - 1)));
}

// show blinking pattern
int pattern()
{
  int r = 0;
  for(int i = 0; i < num_pins; i++) {
    if((i + t) % 2 == 0) continue;
    r |= (1 << i);
  }
  return r;
}

// number of drawing functions
const unsigned int num_drawings = 6;

// array of pointer to drawing functions
int (*Drawings[num_drawings])() = {pattern, toCenter, upDownBit, counter, showBit, toCenterFilled };

/****************************************************/
// the loop
/****************************************************/
void loop()
{
  // trigerred when btn change state and is ON 
  if (hasButtonPressed())
  {
    // iterate trhough drawings
    drawingType = (drawingType + 1) % num_drawings;
  }

  unsigned long now = millis();

  int newPot = getPot();

  if (pot != newPot)
  {
    // get the new tick value
    tick = getTick();

    pot = newPot;

    drawScaleUntil = now + 1000;
  }

  // hold if we asked to show scale
  if (now < drawScaleUntil)
  {
    digitalWriteState(drawScale(pot));
  }
  // tick if we passed tick value
  else if (long(now - lastTick) > tick)
  {
    lastTick = now;

    int state = Drawings[drawingType]();;
    digitalWriteState(state);

    // increment time unit:
    // back to 0 as it is unsigned on overflow
    t++;
  }
}
