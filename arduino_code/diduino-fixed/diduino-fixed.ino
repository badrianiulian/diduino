//---Radionews---https://github.com/Radionews
//---Walhi---https://github.com/walhi
//---04.12.2023---

#define ST A3
#define SH A2
#define DS A1
#define AR0 2
#define AR1 3
#define AR2 4
#define READ 5
#define CS 6
#define WRITE 7
#define DS_POWER 8
#define DS_DATA 9

#define BUF_LEN 8
#define LED 13
/* Voltage control (for programming chips) */
#define voltageControl A4
#define rTop 10040.0      // ohm value for R8 accurately measured!! used for internal Voltmeter calibration
#define rBottom 2392.0    // ohm value for R7 accurately measured!! used for internal Voltmeter calibration


#include <OneWire.h>
OneWire ds(DS_DATA);

typedef enum chipType {
  NONE = 0,
  RE3 = 1,
  RT4 = 2,
  RT14 = 3,
  RT5 = 4
} Chip;

typedef enum mode {
  WAIT,
  Read,
  Write,
  VOLTAGE,
  TEMPERATURE,
  SET
} Modes;

//---prototipes---
void init_power5959();
void set_power(uint8_t level);
void select_chip (chipType new_chip);
void cs_set(chipType chip, bool state);
float get_voltage (void);
void load_shift();
void read_byte(uint16_t in);
void write_byte(uint16_t addr, byte in);

//---var---
unsigned char data_array[512];
unsigned char uart_read_temp = 0;

byte power = 0;
word shift_data = 0;
chipType chip = NONE;
Modes mode = WAIT;
uint8_t log_enable = false;
uint16_t start_address = 0x0000;
uint16_t end_address = 0x0000;

uint8_t buf[BUF_LEN];

uint16_t number_of_impulses = 1000;
uint16_t length_of_impulse = 40;
uint8_t duty = 5;
uint8_t count = 0, r_data = 0;
byte out = 0;
byte temp = 0;
uint16_t i = 0, j = 0;

void setup() {
  //initializing the boost converter shift register
  init_power5959();
  
  pinMode(ST, OUTPUT);
  pinMode(SH, OUTPUT);
  pinMode(DS, OUTPUT);
  pinMode(WRITE, OUTPUT);
  digitalWrite(WRITE,LOW);
  pinMode(CS, OUTPUT);
  digitalWrite(CS,HIGH);
  pinMode(DS_POWER, OUTPUT);
  digitalWrite(DS_POWER,HIGH);
  pinMode(READ, INPUT);
  pinMode(AR0, OUTPUT);
  pinMode(AR1, OUTPUT);
  pinMode(AR2, OUTPUT);
  digitalWrite(AR0,LOW);
  digitalWrite(AR1,LOW);
  digitalWrite(AR2,LOW);

  pinMode(LED, OUTPUT);
  digitalWrite(LED,LOW);
  
  Serial.begin(115200);
  Serial.println("ARDUINO PROGRAMMER DIDUINO VER1.0");

}

