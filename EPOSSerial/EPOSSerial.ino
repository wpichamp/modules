#include <SoftwareSerial.h>

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

SoftwareSerial EPOSSerial(8, 9); // RX, TX

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  // set the data rate for the SoftwareSerial port
  EPOSSerial.begin(9600);

  // enables the coil
  updateControlWord(CTRL_RESET);
  updateControlWord(CTRL_ENABLE);

  delay(1000);
}

int count = 0;

void loop()
{
  if (count % 2)
  {
    updatePositionDemmand(0);
    updateControlWord(CTRL_ENABLE);
    updateControlWord(CTRL_MOVEABS);
    updateControlWord(CTRL_ENABLE);
  }
  else
  {
    updatePositionDemmand(50000);
    // updateControlWord(CTRL_ENABLE);
    updateControlWord(CTRL_MOVEABS);
    updateControlWord(CTRL_ENABLE);
  }
  count++;
}

void updatePositionDemmand(long newValue)
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
  
  Serial.println(newValue);
  for (int index = 0; index < 4; index ++)
  {
    Serial.print(index);
    Serial.print(":");
    Serial.print("0x");
    Serial.print(la.myBytes[index], HEX);
    Serial.print(" ");
  }
  Serial.println("");

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
    EPOSSerial.write(buff[index]);
  }
  
  while (EPOSSerial.available())
  {
    EPOSSerial.read(); // you have to clear the input buffer
  }
}

void updateControlWord(word newWord)
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
    EPOSSerial.write(buff[index]);
  }
  
  while (EPOSSerial.available())
  {
    EPOSSerial.read(); // you have to clear the input buffer
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

