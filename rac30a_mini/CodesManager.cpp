#include "CodesManager.h"
#include <Arduino.h>
#include <String.h>


#define ARDUINODUE
//#define ARDUINOADK
//#define ARDUINOMINI
//#define NODEMCU


//Memory
#ifdef ARDUINODUE
#include <DueFlashStorage.h>
DueFlashStorage dueFlashStorage;
int CodesManager::memoryWrite(int address, int car){ return( dueFlashStorage.write(PHYSICALMEMORYCODEADDRESS+address,car));}
int CodesManager::memoryRead(int address){ return( dueFlashStorage.read(PHYSICALMEMORYCODEADDRESS+address));}
#endif
#ifdef ARDUINOADK
#include <EEPROM.h>
int CodesManager::memoryWrite(int address, int car){ EEPROM.write(PHYSICALMEMORYCODEADDRESS+address,car);return(1);}
int CodesManager::memoryRead(int address){ return( EEPROM.read(PHYSICALMEMORYCODEADDRESS+address));}
#endif
#ifdef ARDUINOMINI
#include <EEPROM.h>
int CodesManager::memoryWrite(int address, int car){ EEPROM.write(PHYSICALMEMORYCODEADDRESS+address,car);return(1);}
int CodesManager::memoryRead(int address){ return( EEPROM.read(PHYSICALMEMORYCODEADDRESS+address));}
#endif
#ifdef NODEMCU
#include <EEPROM.h>
int CodesManager::memoryWrite(int address, int car){ EEPROM.write(PHYSICALMEMORYCODEADDRESS+address,car);EEPROM.commit();return(1);}
int CodesManager::memoryRead(int address){ return( EEPROM.read(PHYSICALMEMORYCODEADDRESS+address));}
#endif
static const int maxaddress = PHYSICALMEMORYLEN;





/*
Tale classe e' utilizzata per implementare un container di codici da abilitare/disabilitare
Prevedono l'uso di una memoria fisica (es: EEPROM) che e' wrappata dalla classe Memory4Codes.
Nella memoria fisica i codici sono memorizzati come una stringa:
[<codiceAmm>;<codice1>;<codice2>;...;<codicen>;]
dove il primo codice e' un codice di amministrazione.

In pratica la stringa deve soddisfare i seguenti requisiti (utilizzati da una apposita funzione di verifica del contenuto della memoria):
1) inizia con "["
2) finisce con "]"
3) la sequenza, a meno delle parentesi quadre iniziali e finali, contiene solo numeri  e ';'
4) lunghezza < dimensione memoria fisica

Dopo un reset;   "[<codiceAmm>;]" che, se non e' impostato e' "00000000"
Ogni codice non di amministrazione e' preceduto da [ e seguito da un ';' ossia "[<codiceamm>;" 

Il codice di amministrazione e' stato introdotto in modo da prevedere operazioni sui codici attraverso stringhe comando:

1)  "<codiceAmm>,<operazione di 2 char><codice>"
2)  "<codice>"

In pratica se nella richiesta operazione non c'e' una "," allora viene interpretato come un check di codice
Se invece c'e' una "," allora viene verificato se il <codiceAmm> e' uguale al codice di amministrazione e viene interpretato il codice
successivo come riportato nella tabella:

Tabella Codici operazione

00<code>  inserisce/abilita codice
01<code>  cancella/disabilita codice
03<code>  cancella tutti i codici  
04<code>  sostituisce codice amministrazione con <code>


*/



void CodesManager::esitoBufferReset(){for (int i=0; i<MAXLEN__ESITOBUFFER ; i++){esitoBuffer[i]='\0';}}
int CodesManager::setEsitoBuffer(String stringa){
     for (int i = 0; i<stringa.length(); i++) {
         if (i<MAXLEN__ESITOBUFFER-1) {
               esitoBuffer[i] = stringa.charAt(i);
               esitoBuffer[i+1]='\0';
          }
     }
     Serial.print("Esito buffer: <");Serial.print(esitoBuffer);Serial.println(">");
}
int CodesManager::setEsitoBuffer(char* str){setEsitoBuffer(String(str));}
char* CodesManager::getEsitoBuffer(){return(esitoBuffer);}

