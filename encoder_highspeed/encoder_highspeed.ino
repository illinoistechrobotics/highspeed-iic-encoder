 /* High speed quadrature encoder reader
 *  
 *  Note: For >50kHz operation, a 74xx373 latch is required
 *  with the A output of the encoder wired to the latch gate
 *  and the B output of the encoder connected through the latch.
 *  
 *  Strong (500-1k ohm) pull up resistors are also required
 *  on the A and B signals for reliable operation at high speed
 *  in noisy environments with E6B2- series encoders since they do NOT
 *  drive the outputs bidirectionally and have a very weak internal pullup.
 *  
 *  Compilation with the -O2 or -O3 flag is also advisable.
 */
#include <Wire.h>

//Config directives
#define A_INT 0  // Pro mini pin 2
#define A_PIN_NUM 2
#define Z_INT 1  // Pro mini pin 3
#define Z_PIN_NUM 3
#define B_PIN_NUM 9
#define B_PIN (PINB & _BV(1))  //Pro mini pin 9
#define TICKS_PER_REV 1024
#define IIC_ADDRESS 5

//Do not modify
#define USEC_PER_MIN (60E6)
#include <util/atomic.h>

volatile int32_t vcount;
volatile int32_t vcount_since_last_read;
int32_t count;
int32_t count_since_last_read;
float rpm;
volatile float vrpm;
unsigned long l_micros, d_micros;

void isr_a(){
  if(B_PIN){
    vcount++;
    vcount_since_last_read++;
  } else {
    vcount--;
    vcount_since_last_read--;
  }
}
void isr_iic(){
  for(int i = sizeof(double)-1; i>=0; i--){
   Wire.write(*(((volatile uint8_t *)(&vrpm)) + i));
  }
  // for Mike's project, send accumulator of ticks since last read
  for(int i = sizeof(int32_t)-1; i>=0; i--){
   Wire.write(*(((volatile uint8_t *)(&vcount_since_last_read)) + i));
  }
  vcount_since_last_read = 0;
}

void setup() {
  pinMode(A_PIN_NUM,INPUT);
  pinMode(B_PIN_NUM,INPUT);
  pinMode(Z_PIN_NUM,INPUT);
  vcount=0;
  vrpm=0;
  vcount_since_last_read=0;
  attachInterrupt(A_INT, isr_a, FALLING);
  l_micros=micros();
  Wire.begin(IIC_ADDRESS);
  Wire.onRequest(isr_iic);
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
