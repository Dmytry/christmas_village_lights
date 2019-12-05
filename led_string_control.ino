#include <EEPROM.h>
#include <FastLED.h>
#define NUM_LEDS 50
#define DATA_PIN 11

CRGB leds[NUM_LEDS];

uint16_t clamp_add(uint16_t a, int8_t b){
  uint16_t result=a+b;
  if(b>=0){
    if(result<a)result=a;
  }else{
    if(result>a)result=0;
  }
  return result; 
}

struct NoiseGen{
  uint16_t v;
  int8_t d;
  void update(int speed, int speed2=1){
    d=d+random(speed*2+1)-speed;    
    if(v>65535-127)v=65535-128; else
    if(d==-128)d=-127;
    v=clamp_add(v, d*speed2);
  }
};

struct Fire{
  NoiseGen noise0;
  NoiseGen noise1;
  uint8_t get(){
    noise0.update(10,4);
    noise1.update(10,4);
    return (noise0.v>>9)+(noise1.v>>9);
  }
};

Fire fires[NUM_LEDS];
CRGB light_color={255, 100, 25};

// house light on/off
int16_t lights[(NUM_LEDS+15)/16];// bit set if the light is off

uint16_t inv_toggle_p=5000;

void read_from_eeprom(){
  for(int i=0; i<NUM_LEDS; ++i)
    for(int j=0; j<3; ++j)
    {
      leds[i][j]=EEPROM.read(i*3+j);
    }
}

int test_button(int pin){  
  int i=0;
  uint8_t port = digitalPinToPort(pin); // get the pin's port
  uint8_t bitmask = digitalPinToBitMask(pin);
  volatile uint8_t *ddr = portModeRegister(port); // get the ddr 
  volatile uint8_t *pinIn = portInputRegister(port); // port input register
  volatile uint8_t *pinOut = portOutputRegister(port);
  *ddr  |= bitmask;
  *pinOut |= bitmask;
  
  delay(1);
  noInterrupts();  
  //pinMode(pin, INPUT);
  //set mode to input
  *ddr &= ~bitmask;
  // disable pullup
  *pinOut &= ~bitmask;    
  /*while(digitalRead(pin)!=LOW && i<10000){
    i++;
  }*/
  while((bitmask & *pinIn) && i<1000)i++;
  interrupts();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  return i;
}
    
void setup() {
  read_from_eeprom();  
  // put your setup code here, to run once:
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS); 
  //Serial.begin(9600); 
}

void interact(){
  
}

uint8_t frac_mul(uint8_t a, uint8_t b){
  return (uint16_t(a)*b)>>8;
}

uint8_t clamp_add(uint8_t a, uint8_t b){
  uint8_t result=a+b;
  if(result<a)return 255;
  return result;
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t mask=1;
  for(int i = 0 ; i < NUM_LEDS; i++ ) {
    
    
    uint8_t f=fires[i].get()>>1;
    uint8_t f2=(uint16_t(f)*uint16_t(f))>>8;
    uint8_t f3=(uint16_t(f2)*f)>>8;
    
    leds[i].r = f;
    leds[i].g = f2;
    leds[i].b = f3;

    if(random(inv_toggle_p)==0){
      lights[i/16]^=mask;
    }

    if(lights[i/16]&mask)
    {
      for(int j=0;j<3;++j)leds[i][j]=clamp_add(leds[i][j], light_color[j]);
    }
    
    
    //leds[i].r = frac_mul(f, fire_color.r);
    //leds[i].g = frac_mul(f2, fire_color.g);
    //leds[i].b = frac_mul(f3, fire_color.b);
    
    //leds[i].r = (uint8_t(f)*fire_color.r)>>8;
    //leds[i].g = (uint8_t(f)*fire_color.g)>>8;
    //leds[i].b = (uint8_t(f)*fire_color.b)>>8;
    //delay(10);    
    //Serial.println(test_button(2));
    mask<<=1;
    if(mask==0)mask=1;
  }
  FastLED.show();
  
}
