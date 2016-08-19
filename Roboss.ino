//color - signature matches:
//1 - GREEN
//2 - RED
//3 - BLUE
//4 - YELLOW

#include <Pixy.h>
#include <SPI.h>

Pixy pixy;
//signatures
int greenS = 1;
int redS = 2;
int blueS = 3;
int yellowS = 4;

//pins
int red = 6;
int white = 5;
int ledState = LOW; ////RUN ATTACK and ROTATION ATTACK: used to make the led blink

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
bool runEscape = true;

//timers
int fireTimeDelta;
long timeStart;
long lastBlockTime; //last time pixy.getBlocks() was successfull (relative to the attack's parameters)
long lastBlink;
long lastInRangeTime;

//measures
//Pixy x: 0 ~ 319
//Pixy height: 1 ~ 200
int runMaxThresh = 82; //RUN ATTACK: if the pixy cam get an higher value for the height of the shield's box, the player will be considered not in range
int runMinThresh = 47; //RUN ATTACK: if the pixy cam get a lower value for the height of the shield's box, the player will be considered not in range
int rotationMaxThresh = 259; //ROTATION ATTACK: if the pixy cam get an higher value for the x position of the shield's box, the player will be considered not in range
int rotationMinThresh = 60; //ROTATION ATTACK: if the pixy cam get a lower value for the x position of the shield's box, the player will be considered not in range
int blinkDelta = 200; //RUN ATTACK and ROTATION ATTACK: set blink frequency
int runTimeToWin = 8000;
int runTimeToLose = 3000;
int rotationTimeToWin = 8000;
int rotationTimeToLose = 3000;
int numberOfFires = 5; //number of consecutive fires


void GameOver() {
  digitalWrite(red, HIGH);
  digitalWrite(white, HIGH);
  delay(1000);
  digitalWrite(red, LOW);
  digitalWrite(white, LOW);
  delay(1000);
}

void AttackWon() {
  Serial.print("catched \n \n");
  digitalWrite(red, LOW);
  digitalWrite(white, LOW);
  attackWon = false;
  attackEND = false;
  newAttack = true;
  currentAttack = SelectNewAttack(); //select next attack
  delay(4000);
}

attackType SelectNewAttack() {
  if (firesCounter > 0) {
    Serial.print("ancora");
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
  if (newAttack == true) {
    Serial.print("FIRE! \n");
    timeStart = millis();
    color = random(2);
    newAttack = false;
  }
  if (color == 1) {
    digitalWrite(red, HIGH);
    CatchFire(greenS, redS);
  } else {
    digitalWrite(white, HIGH);
    CatchFire(redS, greenS);
  }
  if (millis() - timeStart > 3000) {
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
  if (blocks) {                  //uso white (signature GREEN) ma POI ANDRÀ MODIFICATO!!!!***********
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
    digitalWrite(white, HIGH);
  } else {
    if (millis() - lastBlink > blinkDelta) {
      lastBlink = millis();
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
    }
    digitalWrite(white, ledState);
    if (millis() - lastInRangeTime > runTimeToLose) {
      attackEND = true;
      gameOver = true;
    }
  }
  if (millis() - timeStart > runTimeToWin) {
    attackEND = true;
    attackWon = true; //attackWon and gameOver can be true at the same time but the gameOver condition has the priority
  }
}

//Rotation attack
//AXIS:   ---------> x
//        |
//        |
//        |
//      y v
void Rotation() {
  if (newAttack == true) {
    Serial.print("ROTATION! \n");
    timeStart = millis();
    newAttack = false;
    inRange = false;
    lastBlockTime = millis();
    lastBlink = millis();
    lastInRangeTime = millis();
  }
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
    digitalWrite(white, HIGH);
  } else {
    if (millis() - lastBlink > blinkDelta) {
      lastBlink = millis();
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
    }
    digitalWrite(white, ledState);
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
  Serial.begin(9600); //serial port baud
  Serial.print("Starting...\n");
  pixy.init();
  pinMode(white, OUTPUT);
  pinMode(red, OUTPUT);
  digitalWrite(white, LOW);
  digitalWrite(red, LOW);
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
