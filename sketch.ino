#include <TimerOne.h>

const byte leds[] = {9, 10, 11, 12}; //definisane su lampice
const byte buttons[] = {3, 4, 5, 6}; //definisani su tasteri

#define ERROR_LED 13 //definisana je crvena lampica za greske
#define MAX_GAME_LENGTH 100 //definisan je maksimalni broj sekvenci
#define RETRY_BTN 2 //definisan je taster za reset

byte gameSequence[MAX_GAME_LENGTH] = {0}; //definisana je sekvenca
byte gameIndex = 0; //definisan je trenutni nivo
int errorCounter = 0; //definisan je broj napravljenih gresaka

void setup() {
  Serial.begin(9600);
  for (byte i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);  //LED lampice su definisane kao izlazne komponente
    pinMode(buttons[i], INPUT_PULLUP); //tasteri su definisani kao ulazne komponente koji
  }                                    //sadrze interne otpornike

  attachInterrupt(digitalPinToInterrupt(RETRY_BTN), retry, FALLING); //eksterni interrupt kada se pritisne 
                                                                    //RETRY taster
  Timer1.initialize(600000);
  Timer1.attachInterrupt(timerInterrupt);  //vremenski interrupt kada korisnik pogresi zadatu sekvencu

  pinMode(ERROR_LED, OUTPUT); //crvena led lampica koja je definisana kao izlazna komponenta

  randomSeed(analogRead(A0)); //uzima random seed vrednost za generisanje nasumicnog broja
}

//funkcija za vremenski interrupt koji proverava da ako je doslo do greske da se crvena lampica
//u odredjenom intervalu ukljucuje i iskljucuje sve dok korisnik ne pogodi sekvencu
void timerInterrupt() {
  if(errorCounter > 0 && errorCounter < 3) {
    digitalWrite(ERROR_LED, HIGH);
    delay(200);
    digitalWrite(ERROR_LED, LOW);
  }
}

//eksterni interrupt koji prvo proverava da li je crvena LED lampica ukljucena kada se zavrsila igra,
//ako jeste, iskljucuje lampicu, resetuje nivo i broj gresaka na nula cime se pokrece nova igra
void retry(){
  if (digitalRead(ERROR_LED) == HIGH) {
    gameIndex = 0;
    errorCounter = 0;
    digitalWrite(ERROR_LED, LOW);
    delay(300);
    Serial.println("Game starts again.. ");
  }
}

//funkcija koja ukljucuje i iskljucuje lampice iz niza prema odgovarjucem indeksu.
void lightLed(byte index) {
  digitalWrite(leds[index], HIGH);
  delay(200);
  digitalWrite(leds[index], LOW);
}

//funkcija koja ispisuje trenutnu sekvencu koju korisnik treba da ponovi. Povecava se za jedan svaki
//put kad korisnik pogodi sekvencu.
void playSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte currentLed = gameSequence[i];
    lightLed(currentLed);
    delay(100);
  }
}

//funkcija koja ispisuje indekse pritisnutih tastera
byte readButtons() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      byte button = buttons[i];
      if (digitalRead(button) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}

//funkcija koja se poziva kada je korisnik napravi tri greske tokom pogadjanja sekvence,
//prikazuje rezultat, osvojene poene i ukljucuje crvenu LED lampicu.
void gameOver() {
  Serial.print("Game over! your score is: ");
  Serial.println(gameIndex - 2);
  digitalWrite(ERROR_LED, HIGH);
}

//funkcija koja poziva funkciju readButtons() da bi procitala inpute korisnika i uporedila ih sa
//trazenom sekvencom. Uporedjuje taster po taster.
bool checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLed(actualButton); //ukljuci LED lampicu pritisnutog tastera
    if (expectedButton != actualButton) {
      return false;
    }
  }
  return true;
}

//funkcija koja se poziva kada korisnik pogodi sekvencu i omogucava prelazak u naredni nivo
//resetovanjem broja gresaka na nula
void levelUp() {
  Serial.println("You Passed the level!");
  errorCounter = 0;
  delay(300);
}

//while petlja u kojoj se nalazi masina stanja koja proverava sva moguca stanja errorCounter promenljive
void loop() {
  switch(errorCounter){
    case 0:
      if (gameIndex == 0) { //ako je pocetni nivo, generisi sekvencu od dva nasumicna elementa
        gameSequence[gameIndex] = random(0, 4);
        gameIndex++;
        gameSequence[gameIndex] = random(0, 4);
        gameIndex++;
      }
      else { //generisi svaki naredni nasumicni element u sekvenci
        gameSequence[gameIndex] = random(0, 4);
        gameIndex++;
      }
      playSequence(); //pozivanje funkcije ispisa trenutne sekvence

      if (!checkUserSequence()) { //ako je korisnik napravio gresku u pogadjanju sekvence, povecaj
        errorCounter++;           //errorCounter promenljivu za jedan.
        Serial.println("Mistake number: 1");
      }
      else {
        levelUp(); //pozivanje funkcije za uspesno pogodjenu sekvencu
      }  
      break; 

    case 1:
      if (!checkUserSequence()) {
        errorCounter++;
        Serial.println("Mistake number: 2");
      }
      else {
        levelUp();
      } 
      break; 

    case 2:
      if (!checkUserSequence()) {
        errorCounter++;
        Serial.println("Mistake number: 3");
      }
      else{
        levelUp();
      }  
      break;

    case 3:
      if(digitalRead(ERROR_LED) == LOW){
          gameOver(); //ukoliko korisnik napravi tri greske, poziva se funkcija gameOver()
      }
      break;
  }

  //Ako je gameIndex dostigao vrednost 100, igra se zavrsava.
  if (gameIndex >= MAX_GAME_LENGTH) {
    gameIndex = 0;
    Serial.println("You Won!");
  }
}
