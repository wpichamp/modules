#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial WEPOSSerial(8, 9);  // for interfacing with the EPOS4 motor controller - RX, TX

#define BUSBUFFSIZE 8
#define EPOSBUFFSIZE 14

/* start of EPOS command constants */
#define CTRL_MOVEREL 0x007F   // payload for the relative movement
#define CTRL_MOVEABS 0x0001F  
#define CTRL_ENABLE 0x000F    //
#define CTRL_RESET 0x0006     //

#define DLE 0x90  // part one of the sync part of the communication frame
#define STX 0x02  // part two of the sync part of the communication frame
/* end of EPOS command constants */

union longArray
{
  byte myBytes[4];
  long myLong;
};

/* For setting the mode of the MAX485 */
#define TX HIGH
#define RX LOW

byte my_id;

int re_PIN = 10; // recieve enable pin
int de_PIN = 11; // drive enable pin

int button_PIN = 4;

int led0_PIN = 3;
int led1_PIN = 5;
int led2_PIN = 6;

int debug_led_PIN = 13;

void setup() 
{
  Serial.begin(9600);
  WEPOSSerial.begin(9600);
  updateControlWord(WEPOSSerial, CTRL_RESET);
  updateControlWord(WEPOSSerial, CTRL_ENABLE);
  
  pinMode(re_PIN, OUTPUT);
  pinMode(de_PIN, OUTPUT);
  
  pinMode(button_PIN, INPUT);
  
  pinMode(led0_PIN, OUTPUT);
  pinMode(led1_PIN, OUTPUT);
  pinMode(led2_PIN, OUTPUT);

  pinMode(debug_led_PIN, OUTPUT); 

  my_id = EEPROM.read(0); // read the ID stored in eeprom
    
  setMode(RX);
}

byte prefex0;
byte prefex1;
byte prefex2;
byte prefex3;
byte prefex4;
byte payload;
byte depth;
byte incomingChecksum;

bool newMessage = false;

void loop()
{
  if (newMessage)
  {    
    switch(prefex0)
    {
      case 0:
        switch(prefex1)
        {
          case 0:
            analogWrite(led0_PIN, payload);
            updatePositionDemmand(WEPOSSerial, payload * 100);
            updateControlWord(WEPOSSerial, CTRL_MOVEABS);  
            updateControlWord(WEPOSSerial, CTRL_ENABLE);
            break;
          case 1:
            analogWrite(led1_PIN, payload);
            break;
          case 2:
            analogWrite(led2_PIN, payload);
            break;
        }
        break;
    }
    newMessage = false;
  }
}

void serialEvent() {
  
  if (Serial.available() >= BUSBUFFSIZE)
  {    
    byte bus_buff[BUSBUFFSIZE];
    
    Serial.readBytes(bus_buff, BUSBUFFSIZE);
    
    prefex0 = bus_buff[0];
    prefex1 = bus_buff[1];
    prefex2 = bus_buff[2];
    prefex3 = bus_buff[3];
    prefex4 = bus_buff[4];
    payload = bus_buff[5];
    depth = bus_buff[6];
    
    incomingChecksum = bus_buff[7];
    
    byte calculatedChecksum = sum(bus_buff, 7);
    
    int goodChecksum = (calculatedChecksum == incomingChecksum);

    if (goodChecksum)
    {
      digitalWrite(debug_led_PIN, LOW);
      newMessage = true;
    } 
    else
    {
      digitalWrite(debug_led_PIN, HIGH);
      delay(10);
      clearBuffer(); // give up on the current transmission
      newMessage = false;
    }
  }
  else
  {
    newMessage = false;
  }
}

byte sum(byte incomingArray[], int arraySize)
{
  byte sum = 0;
  for (int index = 0; index < arraySize; index++)
  {
    sum += incomingArray[index];
  }
  return sum;
}

void setMode(boolean state)
{
  digitalWrite(re_PIN, state);
  digitalWrite(de_PIN, state);
}

void clearBuffer()
{
  while (Serial.available() > 0)
  {
    Serial.read();
  }
}

void writeBytes(byte tx_bytes[], int num_bytes)
{
  for (int index = 0; index < num_bytes; index++)
  {
    Serial.write(tx_bytes[index]);
  }
  Serial.flush();
}

