#include <SoftwareSerial.h>

#define EPOSBUFFSIZE 14

/* start of EPOS command constants */
#define CTRL_MOVEREL 0x007F   // payload for the relative movement
#define CTRL_MOVEABS 0x0001F  
#define CTRL_ENABLE 0x000F    //
#define CTRL_RESET 0x0006     //
#define CTRL_ERROR 0x0080

#define DLE 0x90  // part one of the sync part of the communication frame
#define STX 0x02  // part two of the sync part of the communication frame
/* end of EPOS command constants */

union longArray
{
  long myLong;
  char myBytes[4];
};

signed union doubleArray
{
  signed long myDouble;
  char myBytes[4];
};

SoftwareSerial EPOSSerial2(7, 6); // RX, TX
SoftwareSerial EPOSSerial3(5, 4); // RX, TX


void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  Serial.println("Starting Init");
  
  // set the data rate for the SoftwareSerial port
  EPOSSerial2.begin(9600);
  EPOSSerial3.begin(9600);
  
  updateControlWord(EPOSSerial2, CTRL_RESET);
  updateControlWord(EPOSSerial2, CTRL_ENABLE);

  updateControlWord(EPOSSerial3, CTRL_RESET);
  updateControlWord(EPOSSerial3, CTRL_ENABLE);

  Serial.println("Ending Init");
}

int count = 0;

void loop()
{
  if (count % 2)
  {
    updatePositionDemmand(EPOSSerial2, 0);
    updateControlWord(EPOSSerial2, CTRL_MOVEABS);
    updateControlWord(EPOSSerial2, CTRL_ENABLE);
    word s = readStatusWord(EPOSSerial2);
    if (s == 8)
    {
      Serial.println("Error Detected, reading Error");
      word e = readErrorRegister(EPOSSerial2);
      
      if (e == 0x8A81)
      {
        Serial.println("Positive Limit Switch Depressed");
        
      }
      
      Serial.println("Resetting Error!");
      updateControlWord(EPOSSerial2, CTRL_ERROR);
      updateControlWord(EPOSSerial2, CTRL_RESET);
    }
  }
  else
  {
    updatePositionDemmand(EPOSSerial2, 50000);
    updateControlWord(EPOSSerial2, CTRL_MOVEABS);
    updateControlWord(EPOSSerial2, CTRL_ENABLE);
    word s = readStatusWord(EPOSSerial2);
    if (s == 8)
    {
      Serial.println("Error Detected, reading Error");
      word e = readErrorRegister(EPOSSerial2);
      
      if (e == 0x8A81)
      {
        Serial.println("Positive Limit Switch Depressed");
      }
      
      Serial.println("Resetting Error!");
      updateControlWord(EPOSSerial2, CTRL_ERROR);
      updateControlWord(EPOSSerial2, CTRL_RESET);
    }
  }
  count++;
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

  byte inputBuff[64];
  int inputBuffSize;

  writeBuffToEPOS(port, buff, EPOSBUFFSIZE, inputBuff, inputBuffSize);
}

long readPositionActualValue(SoftwareSerial &port)
{
  word ObjectIndex = 0x6064;
  byte OpCode = 0x60;
  byte Len = 0x02;
  byte SubIndex = 0x00;
  byte NodeID = 0x01;

  word DataArray[4];

  DataArray[0] = word(Len, OpCode);   // len and opcode
  DataArray[1] = word(lowByte(ObjectIndex), NodeID);
  DataArray[2] = word(0x00, highByte(ObjectIndex));
  DataArray[3] = word(0x00, 0x00);    // Zero word

  word CRC = CalcFieldCRC(DataArray, 4);

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

  /* CRC */ 
  buff[8] = lowByte(CRC); // CRC Low Byte
  buff[9] = highByte(CRC); // CRC High Byte

  byte inputBuff[64];
  int inputBuffSize;
  
  writeBuffToEPOS(port, buff, 10, inputBuff, inputBuffSize);

  doubleArray da;
  
  da.myBytes[0] = inputBuff[1];
  da.myBytes[1] = inputBuff[2];
  da.myBytes[2] = inputBuff[3];
  da.myBytes[3] = inputBuff[4];
     
  return da.myDouble;
}

