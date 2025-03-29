#ifndef pong_H_ 
#define pong_H_

    void BoostTon(){
             for(x=2000 ; x <= 3000;x=x+100) //speed ton
        {
          tone(6, x);
          tone(7, x); 
          delay(5);
        }
        noTone(6);
        noTone(7);
}
byte playerscored = 0;
unsigned long scoretimer = 0;

struct ball{
    int8_t size = BALL_SIZE_STILL;
    int16_t speed = BASE_SPEED;
    int position = (NUM_LEDS_FIELD-1)*100/2; // Anzahl der Leds mit 100 zwischenschritten
    int16_t boost = 0;
    bool blinking = false;

    void setpos(int pos){
      position = pos*100;
      speed = 0;
      size = BALL_SIZE_STILL;
    }

    void reset(){
      position = (NUM_LEDS_FIELD-1)*100/2;
      speed = 0;
      size = BALL_SIZE_STILL;
      playerscored = 0;
    }
  
    void update(){
      position += speed;
      size = 1; //round((BALL_SIZE_MAX) * abs(((float)speed/(BALL_MAX_SPEED))));
  
      if(speed < -BALL_MAX_SPEED){
        speed = -BALL_MAX_SPEED;
      }else if(speed > BALL_MAX_SPEED){
        speed = BALL_MAX_SPEED;
      }
    }
  
    void draw(CRGB *leds){
      uint16_t pos = position/100;
      pos = constrain(pos,0,NUM_LEDS_FIELD-1);

      if(!blinking){   
      leds[pos] = COL_BALL;
  
         
      if(speed == 0 && ((NUM_LEDS_FIELD & 1) == 0) && position == (NUM_LEDS_FIELD-1)*100/2){
        leds[pos+1] = COL_BALL;
      }/*else{
        if(speed <= 0){
          for (byte i = 1; i < size; i++) {
            byte trailPos = pos + i;
            if (trailPos <= NUM_LEDS_FIELD-1) {
              leds[trailPos] = COL_BALL;
              leds[i].maximizeBrightness(255/i);
            }
          }
        }else{
          for (byte i = 1; i < size; i++) {
            byte trailPos = pos - i;
            if (trailPos >= 0) {
              leds[trailPos] = COL_BALL;
              leds[i].maximizeBrightness(255/i);
            }
          }
        }
      }*/
    }
    }
  };
  
ball theball;
unsigned long timer_t = 0; // timer variable für Boost Hold

void drawfield(){
  for(int i = 0; i < BOOST_ZONE; i++) {
    leds_field[i] = CRGB::Red;
    //leds_field[i].maximizeBrightness(BRIGHTNESS*BRIGHTNESS_FIELD_FACTOR);
  }
  for(int i = BOOST_ZONE; i < BOOST_ZONE+HIT_ZONE; i++) {
    leds_field[i] = COL_HIT_ZONE;
    leds_field[i].maximizeBrightness(BRIGHTNESS*BRIGHTNESS_FIELD_FACTOR);
  }
  for(int i = BOOST_ZONE+HIT_ZONE; i < BOOST_ZONE+HIT_ZONE+WARN_ZONE; i++) {
    leds_field[i] = COL_WARN_ZONE;
    leds_field[i].maximizeBrightness(BRIGHTNESS*BRIGHTNESS_FIELD_FACTOR);
  }

  for(int i = NUM_LEDS_FIELD-1; i > NUM_LEDS_FIELD-1-BOOST_ZONE; i--) {
    leds_field[i] = CRGB::Red;
    //leds_field[i].maximizeBrightness(BRIGHTNESS*BRIGHTNESS_FIELD_FACTOR);
  }
  for(int i = NUM_LEDS_FIELD-1-BOOST_ZONE; i > NUM_LEDS_FIELD-1-BOOST_ZONE-HIT_ZONE; i--){
    leds_field[i] = COL_HIT_ZONE;
    leds_field[i].maximizeBrightness(BRIGHTNESS*BRIGHTNESS_FIELD_FACTOR);
  }
  for(int i = NUM_LEDS_FIELD-1-BOOST_ZONE-HIT_ZONE; i > NUM_LEDS_FIELD-1-BOOST_ZONE-HIT_ZONE-WARN_ZONE; i--){
    leds_field[i] = COL_WARN_ZONE;
    leds_field[i].maximizeBrightness(BRIGHTNESS*BRIGHTNESS_FIELD_FACTOR);
  }
}