/* start of EPOS functions */
void updatePositionDemmand(SoftwareSerial &port, long newValue)
{  
  byte Len = 0x04;
  byte OpCode = 0x68;

  byte NodeID = 0x01;
  word ObjectIndex = 0x607A;
  byte SubIndex = 0x00;

  longArray la;
  la.myLong = newValue;
  
  word DataArray[6];

  DataArray[0] = word(Len, OpCode);   // len and opcode
  DataArray[1] = word(lowByte(ObjectIndex), NodeID);
  DataArray[2] = word(0x00, highByte(ObjectIndex));
  DataArray[3] = word(la.myBytes[1], la.myBytes[0]); 
  DataArray[4] = word(la.myBytes[3], la.myBytes[2]);
  DataArray[5] = word(0x00, 0x00);    // Zero word
  
  word CRC = CalcFieldCRC(DataArray, 6);

  byte buff[EPOSBUFFSIZE];

  /* SYNC */ 
  buff[0] = DLE; // DLE
  buff[1] = STX; // STX

  /* HEADER */
  buff[2] = OpCode; // OpCode
  buff[3] = Len; // Len

  /* DATA */ 
  buff[4] = NodeID;                 // LowByte data[0], Node ID
  buff[5] = lowByte(ObjectIndex);   // HighByte data[0], LowByte Index
  buff[6] = highByte(ObjectIndex);  // LowByte data[1], HighByte Index
  buff[7] = SubIndex;               // HighByte data[1], SubIndex
  buff[8] = la.myBytes[0];
  buff[9] = la.myBytes[1];
  buff[10] = la.myBytes[2];
  buff[11] = la.myBytes[3];

  /* CRC */ 
  buff[12] = lowByte(CRC); // CRC Low Byte
  buff[13] = highByte(CRC); // CRC High Byte
  
  for (int index = 0; index < EPOSBUFFSIZE; index++)
  {
    port.write(buff[index]);
  }
  
  while (port.available())
  {
    port.read(); // you have to clear the input buffer
  }
}

void updateControlWord(SoftwareSerial &port, word newWord)
{  
  byte Len = 0x04;
  byte OpCode = 0x68;

  byte NodeID = 0x01;
  word ObjectIndex = 0x6040;
  byte SubIndex = 0x00;

  byte fillword = 0x0000;
  byte data = newWord;
  
  word DataArray[6];

  DataArray[0] = word(Len, OpCode);   // len and opcode
  DataArray[1] = word(lowByte(ObjectIndex), NodeID);
  DataArray[2] = word(0x00, highByte(ObjectIndex));
  DataArray[3] = data; 
  DataArray[4] = fillword;
  DataArray[5] = word(0x00, 0x00);    // Zero word

  word CRC = CalcFieldCRC(DataArray, 6);

  byte buff[EPOSBUFFSIZE];

  /* SYNC */ 
  buff[0] = DLE; // DLE
  buff[1] = STX; // STX

  /* HEADER */
  buff[2] = OpCode; // OpCode
  buff[3] = Len; // Len

  /* DATA */ 
  buff[4] = NodeID;                 // LowByte data[0], Node ID
  buff[5] = lowByte(ObjectIndex);   // HighByte data[0], LowByte Index
  buff[6] = highByte(ObjectIndex);  // LowByte data[1], HighByte Index
  buff[7] = SubIndex;               // HighByte data[1], SubIndex
  buff[8] = lowByte(data);
  buff[9] = highByte(data);
  buff[10] = lowByte(fillword);
  buff[11] = highByte(fillword);

  /* CRC */ 
  buff[12] = lowByte(CRC); // CRC Low Byte
  buff[13] = highByte(CRC); // CRC High Byte
  
  for (int index = 0; index < EPOSBUFFSIZE; index++)
  {
    port.write(buff[index]);
  }
  
  while (port.available())
  {
    port.read(); // you have to clear the input buffer
  }
}

word CalcFieldCRC(word* pDataArray, word numberOfWords)
{
  word shifter, c;
  word carry;
  word CRC = 0;
  
  while(numberOfWords--)
  {
    shifter = 0x8000;
    c = *pDataArray++;
    do
    {
      //Initialize BitX to Bit15
      //Copy next DataWord to c
      carry = CRC & 0x8000;
      CRC <<= 1;
      if(c & shifter) CRC++;
      if(carry) CRC ^= 0x1021;
      shifter >>= 1;
      //Check if Bit15 of CRC is set
      //CRC = CRC * 2
      //CRC = CRC + 1, if BitX is set in c
      //CRC = CRC XOR G(x), if carry is true
      //Set BitX to next lower Bit, shifter = shifter/2
    } while(shifter);
  }
  return CRC;
}
/* end of EPOS functions */

