#include <SPI.h>

#define MISOPIN 36 //17
#define SCLKPIN 35
#define CLKPIN 37 //20

byte byte1; byte byte2; byte byte3;
byte flipper = 0b11111111; // 8 bit number to flip bits of a byte
byte addone = 0b1; // to add one to the two's complement

int check = byte3 << 1;
double ads_output =0;

// calculation formulas:
float f_ADSCLK = 10000000;// 10 MHz

float f_MCLK = f_ADSCLK / 6; 
float DRATE = f_MCLK / 64;

float p_MCLK = 1 / f_MCLK;
float  DRDY_partition = 36 * p_MCLK;
float DOUT_partition = 348 * p_MCLK;
float CONVCYCLE = DRDY_partition + DOUT_partition; // = 384 * p_MCLK;
float t_RESET_5 = 5 * CONVCYCLE; 

void setup(){
 Serial.begin(115200);
 
 pinMode(SCLKPIN, OUTPUT);
 pinMode(MISOPIN, INPUT);
 pinMode(CLKPIN, OUTPUT);
 
 // put ADC on reset at the outset
 reset_adc();

 
 // SPI.begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS
 SPI.begin(SCLKPIN, MISOPIN, 19, CLKPIN );
 // initialize SPI (with default settings, including...
 // CPOL = 0: so that SCLK is normally LOW
 // CPHA = 0: data sampled on rising edge (LOW to HIGH)
 // perhaps try changing CPHA ??
 
 digitalWrite(SCLKPIN, LOW);
 // release ADC from reset; now we're at a known point
 // in the timing diagram, and just have to wait for
 // the beginning of a conversion cycle
Serial.println("setup completed");
}

void loop(){
 Serial.println("start loop");

 if (digitalRead(MISOPIN) == HIGH) 
  Serial.println("PIN HIGH");
  
 read_adc();

 delay(1000);
}


void reset_adc()
// to reset ADC, we need SCLK HIGH for min of 4 CONVCYCLES
// so here, hold SCLK HIGH for 5 CONVCYCLEs = 1440 usec
{
 digitalWrite(SCLKPIN, HIGH);
 delay(t_RESET_5);
}

void read_adc(){
  drdy_wait();
  // go to drdy_wait routine, where we wait for
  // DRDY phase to pass, and thus for DOUT phase to begin
  SPI.beginTransaction(SPISettings(f_ADSCLK, MSBFIRST, SPI_MODE0));
  byte3 = SPI.transfer(0x00);
  byte2 = (byte3 << 8) | SPI.transfer(0x00);
  byte1 = (byte2 << 8) | SPI.transfer(0x00);
    
  SPI.endTransaction();
  // read in adc data (sending out don't care bytes)
  // and store read data into three bytes */

  Serial.println(byte1, DEC);
  Serial.println(byte2, DEC);
  Serial.println(byte3, DEC);
  Serial.println();
 
 
  if (check == 1){
    // Negative number in two's complement form. need to flip bytes. and add one to them.
    //    negative = "-";
    // flip the bits
    
    byte3 = byte3^flipper;
    byte2 = byte2^flipper;
    byte1 = byte1^flipper;
}else{
    // negative = "+";
    // no need to flip or add one
  addone = 0b0;
}
        
ads_output = 5*(((byte3 << 16) + (byte2 << 8) + (byte1 + addone))) >> 24; 

Serial.println(ads_output);

//cout << " two's Complement : " <<  5*(((byte3 << 16) + (byte2 << 8) + (byte1 + addone))/pow(2,24)) << endl;

 
 // print out data;
 // will these instructions eat into time significantly?
 // possible improvement: store all data from multiple cycles
 // into array, and print out only later at end.
}

// wait for DRDY to pass and to reach start-point of DOUT
void drdy_wait(){
 delay(DRDY_partition);
}
