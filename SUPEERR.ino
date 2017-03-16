#include <LiquidCrystal.h>
#include "EnableInterrupt.h"

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define HIT 8
#define STAY 9
#define RESTART 10

bool hitPressed = false;
bool stayPressed = false;
bool restartPressed = false;

struct cardNode {
	cardNode *next = NULL;
	unsigned char value = 0;
	unsigned char* cardsLeft = NULL;
};

unsigned char cardValue[] = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

cardNode* generateCardDeck();
unsigned char takeRandomCard(cardNode *card);

unsigned char getCardGameValue(unsigned char value);
void destroyCardDeck(cardNode* cardDeck);

void buttonClicked();

void setup() {
  randomSeed(analogRead(0));
    Serial.begin(9600);

    pinMode(HIT, INPUT_PULLUP);
    pinMode(STAY, INPUT_PULLUP);
    pinMode(RESTART, INPUT_PULLUP);

    enableInterrupt(HIT, buttonClicked, CHANGE);
    enableInterrupt(STAY, buttonClicked, CHANGE);
    enableInterrupt(RESTART, buttonClicked, CHANGE);
    
  lcd.begin(16,2);
  lcd.setCursor(0, 0);
  lcd.print("Pelaajan:");
  lcd.setCursor(0, 1);
  lcd.print("Jakajan:");
}

void loop() {
  static cardNode *cardDeck = generateCardDeck();
  static unsigned char playerDeck = 0;
  static unsigned char dealersDeck = 0;
	static unsigned char gameState = 0;

	switch(gameState) {
		case 0:
			playerDeck = getCardGameValue(takeRandomCard(cardDeck));
    	dealersDeck = getCardGameValue(takeRandomCard(cardDeck));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pelaajan:");
      lcd.setCursor(0, 1);
      lcd.print("Jakajan:");
      lcd.setCursor(9, 0);
      lcd.print(playerDeck);
      lcd.setCursor(8, 1);
      lcd.print(dealersDeck);
			gameState = 1;
			break;
		case 1:
      if (hitPressed) {
        playerDeck += getCardGameValue(takeRandomCard(cardDeck));
        lcd.setCursor(9, 0);
        lcd.print(playerDeck);
        if (playerDeck > 21) {
          Serial.println("Jakajavoitti");
          gameState = 2;
        }
        hitPressed = false;
      }
      else if (stayPressed) {
        gameState = 6;
        stayPressed = false;
      }
			break;
		case 2:
      Serial.println("jakaa");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Jakaja voitti!");
      lcd.setCursor(0, 1);
      lcd.print("j:");
      lcd.setCursor(2, 1);
      lcd.print(dealersDeck);
      lcd.setCursor(5, 1);
      lcd.print("p:");
      lcd.setCursor(7, 1);
      lcd.print(playerDeck);
      gameState = 4;
      break;
    case 3: 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pelaaja voitti!");
      lcd.setCursor(0, 1);
      lcd.print("p:");
      lcd.setCursor(2, 1);
      lcd.print(playerDeck);
      lcd.setCursor(5, 1);
      lcd.print("j:");
      lcd.setCursor(7, 1);
      lcd.print(dealersDeck);
      gameState = 4;
			break;
    case 4:
      if (restartPressed) {
        destroyCardDeck(cardDeck);
        cardDeck = generateCardDeck();
        gameState = 0;
        restartPressed = false;
      }
      break;
    case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tasapeli");
      gameState = 4;
      break;
    case 6:
      if (dealersDeck < 17) {
          dealersDeck += getCardGameValue(takeRandomCard(cardDeck));
        } else {
          if (dealersDeck > 21) {
            gameState = 3;
          } else {
            unsigned char playerDiff = 21 - playerDeck;
            unsigned char dealersDiff = 21 - dealersDeck;
            if (playerDiff < dealersDiff) {
              gameState = 3;
            } else if (playerDiff > dealersDiff) {
              gameState = 2;
            } else {
              gameState = 5;
            }
          }
      }
      break;
	}
 
}


cardNode* generateCardDeck() {
	cardNode *firstCard = (cardNode*)malloc(sizeof(cardNode));
	cardNode *currentCard = firstCard;
	unsigned char* cardsLeft = (unsigned char*)malloc(sizeof(unsigned char));
	*cardsLeft = 52;
	for (int i = 0; i < 52; i++) {
		currentCard->value = cardValue[i];
		currentCard->next = (cardNode*)malloc(sizeof(cardNode));
		currentCard->cardsLeft = cardsLeft;
		currentCard = currentCard->next;
	}
	return firstCard;
}

void buttonClicked()
{
    TCNT2 = 0;
    TIMSK2 |= (1 << OCIE2A);
}

void destroyCardDeck(cardNode* cardDeck) {
	cardNode* currentCard = cardDeck;
	unsigned char cardsLeft = *cardDeck->cardsLeft;
	for (int i = 0; i < cardsLeft;i++) {
		cardNode* t = currentCard;
		currentCard = t->next;
		free(t);
	}
}

unsigned char takeRandomCard(cardNode* cardDeck) {
	unsigned char number = random(*cardDeck->cardsLeft);
	if (number == 1) {
		cardNode * next = cardDeck->next;
		char value = cardDeck->value;
		cardDeck->value = cardDeck->next->value;
		cardDeck->next = next->next;	
		free(next);
		*cardDeck->cardsLeft = *cardDeck->cardsLeft - 1;
		return value;
	} else {
		cardNode* currentCard = cardDeck;
		for (int i = 0; i < number - 1; i++) currentCard = currentCard->next;
		cardNode *toBeReturned = currentCard->next;
		if (number < *cardDeck->cardsLeft) {
			currentCard->next = toBeReturned->next;
		}
		char value = toBeReturned->value;
		free(toBeReturned);
		*cardDeck->cardsLeft = *cardDeck->cardsLeft -1;
		return value;
	}
}

unsigned char getCardGameValue(unsigned char value) {
	if (value > 1 && value < 10) return value;
	else if (value > 9 && value < 14) return 10;
	else return 1;
}


ISR(TIMER2_COMPA_vect)
{
    TIMSK2 &= ~(1 << OCIE2A);
    hitPressed = !digitalRead(HIT);
    stayPressed = !digitalRead(STAY);
    restartPressed = !digitalRead(RESTART);
}
