//PongMax v 27.03.25
//#include <Arduino.h>
#include <FastLED.h>
#include "score.h"

// LED Configurations
#define LED_TYPE WS2812B
#define NUM_LEDS_FIELD 42
#define NUM_LEDS_P1 15
#define NUM_LEDS_P2 15
#define MAX_mA  500
#define BRIGHTNESS 150 //MAX Helligkeit
int x;
int y;
#define TICK_RATE 120 //Hz

// PIN Defintions
#define PinGlocke 13
#define PIN_LED_FIELD 8
#define PIN_LED_P1 9
#define PIN_LED_P2 10
#define PIN_BUTTON_P1 3
#define PIN_BUTTON_P2 2
#define PIN_BUZZER_P1 7//7
#define PIN_BUZZER_P2 6

#define DEBOUNCETIME 50

#define IDLETIME 60 //s
#define GAMEOVER_TIME 10 //s

#define HIT_ZONE 4
#define BOOST_ZONE 1
#define WARN_ZONE 1
#define BRIGHTNESS_FIELD_FACTOR 0.50 //Hit- und Warnzone abdunkeln

#define BASE_SPEED 8  // Start Geschwingkeit
#define BASE_FORCE 4  // erhöhung bei hit
#define BOOST_FORCE 10 //zusätzlich in bei Boost
#define BOOST_HOLD  0//ticks

#define BALL_MAX_SPEED 80 // MAX 100 um das überspringen von LEDS zu vermeiden, da 100 zwischenschritte in der bewegung
#define BALL_SIZE_STILL 1

#define MAX_SCORE 10

#define COL_BALL        CRGB::White
#define COL_HIT_ZONE    CRGB::Green
#define COL_WARN_ZONE 0xffff00

#define COL_P1          CRGB::Red
#define COL_P2          CRGB::Red

struct player{
  byte buzzer;
  CRGB *score_leds;
  CRGB color;

  player(byte pin_buzzer,CRGB *score_led_array,CRGB player_color){
    buzzer = pin_buzzer;
    score_leds = score_led_array;
    color = player_color;
  }

  void reset(){
    score = 0;
    lastpressed = millis();
    button_pressed = 0;
    active = true;
  }

  byte score = 0;
  volatile bool button_pressed = 0;
  volatile unsigned long lastpressed = 0;
  bool active = true;
};

enum GameState {
  IDLE = 0,    
  COUNTDOWN = 1,     
  PLAYING = 2,
  GAME_OVER = 3   
};

CRGB leds_field[NUM_LEDS_FIELD];
CRGB leds_score_P1[NUM_LEDS_P1];
CRGB leds_score_P2[NUM_LEDS_P2];

GameState gamestate = IDLE;

unsigned long currenttime = millis();


unsigned long lastTick = 0; //in ms

unsigned long lastInput_Tick = 0;
unsigned long ticks = 0;
unsigned long startTick_timer = 0;

player player1(PIN_BUZZER_P1,leds_score_P1,COL_P1);
player player2(PIN_BUZZER_P2,leds_score_P2,COL_P2);
player* winner = nullptr;

#include "pong.h"

void glocke(){
digitalWrite(PinGlocke,LOW);
delay(50);
digitalWrite(PinGlocke,HIGH);

}

void gewinn1(){
             for (y=1 ; y<4; y++){
          for (x=2500 ; x<3000; x=x+10){
                tone(PIN_BUZZER_P1, x);
                delay(10);
          } 
          noTone(PIN_BUZZER_P1);
         delay(100);
        }
        noTone(6);
}
void gewinn2(){
            for (y=1 ; y<4; y++){
          for (x=2500 ; x<3000; x=x+10){
                tone(PIN_BUZZER_P2, x);
                delay(10);
          } 
          noTone(PIN_BUZZER_P2);
         delay(100);
        }
        noTone(PIN_BUZZER_P2);
}
void Player1_Int() {
  unsigned long now = millis();
  
  if (now - player1.lastpressed > DEBOUNCETIME) {
    player1.lastpressed = now;
    lastInput_Tick = ticks;
    player1.button_pressed = true;
//  player1.pressCount++;
  }
}

void Player2_Int() {
  unsigned long now = millis();
  
  if (now - player2.lastpressed > DEBOUNCETIME) {
    player2.lastpressed = now;
    lastInput_Tick = ticks;
    player2.button_pressed = true;
//  player2.pressCount++;
  }
}

void setup() {

  FastLED.addLeds<LED_TYPE, PIN_LED_FIELD, RGB>(leds_field, NUM_LEDS_FIELD);
  FastLED.addLeds<LED_TYPE, PIN_LED_P1, RGB>(leds_score_P1, NUM_LEDS_P1);
  FastLED.addLeds<LED_TYPE, PIN_LED_P2, RGB>(leds_score_P2, NUM_LEDS_P2);

  FastLED.setMaxPowerInVoltsAndMilliamps(5,MAX_mA);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(PIN_BUTTON_P1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_P2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_P1), Player1_Int, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_P2), Player2_Int, FALLING);

  Serial.begin(115200);
  digitalWrite(PinGlocke,HIGH);

}

void clearLeds(CRGB* leds, int numLeds) {
  for(int i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Black;
  }
}