int debug(String stringa){
  if (DEBUGENABLED) Serial.print(stringa);
  //delay(2000);
}


CodesManager::CodesManager(){
       //CodesManager::init();
       esitoBufferReset();
}
CodesManager::~CodesManager(){}
	
	
//-----------------------------------------------------------
//------------PRIVATE--------------------------------------
//-----------------------------------------------------------
int CodesManager::ifNumericCode(String code){
       for (int i=0; i<code.length(); i++) {
            switch (code.charAt(i)) {
                   case '0': break;
                   case '1': break;
                   case '2': break;
                   case '3': break;
                   case '4': break;
                   case '5': break;
                   case '6': break;
                   case '7': break;
                   case '8': break;
                   case '9': break;
                   default:  return (CODESLIB_KO);
            }
       }
       return(CODESLIB_OK);
}
//-----------------------------------------------------------
int CodesManager::ifValidStringToMemory(String stringa){
    //[<codiceAmm>;<codice1>;<codice2>;...;<codicen>;]\n
    if (stringa.length() >= PHYSICALMEMORYLEN) return(CODESLIB_KO);
    if (stringa.charAt(0) != '[') return(CODESLIB_KO);
    for (int i=0; i<stringa.length()-2; i++) {
         switch (stringa.charAt(i)) {
           case '0': break;
           case '1': break;
           case '2': break;
           case '3': break;
           case '4': break;
           case '5': break;
           case '6': break;
           case '7': break;
           case '8': break;
           case '9': break;
           case ';': break;
           default: return(CODESLIB_KO);
         }
    }
    if (stringa.charAt(stringa.length()-2) != ';') return(CODESLIB_KO);
    if (stringa.charAt(stringa.length()-1) != ']') return(CODESLIB_KO);
    return(CODESLIB_OK);
}
//-----------------------------------------------------------
//Salva in memoria una Stringa che dovrebbe essere fatta cosi: "[<codiceAmm>;<codice1>;<codice2>;...;<codicen>;]". Aggiunge '\n' alla fine.
int CodesManager::writeInMemory(String stringa){
   if (ifValidStringToMemory(stringa) != CODESLIB_KO) return(CODESLIB_KO);
   for (int i=0; i<stringa.length(); i++) {memoryWrite(ROM_ADDRESS_CODES + i,stringa.charAt(i));
                                           //Serial.print("Memory check: address(");
                                           //Serial.print(ROM_ADDRESS_CODES + i);
                                           //Serial.print(")  value:");
                                           //Serial.println(memoryRead(ROM_ADDRESS_CODES + i));
   }
   //debugMemory();
   return(CODESLIB_OK);
 }
