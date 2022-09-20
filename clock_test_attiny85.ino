#define LOAD 4            // MAX LOAD Pin connected to ATtiny pin 4 
#define CLK 2
#define DIN 0
#define MISO 1
#define chipEnablePin 3   // The DS1306 CE (chip enable) pin is connected to ATtiny pin 3.
#define button 5        // Button that sets time

byte minute;
byte tenMin;
byte hour;
byte tenHour;

void initialise()
{
  pinMode(LOAD, OUTPUT);
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);

  // Set the Chip enable pin as OUTPUT.
  pinMode(chipEnablePin, OUTPUT);
  digitalWrite(chipEnablePin, LOW);
}

void setTime()    // Should be 18:00
{
  digitalWrite(chipEnablePin, HIGH);                // sets clock hours 000Hhhhh remember to make it Bit
  shiftOut(DIN, CLK, MSBFIRST, 0x82);
  shiftOut(DIN, CLK, MSBFIRST, B00100010);          // B
  digitalWrite(chipEnablePin, LOW);

  digitalWrite(chipEnablePin, HIGH);                // sets clock minutes 0MMMmmmm
  shiftOut(DIN, CLK, MSBFIRST, 0x81);
  shiftOut(DIN, CLK, MSBFIRST, B01000111);          // B
  digitalWrite(chipEnablePin, LOW);

  digitalWrite(chipEnablePin, HIGH);                // sets clock Seconds 0SSSssss
  shiftOut(DIN, CLK, MSBFIRST, 0x80);
  shiftOut(DIN, CLK, MSBFIRST, B00000000);          // B
  digitalWrite(chipEnablePin, LOW);

}

void output(byte address, byte data)
{
  digitalWrite(LOAD, LOW);
  shiftOut(DIN, CLK, MSBFIRST, address);
  shiftOut(DIN, CLK, MSBFIRST, data);
  digitalWrite(LOAD, HIGH);
}

void setup() {

  // Init ATTINY85 Pins
  initialise();                                       // sets all required pins to output mode

  // Init DS1306
  digitalWrite(chipEnablePin, HIGH);
  shiftOut(DIN, CLK, MSBFIRST, 0x8f);                 // Address the Control Register
  shiftOut(DIN, CLK, MSBFIRST, B00000100);            // Clear write protect and turn on 1HZ signal
  digitalWrite(chipEnablePin, LOW);                   //
  /*
    // RAM reset
    digitalWrite(chipEnablePin, HIGH);                  //
    shiftOut(DIN, CLK, MSBFIRST, 0xA0);
    shiftOut(DIN, CLK, MSBFIRST, B00000000);            // Write RAM
    digitalWrite(chipEnablePin, LOW);
  */

  digitalWrite(chipEnablePin, HIGH);                  // READ USER RAM
  shiftOut(DIN, CLK, MSBFIRST, 0x20);                 // Address the USER RAM
  byte userRAM = shiftIn(MISO, CLK, MSBFIRST);        // READ USER RAM
  digitalWrite(chipEnablePin, LOW);

  // Init MAX7219
  output(0x0f, 0x00);    // exit display test mode
  output(0x0c, 0x01);    // enter normal operation mode
  output(0x0b, 0x03);    // set number of displays
  output(0x0a, 0x04);    // set display brightness
  output(0x09, 0x0f);    // set into decode mode

  // Displays RAM content
  for (byte i = B00000000; i < 4; i++) {
    output((4 - i), ((userRAM & ( B00000001 << i)) >> i) );
  }
  delay(5000);
  /*
    if (userRAM == B00000000) {                       // Check if RAM address is UNSET

    // Sets clock to 24hr mode
    digitalWrite(chipEnablePin, HIGH);
    shiftOut(DIN, CLK, MSBFIRST, 0x82);
    shiftOut(DIN, CLK, MSBFIRST, B00000000);
    digitalWrite(chipEnablePin, LOW);

    setTime();                                        // Sets the intial time

    delay(100);
    digitalWrite(chipEnablePin, HIGH);                 // Set RAM Bit to avoid re-initialization
    shiftOut(DIN, CLK, MSBFIRST, 0xA0);
    shiftOut(DIN, CLK, MSBFIRST, B00000001);          // Write RAM
    digitalWrite(chipEnablePin, LOW);
    }
  */
}

void loop() {                                         // Updates time
  byte minuteReadAddress = 0x01;
  byte hourReadAddress = 0x02;
  byte clockData = 0x00; // uhhh whats this

  digitalWrite(chipEnablePin, HIGH);
  shiftOut(DIN, CLK, MSBFIRST, minuteReadAddress);    // Address the Minute register
  byte minuteByte = shiftIn(MISO, CLK, MSBFIRST);     // Read the data into the 'minuteByte' variable
  tenMin = minuteByte >> 4;                           // Shift bits to the right 4 times remember to times by 10 so it becomes a 10 minute value
  minute = minuteByte & B00001111;                    // This is a bitMask

  digitalWrite(chipEnablePin, LOW);

  output(0x03, tenMin);
  output(0x04, minute);

  digitalWrite(chipEnablePin, HIGH);
  shiftOut(DIN, CLK, MSBFIRST, hourReadAddress);
  byte hourByte = shiftIn(MISO, CLK, MSBFIRST);
  tenHour = hourByte & B00110000;
  tenHour = tenHour >> 4;

  hour = hourByte & B00001111;

  digitalWrite(chipEnablePin, LOW);

  output(0x01, tenHour);
  output(0x02, hour);

  delay(1000);    // Should use interrupt or some other means
}

/*
  tenMin = (minuteByte >> 4) * 10;
  minute = minuteByte & B00001111;

*/