void loop() {
  switch (mode) {
    case Read:
      if (chip == NONE) {
        mode = WAIT;
        break;
      }
      for (i = start_address; i <= end_address; i++) {
        read_byte(i);
        Serial.write(&out, sizeof(out));
        if (i == end_address) break; // uint16 overflow protection
      }
      delay(100);
      mode = WAIT;
      break;
      
    case Write:
      if (chip == NONE) {
        mode = WAIT;
        break;
      }
      for (i = start_address; i <= end_address; i++) {
        while(Serial.available()==0);
        r_data = Serial.read();
        //count++;
        write_byte(i, r_data);
        Serial.write(&out, sizeof(out));
        if (i == end_address) break;
      }
      Serial.print("Write success.");
      delay(100);
      mode = WAIT;
      break;
      
    case VOLTAGE:
      Serial.print("VOLTAGE: ");
      Serial.println(get_voltage(), 2);
      mode = WAIT;
      break;

    case TEMPERATURE:
      //digitalWrite(DS_POWER,LOW); //leave DS12B20 always powered due to the 1s delay probe
      delay(100);
      //digitalWrite(DS_POWER,HIGH); //leave DS12B20 always powered due to the 1s delay probe
      delay(100);
      Serial.print("Temperature: ");
      byte data[2];
      ds.reset();
      ds.write(0xCC);
      ds.write(0x44);
      delay(100); //DS12B20 needs at least 1s delay probe but will retain last read temperature
      ds.reset();
      ds.write(0xCC); 
      ds.write(0xBE);

      data[0] = ds.read(); //Reading the low byte of the temperature value
      data[1] = ds.read(); //And now the eldest
      //digitalWrite(DS_POWER,LOW); //leave DS12B20 always powered due to the 1s delay probe
      Serial.println(((data[1] << 8) | data[0]) * 0.0625);
      mode = WAIT;
      break;

    case SET:
      if (chip == NONE) {
        mode = WAIT;
        break;
      }
      while (Serial.available()!=2);
      temp = Serial.read();
      number_of_impulses = temp;
      temp = Serial.read();
      number_of_impulses |= (temp<<8);

      while (Serial.available()!=2);
      temp = Serial.read();
      length_of_impulse = temp;
      temp = Serial.read();
      length_of_impulse |= (temp<<8);

      while (Serial.available()!=1);
      temp = Serial.read();
      duty = temp;
      
      while (Serial.available()!=1);
      temp = Serial.read();
      set_power(temp);
      delay(2000);
      
      mode = WAIT;
      break;
      
    default:
      if (chip == NONE) Serial.println("Chip not selected!");
      Serial.println("Wait commands...");
      while (Serial.available()) Serial.read();
      do {} while (Serial.available() == 0);
      char incomingByte = Serial.read();
      while (Serial.available()) Serial.read();
      switch (incomingByte) {
        case 'r': mode = Read; break;
        case 'w': mode = Write; break;
        case 'v': mode = VOLTAGE; break;
        case 't': mode = TEMPERATURE; break;
        case 'g': select_chip(RE3); break;
        case 'h': select_chip(RT4); break;
        case 'i': select_chip(RT14); break;
        case 'j': select_chip(RT5); break;
        case 'o': mode = SET; break;
      }
  }
}

void select_chip (chipType new_chip) {
  switch (new_chip) {
    case RE3:
      chip = new_chip;
      end_address = 0x001f;
      break;
    case RT4:
      chip = new_chip;
      end_address = 0x00ff;
      break;
    case RT14:
      chip = new_chip;
      end_address = 0x07cf;
      break;
    case RT5:
      chip = new_chip;
      end_address = 0x01ff;
      break;
    default:
      chip = NONE;
      end_address = 0x0000;
  }
}
void init_power5959(){
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(A0, OUTPUT);
  digitalWrite(A0,HIGH);  
}

//voltage setting
void set_power(uint8_t level){
  if(level>7) level = 7;
  if(level==0)digitalWrite(A0,HIGH);
  else{
    digitalWrite(11, LOW);
   // shiftOut(10, 12, MSBFIRST, ~(1<<(8-level))); 
    shiftOut(10, 12, MSBFIRST, ~(1<<(7-level)));
    digitalWrite(11, HIGH);
    digitalWrite(A0,LOW);
  }
}

//loading data into shift registers
void load_shift(){
  digitalWrite(SH, LOW);
  shiftOut(DS, ST, LSBFIRST, (shift_data>>8)&0xFF);
  shiftOut(DS, ST, LSBFIRST, shift_data&0xFF);   
  digitalWrite(SH, HIGH);
}

void cs_set(chipType chip, bool state){
  switch(chip){
    case NONE:
    break;
    case RE3:
      digitalWrite(CS,state);
    break;
    case RT4:
      digitalWrite(CS,state);
    break;
    case RT14:
      digitalWrite(CS,state);
    break;
    case RT5:
      if(state){
        digitalWrite(CS,HIGH);
        shift_data |= (1<<12)|(1<<11);
        shift_data &= ~(1<<10);
      }
      else{
        digitalWrite(CS,LOW);
        shift_data |= (1<<10);
        shift_data &= ~((1<<12)|(1<<11));
      }
      
    break; 
  }
}