void checkScore(){
  if(player1.score >= MAX_SCORE ){
    winner = &player1;
    Serial.println("Player 1 Won !");
    gewinn1();//sound
    gamestate = GAME_OVER;
  }else if(player2.score >= MAX_SCORE){
    winner = &player2;
    Serial.println("Player 2 Won !");
    gewinn2();//sound
    gamestate = GAME_OVER;
  }
}

void winninganimation(unsigned long t, bool dir, uint16_t l){
  static int16_t pos = 0;

  if(t % 5 == 0){
    pos += (dir) ? -1 : 1; 
   // pos = pos % NUM_LEDS_FIELD;
  }

  for(uint16_t i = 0; i < l/2; i++) {
    leds_field[(i+pos)%NUM_LEDS_FIELD] = winner->color;
  }
  for(uint16_t i = 0; i < l/2; i++) {
    leds_field[(i+pos+l)%NUM_LEDS_FIELD] = winner->color;
  }

}

void showscore(player* player){
  int i = 0;
  switch (player->score) {
    case 0:
      for (i = 0; i < sizeof(N0); i++) {
        player->score_leds[N0[i]] = player->color;
      }
      break;
    case 1:
      for (i = 0; i < sizeof(N1); i++) {
        player->score_leds[N1[i]] = player->color;
      }
      break;
    case 2:
      for (i = 0; i < sizeof(N2); i++) {
        player->score_leds[N2[i]] = player->color;
      }
      break;
    case 3:
      for (i = 0; i < sizeof(N3); i++) {
        player->score_leds[N3[i]] = player->color;
      }
      break;
    case 4:
      for (i = 0; i < sizeof(N4); i++) {
        player->score_leds[N4[i]] = player->color;
      }
      break;
    case 5:
      for (i = 0; i < sizeof(N5); i++) {
        player->score_leds[N5[i]] = player->color;
      }
      break;
    case 6:
      for (i = 0; i < sizeof(N6); i++) {
        player->score_leds[N6[i]] = player->color;
      }
      break;
    case 7:
      for (i = 0; i < sizeof(N7); i++) {
        player->score_leds[N7[i]] = player->color;
      }
      break;
    case 8:
      for (i = 0; i < sizeof(N8); i++) {
        player->score_leds[N8[i]] = player->color; 
      }
      break;
    case 9:
      for (i = 0; i < sizeof(N9); i++) {
        player->score_leds[N9[i]] = player->color;
      }
      break;
  }
}

void loop(){

  unsigned long currentTime = millis();

  if(currentTime - lastTick >= 1000/TICK_RATE){ 
    lastTick = currentTime; 

    clearLeds(leds_field, NUM_LEDS_FIELD);
    clearLeds(leds_score_P1, NUM_LEDS_P1);
    clearLeds(leds_score_P2, NUM_LEDS_P2);

    switch (gamestate){
      case IDLE:
      
        drawfield();
        theball.draw(leds_field);

        //Spiel Simulieren
        theball.update();
        if(theball.position >= (NUM_LEDS_FIELD-HIT_ZONE-BOOST_ZONE+1)*100 || theball.position < (BOOST_ZONE+HIT_ZONE-1)*100){
          theball.speed *= -1;
        }
  

        if(player1.button_pressed || player2.button_pressed){   
          gamestate = COUNTDOWN;
          tone(PIN_BUZZER_P1, 3000, 100);
          delay (200);
          tone(PIN_BUZZER_P2, 3000, 100);
          delay (200);
          theball.reset();
          player1.reset();
          player2.reset();
        }

      break;
      case COUNTDOWN:

        if (startTick_timer == 0) {
          startTick_timer = ticks;
        }

        if(ticks - startTick_timer >= 1 * TICK_RATE){    // 1 Sekunde bis Ball gezeigt wird    
          gamestate = PLAYING;
          startTick_timer = 0;
          player1.reset();
          player2.reset(); 
          

        }

        drawfield();

      break;
      case PLAYING:

        // Kehre zum IDLE State zurück nach IDLETIME Sekunden
        if( (ticks - lastInput_Tick) >= IDLETIME * TICK_RATE){
          gamestate = IDLE;
          theball.speed = BASE_SPEED;
        }
 
        updateGame(ticks); //Inputs und Spielelogik auswerten

        theball.update(); // Ball bewegen
        checkScore();     // überprüfen ob es eine Gewinner gibt ->  geht in Gameoverstate

        drawfield();      // Spielfeldzonen zeichen
        theball.draw(leds_field); 

        //Score anzeigen
        showscore(&player1);
        showscore(&player2);

      break;  
      case GAME_OVER:

        if (startTick_timer == 0) {
          startTick_timer = ticks;
        }

        if ( abs(player1.score - player2.score) == MAX_SCORE){ //überprüft ob es ein MAX_SCORE zu 0 gibt, Glockenfunktion einbauen vlt andere sieger animationen benutzen
          Serial.println ("Haushoher Sieg");
           glocke();
        }

        winninganimation(ticks, (player2.score == MAX_SCORE) ? true : false, 10); // zeigt gewinner animation auf Led Streifen in passender farbe und Laufrichtung
        showscore(&player1);
        showscore(&player2);
        
        

        // Nach GAMEOVER_TIME kehre zurück zu IDLE
        if(ticks - startTick_timer >= GAMEOVER_TIME * TICK_RATE){
          gamestate = IDLE;
          startTick_timer = 0;
          Serial.println("Gameover->IDLE");
          theball.speed = BASE_SPEED;
        }

      break;
    }
    ticks++;
    FastLED.show(); // Zeige alle LEDS
  }

  
}