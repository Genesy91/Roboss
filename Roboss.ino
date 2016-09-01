//color - signature matches:
//1 - GREEN
//2 - RED
//3 - BLUE
//4 - PINK

#include <SoftwareSerial.h>
#include <Pixy.h>
#include <SPI.h>

Pixy pixy;
SoftwareSerial serialWheels(3, 2); //needed to use wheels (yellow wire, green wire)
//signatures
int greenS = 1;
int redS = 2;
int blueS = 3;
int pinkS = 4;

//pins assigned to the colors of the LED stripe
int blue = 10;
int red = 9;
int green = 8;

int i;
int color;
int firesCounter;

//triggers
enum  attackType {FIRE, RUN, ROTATION};
enum  attackType currentAttack;
bool newAttack = true;
bool gameOver = false;
bool attackWon = false;
bool attackEND = false;
bool inRange = false;
bool rotationClock = true; //ROTATION ATTACK: clockwise
bool runEscape = true; //RUN ATTACK: escape or chase
bool blinkOn = false; //RUN ATTACK and ROTATION ATTACK: used to make the led blink

//timers
int fireTimeDelta;
long timeStart;
long lastBlockTime; //last time pixy.getBlocks() was successfull (relative to the attack's parameters)
long lastBlink;
long lastInRangeTime;

//measures
//Pixy x: 0 ~ 319
//Pixy height: 1 ~ 200
int fireTime = 2000;
int runMaxThresh = 55; //RUN ATTACK: if the pixy cam get an higher value for the height of the shield's box, the player will be considered not in range
int runMinThresh = 28; //RUN ATTACK: if the pixy cam get a lower value for the height of the shield's box, the player will be considered not in range
int rotationMaxThresh = 259; //ROTATION ATTACK: if the pixy cam get an higher value for the x position of the shield's box, the player will be considered not in range
int rotationMinThresh = 60; //ROTATION ATTACK: if the pixy cam get a lower value for the x position of the shield's box, the player will be considered not in range
int blinkDelta = 200; //RUN ATTACK and ROTATION ATTACK: set blink frequency
int runTimeEscape; //RUN ATTACK: in setup set this variable as a fraction of runTimeToWin
int runTimeToWin = 8000;
int runTimeToLose = 3000;
int rotationTimeToWin = 8000;
int rotationTimeToLose = 3000;
int numberOfFires = 4; //number of consecutive fires


//functions to use the LED stripe
void MakeGREEN() {
  digitalWrite(blue, LOW);
  digitalWrite(red, LOW);
  digitalWrite(green, HIGH);
}

void MakeRED() {
  digitalWrite(blue, LOW);
  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
}

void MakeBLUE() {
  digitalWrite(blue, HIGH);
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
}

void MakePINK() {
  digitalWrite(blue, HIGH);
  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
}

void MakeWHITE() {
  digitalWrite(blue, HIGH);
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
}

void LedsOff() {
  digitalWrite(blue, LOW);
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
}

void GameOver() {
  serialWheels.println("r 0 0 0");
  MakeRED();
  delay(1000);
  LedsOff();
  delay(1000);
  Serial.print("\n GAME OVER");
}

void AttackWon() {
  serialWheels.println("r 0 0 0");
  Serial.print("catched \n \n");
  LedsOff();
  attackWon = false;
  attackEND = false;
  newAttack = true;
  currentAttack = SelectNewAttack(); //select next attack
  delay(1000);
}

attackType SelectNewAttack() {
  if (firesCounter > 0) {
    Serial.print("ancora \n");
    --firesCounter;
    return FIRE;
  } else {
    switch (random(3)) {
      case 0: firesCounter = numberOfFires;
        break;
      case 1: return RUN;
        break;
      case 2: return ROTATION;
        break;
    }
    if (firesCounter > 0) {
      firesCounter--;
      return FIRE;

    }
  }
}


//random color fire attack
void Fire() {
  serialWheels.println("r 0 0 0");
  if (newAttack == true) {
    Serial.print("FIRE! \n");
    timeStart = millis();
    color = random(4);
    newAttack = false;
  }
  if (color == 3) {
    MakeGREEN();
    CatchFire(greenS, redS);
  } else if (color == 2) {
    MakeRED();
    CatchFire(redS, greenS);
  } else if (color == 1) {
    MakeBLUE();
    CatchFire(blueS, pinkS);
  } else {
    MakePINK();
    CatchFire(pinkS, blueS);
  }
  if (millis() - timeStart > fireTime) {
    attackEND = true;
    if (attackWon == false) {
      gameOver = true;
    }
  }
}

//check if the player rised the correct shield and hid the other one
void CatchFire(int signature, int oppositeSig) {
  uint16_t blocks;
  blocks = pixy.getBlocks();
  if (blocks) {
    if (blocks == 1) {
      if (pixy.blocks[0].signature == signature) {
        attackWon = true;
      } else attackWon = false;
    }
    if (blocks > 1) {
      bool win = false;
      for (i = 0; i < blocks; i++) {
        if (pixy.blocks[i].signature == signature) {
          win = true;
        }
        if (pixy.blocks[i].signature == oppositeSig) {
          win = false;
          break;
        }
      }
      if (win == true) {
        attackWon = true;
      } else attackWon = false;
    }
  }
}

