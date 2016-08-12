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
uint8_t blocks;
int i;

//random color fire attack
void Fire() {
  if (random(2) == 1) {
    Serial.print("red fire \n");
    digitalWrite(red, HIGH);
    delay(3000); //time for the player to rise the shield
    CatchFire(1, 2);
    digitalWrite(red, LOW);
    delay(3000);
  } else {
    Serial.print("white fire \n");
    digitalWrite(white, HIGH);
    delay(3000);
    CatchFire(2, 1);
    digitalWrite(white, LOW);
    delay(3000);
  }
}

//check if the player rised the correct shield and hid the other one
void CatchFire(int signature, int oppositeSig) {
  blocks = pixy.getBlocks();
  if (blocks) {
    if (blocks == 1) {
      Serial.print("un blocco");
      if (pixy.blocks[0].signature == signature) {
        Serial.print("WIN \n \n");
      } else Serial.print("LOSE \n \n");
    }
    if (blocks > 1) {
      Serial.print("più blocchi");
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
        Serial.print("WIN \n \n");
      } else Serial.print("LOSE \n \n");
    }
  } else Serial.print("LOSE no blocchi \n \n");
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
  Fire();

}