//-----------------------------------------------------------
int CodesManager::verifyCodesMemory(){
    debug("VerifyCodesMemory ...\n");
    char tmp = memoryRead(ROM_ADDRESS_CODES);
    //debug(String("verifyCodesMemory: address(0):   --> ") + String(tmp) + String("\n")); 
    if (tmp != '[') return(CODESLIB_KO);
    int ctrl = 1;
    int counter = 1;
    int len = 0;
    while ((ctrl) && (counter < maxaddress)){
        tmp = memoryRead(ROM_ADDRESS_CODES + counter);
        switch (tmp) {
          case '0': break;
          case '1': break;
          case '2': break;
          case '3': break;
          case '4': break;
          case '5': break;
          case '6': break;
          case '7': break;
          case '8': break;
          case '9': break;
          case ';': break;
          case ']': {
                      len = counter;
                      ctrl=0;
                      break;
                    }
          default:
                    return(CODESLIB_KO);
        }
        counter++;
	  }
    if (ctrl == 1) return(CODESLIB_KO);
    //[00000000;1234;]
    if (memoryRead(ROM_ADDRESS_CODES + len-1) != ';' ) return(CODESLIB_KO);
    debug("verifyCodesMemory(): OK\n");
	  return(CODESLIB_OK);
}
//-----------------------------------------------------------
int CodesManager::getLengthCodesMemory(){
       if (verifyCodesMemory() == CODESLIB_OK) {
           for (int i=0; i<maxaddress; i++) {
           	   if (memoryRead(ROM_ADDRESS_CODES + i) == ']') {
           	   	     return(i+1);
           	   }
           }
           return(0);
       } else {
           return(0);
       }
}
//-----------------------------------------------------------
int CodesManager::resetCodesMemory(String codeamm){
       //"[<codeamm>;]/n"
       debug(String("ResetCodesMemory: ") + codeamm + String("\n"));
       if (ifNumericCode(codeamm) != CODESLIB_OK ) {debug("ERROR CODE NOT VALID\n"); return(CODESLIB_KO);}
       if (codeamm.length() ==0) codeamm=String("00000000");
       String tmp = String("[")+codeamm+String(";]");
       debug(String("Inizializing memory with sequence <") + tmp + String(">\n")); 
       return(writeInMemory(tmp));
}
int CodesManager::resetCodesMemory(){
       return(resetCodesMemory("00000000"));
}
//---------------------------------------------------------------------------
int CodesManager::init(){
    #ifdef NODEMCU
    EEPROM.begin(1024);
    #endif
    debug("InitCodes ...\n");
    if (verifyCodesMemory() == CODESLIB_KO) {
         debug("Initializing ... \n");
         delay(2000);
         resetCodesMemory(String("00000000"));
         toSerial();
    } else {
         debug("Already initialized.\n");
    }
}
//-----------------------------------------------------------
int CodesManager::enableCode(String code){
    debug(String("EnableCode: ") + code + String("\n")); 
    if (ifNumericCode(code) != CODESLIB_OK ) {debug("ERROR CODE NOT VALID\n"); return(CODESLIB_KO);}
    String x = loadCodesFromMemory();
    //debug(x);
    if (x.length()==0) {debug("ERROR"); return(CODESLIB_KO);}
    if (x.indexOf(String(";") + code + String(";")) == -1) {
        String y=  x.substring(0,x.length()-1) + code + String(";]");
        debug(String("From:<") + x + String(">   -->  <") + y+ String(">\n"));
        writeInMemory(y);
        return(CODESLIB_OK);
    } else {
        return(CODESLIB_OK);
    }
}
//-----------------------------------------------------------
int CodesManager::disableCode(String code){
    debug(String("DisableCode: ") + code + String("\n"));
    if (ifNumericCode(code) != CODESLIB_OK ) {debug("ERROR CODE NOT VALID\n"); return(CODESLIB_KO);}
    String x = loadCodesFromMemory();
    //debug(x);
    int index = x.indexOf(String(";") + code + String(";"));
    if ( index == -1) {debug("CODE NOT EXISTING\n"); return(CODESLIB_OK);
    } else {
     String tmp = x.substring(0,index) + x.substring(index + code.length() + 1);
     debug(String("From:<") + x + String(">   -->  <") + tmp+ String(">\n"));
     writeInMemory(tmp);
    }
    return(CODESLIB_OK);
}
//-----------------------------------------------------------
int CodesManager::disableAllCodes(){
      debug("DisableAllCodes\n");
      String x = loadCodesFromMemory();
      String y = x.substring(0,x.indexOf(String(";"))) + String(";]");
      writeInMemory(y);
}
//-----------------------------------------------------------
int CodesManager::checkCode(String code){
    debug(String("CheckCode: ") + code + String("\n"));
    if (ifNumericCode(code) != CODESLIB_OK ) {debug("ERROR CODE NOT VALID\n"); return(CODESLIB_KO);}
    String x = loadCodesFromMemory();
    int index = x.indexOf(String(";") + code + String(";"));
    switch (index) {
          case -1: {debug("CODE NOT EXISTING\n"); return(CODESLIB_KO);}
          default: {debug("CODE EXISTING\n"); return(CODESLIB_OK);}    
    }
}
//-----------------------------------------------------------
int CodesManager::checkAmmCode(String p){
    String y = String();
    y = p;
    debug(String("CheckAmmCode: <") + y + String(">\n"));
    if (ifNumericCode(y) != CODESLIB_OK ) {debug("ERROR CODE NOT VALID\n"); return(CODESLIB_KO);}
    //debug(y);
    String authcode=String();
    authcode = getAmmCode();
    //debug(authcode);
    if (y == authcode) {
      //debug(y);
      debug("CODE EXISTING\n"); 
      return(CODESLIB_OK);
    } else {
      //debug(y);
      debug("NOT EXISTING\n"); 
      //Serial.println(y);
      return(CODESLIB_KO);
    }
}
//-----------------------------------------------------------
String CodesManager::getAmmCode(){
    String x = loadCodesFromMemory();
    return(x.substring(1,x.indexOf(';')));
}