void updateGame(unsigned long t){


  if(playerscored){
    if((t % 10) == 0){
      theball.blinking = !theball.blinking;
    }
    if(t - scoretimer == 60){
      if(playerscored == 1){
        player1.score++;
        //theball.setpos(NUM_LEDS_FIELD-(HIT_ZONE+BOOST_ZONE+1));
      }else if(playerscored == 2){
        player2.score++;
        //theball.setpos(HIT_ZONE+BOOST_ZONE);
      }
      tone(PIN_BUZZER_P1, 1000, 100);
      delay(100);
    }
    if(t - scoretimer == 80){
      tone(PIN_BUZZER_P1, 500, 200);
      delay(200);
      if(playerscored == 1){
        
        theball.setpos(NUM_LEDS_FIELD-(HIT_ZONE+BOOST_ZONE+1));
      }else if(playerscored == 2){
      
        theball.setpos(HIT_ZONE+BOOST_ZONE);
      }
      playerscored = 0;
      theball.blinking = false;
    }
  }else{

    if (t - timer_t > BOOST_HOLD && theball.boost != 0){ // nach ablauf des Boost Hold geschw. zurücksetzten zum vohrigen zustand und tasten aktivieren
      theball.speed = theball.boost;
      theball.boost = 0;
      player1.active = true;
      player1.active = true;
    }

    if (theball.position >= (NUM_LEDS_FIELD)*100){ // P2 zuspät
      Serial.println("P2 Missed Pos: ");Serial.println(theball.position);  
      playerscored = 1;scoretimer = t;
      theball.setpos(NUM_LEDS_FIELD-(HIT_ZONE+BOOST_ZONE+1));
      tone(PIN_BUZZER_P2, 200, 50); //Pung 
      delay(50);
      player1.active = false;player2.active = true;    
    }else if(theball.position < 0){ // P1 zuspät
      Serial.println("P1 Missed Pos: ");Serial.println(theball.position);     
      playerscored = 2;scoretimer = t;
      theball.setpos(HIT_ZONE+BOOST_ZONE);
      tone(PIN_BUZZER_P1, 200, 50);//Pung246
      delay(50);
      player2.active = false;player1.active = true;
      
    }

    if(player1.button_pressed && playerscored == 0){ 
      if(theball.speed < 0){                                // Wenn P1 gedrückt hat prüfen ob der Ball sich auch auf P1 zubewegt
        if(theball.position <= (BOOST_ZONE+HIT_ZONE+1)*100){   // In der Hitzone
          theball.speed = theball.speed *-1 + BASE_FORCE;   // Ball umkehren und beschleunigen
          tone(PIN_BUZZER_P1, 246, 50);//Pong
          delay(50);
          if(theball.position < BOOST_ZONE*100){            // Wenn zusätlich in der Boostzone , timer setzten aktuelle geschw. + boost zwischen speichern in boost var, ball stoppen und spieler taste deaktivieren
            timer_t = t; 
            theball.boost = theball.speed + BOOST_FORCE;
            BoostTon();
            theball.speed = 0;
            //player1.active = false;
          }
        }else{                                              // P1 zufrüh gedrückt 
          Serial.println("P1 Early Pos: ");Serial.println(theball.position);
          theball.speed = 0;playerscored = 2;scoretimer = t;
          
          tone(PIN_BUZZER_P1, 200, 50);//Pung 246
          delay(50);
          player2.active = false;player1.active = true;         // P1 deaktiveren um aufschlag von p2 zu ermöglichen
          
        }
      }else if(theball.speed == 0 && player1.active){           // aufschlag P1
        theball.speed = BASE_SPEED;
        player2.active = true;
      }
    }
    
    
    if(player2.button_pressed && playerscored == 0){     // Analog zu P1, mit umkehrter geschwingkeit (Richtung) sowie Grenzen 
      if(theball.speed > 0){
        if(theball.position >= (NUM_LEDS_FIELD-HIT_ZONE-BOOST_ZONE-1)*100){
          theball.speed = theball.speed *-1 - BASE_FORCE;
          tone(PIN_BUZZER_P2, 491, 50);//Ping 491
          delay(50);
          if(theball.position >= (NUM_LEDS_FIELD-BOOST_ZONE)*100){
            timer_t = t; 
            theball.boost = theball.speed - BOOST_FORCE;
            BoostTon();
            theball.speed = 0;
            //player2.active = false;
          }
        }else{
          Serial.println("P2 Early Pos: ");Serial.println(theball.position);
          theball.speed = 0;
          playerscored = 1;scoretimer = t;
          
          tone(PIN_BUZZER_P2, 200, 50);//Pung
          delay(50);
          player1.active = false;
          player2.active = true;
          
        }
      }else if(theball.speed == 0 && player2.active){
        theball.speed = -BASE_SPEED;
        player1.active = true;
      }
    }
  }

  player1.button_pressed = false; //Inputs zurücksetzten
  player2.button_pressed = false;
  
}

#endif