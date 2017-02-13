#include <Wire.h>

//Config directives
#define A_INT 0  // Pro mini pin 2
#define A_PIN_NUM 2
#define Z_INT 1  // Pro mini pin 3
#define Z_PIN_NUM 3
#define B_PIN_NUM 9
#define B_PIN (PINB & _BV(1))  //Pro mini pin 9
#define TICKS_PER_REV 1024
#define IIC_ADDRESS 6

//Do not modify
#define USEC_PER_MIN (60E6)
#include <util/atomic.h>

volatile int16_t vcount;
int16_t count;
float rpm;
volatile float vrpm;
unsigned long l_micros, d_micros;

void isr_a(){
  if(B_PIN){
    vcount++;
  } else {
    vcount--;
  }
}
void isr_iic(){
  //Wire.write();
}

void setup() {
  pinMode(A_PIN_NUM,INPUT);
  pinMode(B_PIN_NUM,INPUT);
  digitalWrite(B_PIN_NUM, HIGH);
  pinMode(Z_PIN_NUM,INPUT);
  vcount=0;
  vrpm=0;
  attachInterrupt(A_INT, isr_a, RISING);
  l_micros=micros();
  //Wire.begin(IIC_ADDRESS);
  //Wire.onRequest(isr_iic);
  Serial.begin(57600);
}

void loop() {
 delay(100);
 ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
 {
  count = vcount;
  vcount = 0;
 }
 d_micros = micros() - l_micros;
 l_micros = micros();
 rpm = count;
 rpm /= TICKS_PER_REV;
 rpm /= d_micros;
 rpm *= USEC_PER_MIN; 
 
 ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
 {
  vrpm = rpm;
 }
 Serial.println(rpm);
}
