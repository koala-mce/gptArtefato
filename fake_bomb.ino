// Code designed for Arduino Uno R3

#include <LiquidCrystal.h>

// Connections to the circuit: LCD screen
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

const int TOTAL_WIRES = 6;
const int CUTTABLE_WIRES[TOTAL_WIRES] = {A8, A9, A10, A11, A12, A13};
const bool WIRES_TO_CUT[TOTAL_WIRES] = { 1, 0, 1, 1, 0, 0 };

const unsigned long TOTAL_TIME = 3600 * 1000L;
//const unsigned long TOTAL_TIME = 30 * 1000L;

unsigned long remainingTime = TOTAL_TIME;
unsigned long lastTimeUpdatedAt = 0;
bool wireStates[TOTAL_WIRES];
byte currentColor = 0;



void formatTime(unsigned long t, char str[13]) {
  unsigned long totalSeconds = t / 1000;
  unsigned long ms = t % 1000;
  unsigned long h = totalSeconds / 3600;
  unsigned long m = (totalSeconds - 3600*h) / 60;
  unsigned long s = totalSeconds - 3600*h - 60*m;
  
  //snprintf(str, 13, "%02lu:%02lu:%02lu.%03lu", h, m, s, ms); // uncomment to display milliseconds
  snprintf(str, 13, "%02lu:%02lu:%02lu", h, m, s);
}

void setup() {
  Serial.begin(9600); // for debug
 

  for (int i=0; i<TOTAL_WIRES; i++) {
    pinMode(CUTTABLE_WIRES[i], INPUT_PULLUP);
    wireStates[i] = LOW; // wires are initially tied to GND
  }



  lcd.clear();

  Serial.println("Ready");
}

int detectWireStateChange() {
  for (int i=0; i<TOTAL_WIRES; i++) {
    int newValue = digitalRead(CUTTABLE_WIRES[i]);
    if (newValue != wireStates[i]) {
      wireStates[i] = newValue;
      return i;
    }
  }
  return -1;
}

void displayCurrentState() {
  lcd.setCursor(0, 1);
  int missingWires = 0;
  for (int i=0; i<TOTAL_WIRES; i++) {
    if (WIRES_TO_CUT[i]) {
      if (wireStates[i]) {
        // Wire was correctly cut
        lcd.print("*");
      } else {
        missingWires++;
      }
    }
  }

  // This is just to erase previously shown asterisks
  for (int i=0; i<missingWires; i++) {
    lcd.print(" ");
  }
}

void displayTimer() {
  char s[13];
  formatTime(remainingTime, s);
  lcd.setCursor(0, 0);
  lcd.print(s);
}

bool isIncorrectWriteCut() {
  for (int i=0; i<TOTAL_WIRES; i++) {
    if (wireStates[i] == 1 && WIRES_TO_CUT[i] == 0) {
      return true;
    }
  }
  return false;
}

bool areAllCorrectWiresCut() {
  for (int i=0; i<TOTAL_WIRES; i++) {
    if (wireStates[i] == 0 && WIRES_TO_CUT[i] == 1) {
      return false;
    }
  }
  return true;
}

void handleWireStateChange(int wireWithNewState) {
  Serial.print("Wire ");
  Serial.print(wireWithNewState);
  if (wireStates[wireWithNewState]) {
    Serial.print(" was cut");
    if (WIRES_TO_CUT[wireWithNewState]) {
      Serial.println(" => correct");
      displayCurrentState();
      
      delay(1000);
    } else {
      Serial.println(" => INCORRECT");
    }
  } else {
    Serial.println(" was reconnected");
  }
  
  if (isIncorrectWriteCut()) {
   
  } else {
  
  }
}

void updateRemainingTime() {
  if (remainingTime == 0) return;
  
  unsigned long now = millis();
  if (lastTimeUpdatedAt == now) return;
  unsigned long elapsedTimeSinceLastUpdate = now - lastTimeUpdatedAt;

  if (isIncorrectWriteCut()) {
    // Time goes down twice as fast if incorrect wire is cut
    elapsedTimeSinceLastUpdate *= 2;
  }

  if (elapsedTimeSinceLastUpdate >= remainingTime) {
    remainingTime = 0;
  } else {
    remainingTime -= elapsedTimeSinceLastUpdate;
  }
  
  lastTimeUpdatedAt = now;
}

void loop() {
  updateRemainingTime();
  
  if (remainingTime == 0) {
    
    lcd.clear();
    lcd.write("*** BOOM ***");
    while (true);
  }

  displayTimer();

  int wireWithNewState = detectWireStateChange();
  if (wireWithNewState >= 0) {
    handleWireStateChange(wireWithNewState);
  }

  displayCurrentState();

  if (areAllCorrectWiresCut() && !isIncorrectWriteCut()) {
    // Win
    
    lcd.clear();
    lcd.write("BOMB DEFUSED");
    lcd.setCursor(0, 1);
    lcd.write("CODE = 750");
    while (true);
  }
}