word readErrorRegister(SoftwareSerial &port)
{
  word ObjectIndex = 0x603F;
  byte OpCode = 0x60;
  byte Len = 0x02;
  byte SubIndex = 0x00;
  byte NodeID = 0x01;

  word DataArray[4];

  DataArray[0] = word(Len, OpCode);   // len and opcode
  DataArray[1] = word(lowByte(ObjectIndex), NodeID);
  DataArray[2] = word(0x00, highByte(ObjectIndex));
  DataArray[3] = word(0x00, 0x00);    // Zero word

  word CRC = CalcFieldCRC(DataArray, 4);

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

  /* CRC */ 
  buff[8] = lowByte(CRC); // CRC Low Byte
  buff[9] = highByte(CRC); // CRC High Byte

  byte inputBuff[64];
  int inputBuffSize;
  
  writeBuffToEPOS(port, buff, 10, inputBuff, inputBuffSize);
   
  word w = word(inputBuff[2], inputBuff[1]); // issa hack
  
  return w;
}

word readStatusWord(SoftwareSerial &port)
{
  word ObjectIndex = 0x6041;
  byte OpCode = 0x60;
  byte Len = 0x02;
  byte SubIndex = 0x00;
  byte NodeID = 0x01;

  word DataArray[4];

  DataArray[0] = word(Len, OpCode);   // len and opcode
  DataArray[1] = word(lowByte(ObjectIndex), NodeID);
  DataArray[2] = word(0x00, highByte(ObjectIndex));
  DataArray[3] = word(0x00, 0x00);    // Zero word

  word CRC = CalcFieldCRC(DataArray, 4);

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

  /* CRC */ 
  buff[8] = lowByte(CRC); // CRC Low Byte
  buff[9] = highByte(CRC); // CRC High Byte

  byte inputBuff[64];
  int inputBuffSize;
  
  writeBuffToEPOS(port, buff, 10, inputBuff, inputBuffSize);
 
  word w = word(inputBuff[0], inputBuff[1]); // issa hack
  
  return w;
}

void updateControlWord(SoftwareSerial &port, word newWord)
{  
  byte Len = 0x04;
  byte OpCode = 0x68;

  byte NodeID = 0x01;
  word ObjectIndex = 0x6040;
  byte SubIndex = 0x00;

  byte data = newWord;
  byte fillword = 0x0000;
  
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

  byte inputBuff[64];
  int inputBuffSize;
  
  writeBuffToEPOS(port, buff, EPOSBUFFSIZE, inputBuff, inputBuffSize);
}

void writeBuffToEPOS(SoftwareSerial &port, byte *outputBuff, int outputBuffSize, byte *dataBuffer, int &dataBufferSize)
{

  byte b; // to hold bytes read in from the EPOS
  byte inputBuffer[64]; // hold the bytes from the EPOS
  int inputBufferSize = 0; // count the size of the EPOS
  bool searchingForLength = true; // remain true until the len byte has been read in
  int bytesInData; // hold the number of bytes in the data part of the message
  int numberAvailableBytes = 0; // hold the number of bytes in the input buffer of the EPOS port
  
  for (int index = 0; index < outputBuffSize; index++)
  {
    port.write(outputBuff[index]);
  }

  while (searchingForLength)
  {
    if (port.available())
    {
      b = port.read(); // start to read in the INPUT buffer

      if ((inputBufferSize == 0) && (b == DLE)) // get the start of frame
      {
        inputBuffer[inputBufferSize] = b;
        inputBufferSize++;  
      }

      else if ((inputBufferSize == 1) && (b == STX)) // get the sync
      {
        inputBuffer[inputBufferSize] = b;
        inputBufferSize++;
      }

      else if (inputBufferSize == 2) // get OpCode
      {
        inputBuffer[inputBufferSize] = b;
        inputBufferSize++;
      }

      else if (inputBufferSize == 3) // get len, then read in the data bytes and checksum
      {
        bytesInData = ((b) * 2); // b will hold the number of two byte WORDS that will be read in so it must be *2'd
        searchingForLength = false;

        inputBuffer[inputBufferSize] = b; // b is len at this point
        inputBufferSize++;
        
        while (numberAvailableBytes < (bytesInData + 2)) // wait until there are the number of bytes for the header + the message + 2 for the crc
        {
          numberAvailableBytes = port.available();
        }

        dataBufferSize = 0;
        
        for (int index = 0; index < bytesInData; index++) // read in the header + the data for the actual frame
        {
          b = port.read();
          inputBuffer[inputBufferSize] = b;
          
          if ((inputBufferSize > 6) && (dataBufferSize < 4)) // don't read DLE, STX, len, OpCode
          {
            dataBuffer[dataBufferSize] = b;
            dataBufferSize++;
          }
          inputBufferSize++;
        }
        
        for (int index = 0; index < 2; index++) // read in the two bytes for the checksum
        {
          b = port.read();
          inputBuffer[inputBufferSize] = b;
          inputBufferSize++;
        }
      }
    }
  }
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

word CalcFieldCRC(word* pDataArray, word numberOfWords)
/* This is from the maxon docs */
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
