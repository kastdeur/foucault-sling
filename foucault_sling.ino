/*
Foucault Sling (IR Trigger)
*/

const int irPin = 2;

const unsigned long irTriggerMinPeriod = 100UL; // milliseconds
const unsigned long irTriggerMaxPeriod = 1500UL; // milliseconds

const unsigned long irTriggerShortInterval = 3*1000UL;// 50;// milliseconds, between up and down swing
const unsigned long irTriggerRepeatInterval = 8*1000UL; // 7200;// milliseconds
const unsigned long irTriggerIntervalLeeway = 2000UL; // 200;// milliseconds

const unsigned long irTriggerSetStaleInterval = 60*1000UL; // 12 hours

const unsigned int irTriggerSetCountRequired = 3L; // required consecutive triggers
const unsigned int irTriggetSetInvalidateCount = 3L;

const unsigned long lampTimeSetInterval = 10*1000UL; // 10 seconds

int irState =  HIGH; // It is pulled low on trigger
int prevIrState;

unsigned long lampTime = 0; // zero when sling is overhead
unsigned long curMills = 0;
unsigned long lampTimeSetMills = 0; // only set the lampTime once every lampTimeSetInterval

// timestamps for triggering
unsigned long irCurrentTriggerStart = 0;
unsigned long irPrevTriggerStart = 0;
unsigned long irTriggerSetStart = 0;
unsigned long irTriggerSetLast = 0;
int irTriggerSetCount = 0;
int irTriggerSetCountFinal = 0;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(irPin, INPUT);

  Serial.begin(9600);

  while(!Serial) {
    ;
  }

  Serial.println("Initialise");
}

void loop() {
  prevIrState = irState;

  // new loop
  curMills = millis();
  irState = digitalRead(irPin);


  // Trigger on IR
  if ( irState != prevIrState )
  {
    // reset trigger counters if we miss a few swings
    if ( irTriggerSetCount != 0 )
    {
      if (irPrevTriggerStart != 0 && curMills - irTriggerSetLast > irTriggetSetInvalidateCount*irTriggerRepeatInterval )
      {
        irTriggerSetCount = 0;
        irTriggerSetCountFinal = irTriggerSetCount;

        Serial.println("Resetting TriggerSet");
      }
    }
    
    // falling edge, new potential trigger period
    if ( irState == LOW ) 
    {
      Serial.println("Starting Trigger Period");
      irCurrentTriggerStart = curMills;
    }
    // rising edge, see if we want to trigger
    else {
      Serial.println("--");
      Serial.print("Triggered Period: ");
      Serial.print(curMills - irCurrentTriggerStart);
      Serial.println("ms");

      Serial.print("Last Trigger: ");
      Serial.print(curMills - irPrevTriggerStart);
      Serial.println("ms");
      
      if (curMills - irCurrentTriggerStart >= irTriggerMinPeriod
              && curMills - irCurrentTriggerStart <= irTriggerMaxPeriod
      ) {
        // this seems a valid trigger period length
        Serial.print("Potential Trigger: ");

        // in shortinterval window?
        if ( curMills - irPrevTriggerStart < irTriggerShortInterval ) {
          // keep the previous triggerstart
          irPrevTriggerStart = irPrevTriggerStart;
          Serial.print("Short Interval");
        }
        // within repeatInterval from the previous trigger?
        else if (curMills - irPrevTriggerStart >= irTriggerRepeatInterval - irTriggerIntervalLeeway
                && curMills - irPrevTriggerStart <= irTriggerRepeatInterval + irTriggerIntervalLeeway
        ) {
          Serial.print("Repeat Interval");

          // second trigger of a set?
          if ( curMills -  irTriggerSetLast >= irTriggerSetStaleInterval )
          {
            Serial.println(" (First of Set)");

            Serial.print("Last Trigger in previous Set: ");
            Serial.print(irTriggerSetLast);
            Serial.println("ms");

            Serial.print("ago: ");
            Serial.println(curMills - irTriggerSetLast);
            Serial.print("Stale after: ");
            Serial.println(irTriggerSetStaleInterval);

            irTriggerSetStart = irPrevTriggerStart;
            irTriggerSetCount = 0;
            irTriggerSetCountFinal = 0;

            Serial.print("Set Start: ");
            Serial.print(irTriggerSetStart);
            Serial.println("ms");
          }

          irTriggerSetCount += 1;
          irTriggerSetLast = irCurrentTriggerStart;
          Serial.println();
          Serial.print("TriggerSet Count: ");
          Serial.print(irTriggerSetCount);
        }
        else{
          Serial.print("wait for repeating trigger in ");
          Serial.print(irTriggerRepeatInterval);
          Serial.print(" +- ");
          Serial.print(irTriggerIntervalLeeway);
          Serial.print("ms");
        }

        Serial.println("");
        irPrevTriggerStart = irCurrentTriggerStart;
      }
    }
  }

  // Reset the lampTime
  if ( irTriggerSetCountFinal >= irTriggerSetCountRequired || irTriggerSetCount >= irTriggerSetCountRequired )
  { // set lampTime even if we are not yet finalised

    // only set lampTime if the set is not stale yet
    if ( curMills - lampTimeSetMills > lampTimeSetInterval )
    {
      int thisCount = irTriggerSetCountFinal;
      if ( irTriggerSetCount >= irTriggerSetCountRequired )
      {
        thisCount = irTriggerSetCount;
      }

      lampTime = irTriggerSetStart + thisCount * irTriggerRepeatInterval / 2;

      lampTimeSetMills = curMills;

      Serial.print("Setting lampTime: ");
      Serial.println(lampTime);
      Serial.println(curMills - lampTime);
      Serial.println(lampTimeSetInterval);
    }
  }

  // set correct lamps..
  if ( lampTime != 0 && irTriggerSetLast != 0)
  {
    digitalWrite(LED_BUILTIN, HIGH );
  }
}