//color - signature matches:
//1 - GREEN
//2 - RED
//3 - BLUE
//4 - YELLOW

#include <Pixy.h>
#include <SPI.h>

Pixy pixy;
int red = 6;
int white = 5;
int i;
int color;
long timeStart;

//triggers
bool newAttack = true;
bool gameOver = false;
bool attackWon = false;
bool attackEND = false;

//timers
int fireTimeDelta;

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
  delay(4000);
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
    CatchFire(1, 2);
  } else {
    digitalWrite(white, HIGH);
    CatchFire(2, 1);
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //serial port baud
  Serial.print("Starting...\n");
  pixy.init();
  pinMode(white, OUTPUT);
  pinMode(red, OUTPUT);
  digitalWrite(white, LOW);
  digitalWrite(red, LOW);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (gameOver == true) {
    GameOver();
  } else {
    Fire();
    if (attackEND == true && attackWon == true) {
      AttackWon();
    }
  }
}