//Escape or chase attack
//ESCAPE = Roboss escape from the player
//CHASE = Roboss chase the player
void Run() {
  if (newAttack == true) {
    Serial.print("RUN! \n");
    timeStart = millis();
    newAttack = false;
    inRange = false;
    lastBlockTime = millis();
    lastBlink = millis();
    lastInRangeTime = millis();
  }
  uint16_t blocks;
  blocks = pixy.getBlocks();
  if (blocks) {
    inRange = false;
    for (i = 0; i < blocks; i++) {
      if (pixy.blocks[i].signature == greenS && pixy.blocks[i].height > runMinThresh && pixy.blocks[i].height < runMaxThresh) {  //match this number with the front shield signature
        lastBlockTime = millis();
        lastInRangeTime = millis();
        inRange = true;
      }
    }
  } else if (millis() - lastBlockTime > 100) {
    inRange = false;
  }
  if (inRange) {
    MakeWHITE();
  } else {
    if (millis() - lastBlink > blinkDelta) {
      lastBlink = millis();
      if (blinkOn == false) {
        blinkOn = true;
      } else {
        blinkOn = false;
      }
    }
    if (blinkOn == true) {
      MakeWHITE();
    } else LedsOff();
    if (millis() - lastInRangeTime > runTimeToLose) {
      attackEND = true;
      gameOver = true;
    }
  }
  if (millis() - timeStart > runTimeToWin) {
    attackEND = true;
    attackWon = true; //attackWon and gameOver can be true at the same time but the gameOver condition has the priority
  }
  if (millis() - timeStart < runTimeEscape) {
    serialWheels.println("r 0.4 0 0");
  } else serialWheels.println("r -0.4 0 0");
}

//Rotation attack
//AXIS:   ---------> x
//        |
//        |
//        |
//      y v
void Rotation() {
  if (newAttack == true) {
    if (rotationClock == true) {
      rotationClock = false;
    } else rotationClock = true;
    Serial.print("ROTATION! \n");
    timeStart = millis();
    newAttack = false;
    inRange = false;
    lastBlockTime = millis();
    lastBlink = millis();
    lastInRangeTime = millis();
  }
  if (rotationClock == true) {
    serialWheels.println("r 0 0 -0.95");
  } else serialWheels.println("r 0 0 0.95");
  uint16_t blocks;
  blocks = pixy.getBlocks();
  if (blocks) {                  //uso white (signature GREEN) ma POI ANDRÀ MODIFICATO!!!!***********
    inRange = false;
    for (i = 0; i < blocks; i++) {
      if (pixy.blocks[i].signature == greenS && pixy.blocks[i].x > rotationMinThresh && pixy.blocks[i].x < rotationMaxThresh) { //it defines the field of view of the robot
        lastBlockTime = millis();
        lastInRangeTime = millis();
        inRange = true;
      }
    }
  } else if (millis() - lastBlockTime > 100) {
    inRange = false;
  }
  if (inRange) {
    MakeWHITE();
  } else {
    if (millis() - lastBlink > blinkDelta) {
      lastBlink = millis();
      if (blinkOn == false) {
        blinkOn = true;
      } else {
        blinkOn = false;
      }
    }
    if (blinkOn == true) {
      MakeWHITE();
    } else LedsOff();
    if (millis() - lastInRangeTime > rotationTimeToLose) {
      attackEND = true;
      gameOver = true;
    }
  }
  if (millis() - timeStart > rotationTimeToWin) {
    attackEND = true;
    attackWon = true; //attackWon and gameOver can be true at the same time but the gameOver condition has the priority
  }
}


void setup() {
  // put your setup code here, to run once:
  serialWheels.begin(115200);
  Serial.begin(9600); //serial port baud
  Serial.print("Starting...\n");
  serialWheels.println("r 0 0 0");
  pixy.init();
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  runTimeEscape = runTimeToWin / 2;
  MakeGREEN();
  delay(700);
  MakePINK();
  delay(700);
  MakeBLUE();
  delay(700);
  MakeRED();
  delay(700);
  LedsOff();
  delay(1000);
  //**************************************************
  //COMMENT ONE OF THE 2 OPTIONS
  //**************************************************
  //firesCounter = 0; //start with a random attack
  firesCounter = numberOfFires; //always start with Fire
  //**************************************************
  //**************************************************
  currentAttack = SelectNewAttack(); //select the first attack
}

void loop() {
  // put your main code here, to run repeatedly:
  if (gameOver == true) {
    GameOver();
  } else {
    switch (currentAttack) {
      case FIRE: Fire();
        break;
      case RUN: Run();
        break;
      case ROTATION: Rotation();
        break;
    }
    //*******************************
    //CODICE NECESSARIO PER TESTARE SINGOLARMENTE GLI ATTACCHI
    //*******************************
    //Fire();
    //Run();
    //Rotation();
    //*******************************
    //*******************************
    if (attackEND == true && attackWon == true) {
      AttackWon();
    }
  }
}