void read_byte(uint16_t in){
  digitalWrite(WRITE,LOW);
  
  shift_data = in;
  cs_set(chip,true);
  load_shift();

  out = 0;
  temp = 0;
  for(j=0;j<8;j++){
    if((j&0x01)==1)     {digitalWrite(AR0,HIGH);}
    else                {digitalWrite(AR0,LOW);}

    if(((j>>1)&0x01)==1){digitalWrite(AR1,HIGH);}
    else                {digitalWrite(AR1,LOW);}

    if(((j>>2)&0x01)==1){digitalWrite(AR2,HIGH);}
    else                {digitalWrite(AR2,LOW);}

    delay(1);
    digitalWrite(LED, !digitalRead(LED));
    
    if(((chip==RT4)||(chip == RT14))&&(j>3)){
      return;
    }
    
    temp = !digitalRead(READ);
    out |= temp<<j;    
  }
}

void write_byte(uint16_t addr, byte in){
  //just in case, before manipulating, we lower the recording leg, you never know if it was raised by something
   digitalWrite(WRITE,LOW);

  // set the cell address
  shift_data = addr;
  //cs line(s) to read
  cs_set(chip,true);
  load_shift();
  
  out = 0;
  temp = 0;
  for(j=0;j<8;j++){
    //set the bit in the cell
    if((j&0x01)==1)     digitalWrite(AR0,HIGH);
    else                digitalWrite(AR0,LOW);
    if(((j>>1)&0x01)==1)digitalWrite(AR1,HIGH);
    else                digitalWrite(AR1,LOW);
    if(((j>>2)&0x01)==1)digitalWrite(AR2,HIGH);
    else                digitalWrite(AR2,LOW);
    delay(1);
    digitalWrite(LED, !digitalRead(LED));
    if(((chip==RT4)||(chip == RT14))&&(j>3)){
      return;
    }

    if((((in>>j)&0x01)>(!digitalRead(READ))) && chip!= RT5){
             
      for(int16_t k=0;k<number_of_impulses;k++){
        cs_set(chip,false);
        load_shift();
        digitalWrite(WRITE,HIGH);
        if(length_of_impulse<1000){delayMicroseconds(length_of_impulse);}
        else                      {delay(length_of_impulse/1000);}
        digitalWrite(WRITE,LOW);
        cs_set(chip,true);
        load_shift();
        if((length_of_impulse*(100/duty))<1000){delayMicroseconds(length_of_impulse*(100/duty));}
        else                          {delay(length_of_impulse*(100/duty)/1000);}

        if(((in>>j)&0x01) == (!digitalRead(READ))) {break;}
      }
    }
      
    if((((in>>j)&0x01)<(!digitalRead(READ))) && chip == RT5){
      
      for(int16_t k=0;k<number_of_impulses;k++){
        cs_set(chip,false);
        load_shift();
        digitalWrite(WRITE,HIGH);
        if(length_of_impulse<1000){delayMicroseconds(length_of_impulse);}
        else                      {delay(length_of_impulse/1000);}
        digitalWrite(WRITE,LOW);
        cs_set(chip,true);
        load_shift();
        if((length_of_impulse*(100/duty))<1000){delayMicroseconds(length_of_impulse*(100/duty));}
        else                          {delay(length_of_impulse*(100/duty)/1000);}

        if(((in>>j)&0x01) == (!digitalRead(READ))) {break;}
      } 
    }
    
    temp = !digitalRead(READ);
    out |= temp<<j;
  }
}

float get_voltage (void) {
  float vADC = (analogRead(voltageControl) / 1024.) * 4.95;  //// !ATTENTION! for internal Voltmeter calibration - use Aref voltage (4.95)
  float current = vADC / rBottom;
  return (current * (rTop + rBottom));
}
