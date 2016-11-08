#ifndef CODESMANAGER_h
#define CODESMANAGER_h

//-------------------------- CODESMANAGER Class -------------------------------------------------

#include <Arduino.h>

#define ROM_ADDRESS_CODES   100
#define CODICEAMM           200
#define CODESLIB_OK         300
#define CODESLIB_KO         400



#define ENABLECODE_CMDCODE     "00"
#define DISABLECODE_CMDCODE    "01"
#define CHKCODE_CMDCODE        "02"
#define DISABLEALL_CMDCODE     "03"
#define GETCODES_CMDCODE       "04"
#define SYSRESET_CMDCODE       "05"
#define CHGAMMCODE_CMDCODE     "06"
#define CHKAMMCODE_CMDCODE     "07"
#define LISTCODES_CMDCODE      "08"
#define OPEN1_CMDCODE          "09"
#define OPEN2_CMDCODE          "10"


#define MAXLEN__ESITOCOMMANDBUFFER      50
#define MAXLEN__ESITOBUFFER      50

#define PHYSICALMEMORYLEN 600
#define PHYSICALMEMORYCODEADDRESS 0


#define DEBUGENABLED     1


class CodesManager { 
      char esitoBuffer[MAXLEN__ESITOBUFFER];
      char administrationCode[15];
      char code[100];
      int ifNumericCode(String code);
      int ifValidStringToMemory(String stringa);
      int writeInMemory(String stringa);
      int verifyCodesMemory();
      int getLengthCodesMemory();
      void debugMemory();
      int memoryWrite(int address, int car);
      int memoryRead(int address);

   public:
      CodesManager();
      ~CodesManager();
      int init();

      int enableCode(String code);
      int disableCode(String code);
      int checkCode(String code);
      int disableAllCodes();
      String getCodes();      
      int    setAmmCode(String code);
      String getAmmCode();      
      int    checkAmmCode(String code);
      int resetCodesMemory();
      int resetCodesMemory(String codeamm);
      char* getEsitoBuffer();
      int setEsitoBuffer(String stringa);
      int setEsitoBuffer(char* str);
      void esitoBufferReset();
      String loadCodesFromMemory();
      int toSerial();
      char* dumpMemory(char * array);
      void manageCommand(String request);
};


#endif