//-----------------------------------------------------------
int CodesManager::setAmmCode(String code){
    debug(String("SetAmmCode: ") + code + String("\n"));
    if (ifNumericCode(code) != CODESLIB_OK ) {debug("SetAmmCode: ERROR CODE NOT VALID\n"); return(CODESLIB_KO);}
    String x = loadCodesFromMemory();
    if (x.indexOf(String("[") + code + String(";")) == -1) {
       String tmp = String("[") + code + String(";") + x.substring(x.indexOf(";")+1);
       debug(String("SetAmmCode: from:<") + x + String(">   -->  <") + tmp+ String(">\n"));
       writeInMemory(tmp);
       return(CODESLIB_OK);
    } else {
        return(CODESLIB_OK);
    }
}
//-----------------------------------------------------------
String CodesManager::loadCodesFromMemory(){
    debug("LoadCodesFromMemory\n");
    int len = getLengthCodesMemory();
    debug(String("Size: ") + String(len) + String("\n"));
    if (len == 0) return(String(""));
    char arraytmp[len];
	  for (int i=0; i<len; i++) arraytmp[i] = memoryRead(ROM_ADDRESS_CODES + i);	
    arraytmp[len]='\0';
    debug(String("Codes: <") + String(arraytmp) + String(">\n"));
    String tmp = String(arraytmp);
    return(tmp);
}    	
//-----------------------------------------------------------
int CodesManager::toSerial(){
    String x = loadCodesFromMemory();
    Serial.println(x);
    return(CODESLIB_OK);
}
//-----------------------------------------------------------
String CodesManager::getCodes(){
    String x = loadCodesFromMemory();
    return(x.substring(x.indexOf(";"),x.length()-1));
}

//-----------------------------------------------------------
char* CodesManager::dumpMemory(char * array){
    for (int i=0; i<maxaddress; i++) array[i] = memoryRead(ROM_ADDRESS_CODES + i);
    return(array);
}
//-----------------------------------------------------------
void CodesManager::debugMemory(){
    int x = getLengthCodesMemory();
    if (x == 0) {
       for (int i=0; i<200; i++) {
          Serial.print(String("Memory:  address(") + String(i) + String(")   value:") + String(memoryRead(ROM_ADDRESS_CODES + i)) + String("\n"));
          //delay(1000);
       }
    } else {
        for (int i=0; memoryRead(ROM_ADDRESS_CODES + i) != ']'; i++) {
          Serial.print(String("Memory:  address(") + String(i) + String(")   value:") + String(memoryRead(ROM_ADDRESS_CODES + i)) + String("\n"));
          //delay(1000);
       }     
    }
}

  

//-----------------------------------------------------------
void CodesManager::manageCommand(String request){
  //"<codiceAuth>,<comando><extradata>" oppure ",<comando>"
  debug(String("ManageCommand: cmd:<") + request + String(">\n"));
  esitoBufferReset();
  if (request.indexOf(",") == -1) { 
    debug("Sintax error command\n");return;
  }
  String authcode = request.substring(0,request.indexOf(','));
  if (request.substring(request.indexOf(",") +1).length() < 2) { 
    debug("Syntax error command\n");return;
  }
  String codecommand = request.substring(request.indexOf(",") +1,request.indexOf(",") +3);
  String extradata = request.substring(request.indexOf(",") +3);
  String x = loadCodesFromMemory();
  //esempio: "99999999,00123456789"
  debug(String("authcode:<") + authcode + String(">  codecommand:<") + codecommand + String(">  extradata:<") + extradata + String(">\n"));
  if         (codecommand == ENABLECODE_CMDCODE)  { 
                                                     debug("Eenabling code\n"); 
                                                     int res = checkAmmCode(authcode);
                                                      if  (res == CODESLIB_OK) {
                                                            enableCode(extradata);
                                                            setEsitoBuffer("@ENABLECODE:OK\n");
                                                      } else {
                                                            setEsitoBuffer("@ENABLECODE:KO\n");
                                                      }
  } else if  (codecommand == DISABLECODE_CMDCODE) { 
                                                     debug("Disabling code\n");        
                                                     int res = checkAmmCode(authcode);
                                                      if  (res == CODESLIB_OK) {
                                                            disableCode(extradata);
                                                            setEsitoBuffer("@DISABLECODE:OK\n");
                                                      } else {
                                                            setEsitoBuffer("@DISABLECODE:KO\n");
                                                      }
  } else if  (codecommand == DISABLEALL_CMDCODE) { 
                                                     debug("Disabling all codes\n");        
                                                     int res = checkAmmCode(authcode);
                                                      if  (res == CODESLIB_OK) {
                                                            disableAllCodes();
                                                            setEsitoBuffer("@DISABLEALL:OK\n");
                                                      } else {
                                                            setEsitoBuffer("@DISABLEALLE:ERROR\n");
                                                      }
   } else if  (codecommand == CHKCODE_CMDCODE)     {
                                                     debug("Checking code\n");         
                                                     int res = checkCode(authcode);
                                                     if  (res== CODESLIB_OK) {
                                                            setEsitoBuffer("@CHECKCODE:OK\n");
                                                     } else {
                                                            setEsitoBuffer("@CHECKCODE:KO\n");
                                                     }
   } else if  (codecommand == SYSRESET_CMDCODE)    {
                                                     debug("Resetting codes\n");        
                                                     int res = checkAmmCode(authcode);
                                                      if  (res== CODESLIB_OK) {
                                                            resetCodesMemory();
                                                            setEsitoBuffer("@SYSRESETE:OK\n");
                                                      } else {
                                                            setEsitoBuffer("@SYSRESET:ERROR\n");
                                                      }
   } else if  (codecommand == CHGAMMCODE_CMDCODE)  { 
                                                     debug("Changing amm code\n");        
                                                     int res = checkAmmCode(authcode);
                                                      if  (res== CODESLIB_OK) {
                                                            setAmmCode(extradata);
                                                            setEsitoBuffer("@CHGAMMCODE:OK\n");
                                                      } else {
                                                            setEsitoBuffer("@CHGAMMCODE:ERROR\n");
                                                      }
   } else if  (codecommand == CHKAMMCODE_CMDCODE)  { 
                                                     debug("Checking amm code\n");         
                                                     int res = checkAmmCode(extradata);
                                                     if  (res== CODESLIB_OK) {
                                                            setEsitoBuffer("@CHKAMMCODE:OK\n");
                                                     } else {
                                                            setEsitoBuffer("@CHKAMMCODE:ERROR\n");
                                                     }
   } else if  (codecommand == LISTCODES_CMDCODE)   { 
                                                     debug("Listing codes\n");        
                                                     int res = checkAmmCode(authcode);
                                                      if  (res== CODESLIB_OK) {
                                                            String tmp = getCodes();
                                                            setEsitoBuffer(String("@LISTCODES:OK (")+tmp+String(")\n"));
                                                      } else {
                                                            setEsitoBuffer("@LISTCODES:ERROR\n");
                                                      }
   } else {                            
    debug("Cmd not recognized\n");
   }
   return;
}
 
