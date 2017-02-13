
#ifndef __MFRC522DEF__
#define __MFRC522DEF__

typedef struct UIDStruct Uid;

enum StatusCode PCD_CalculateCRC(  unsigned char *data, unsigned char length, unsigned char *result);
void PCD_StopCrypto1();
int MFRC522loop();
static void BoardInit(void);
void pinMode(unsigned char Pin, unsigned char direction);
unsigned char digitalRead(unsigned char ipPin);
void digitalWrite(unsigned char opPin, unsigned char Level);
void delay(unsigned int delaytime);
void MFRC522_init();
void PCD_WriteRegister(  unsigned char reg, unsigned char value);
void PCD_WriteRegister_alternate(  unsigned char reg, unsigned char value);
void PCD_WriteBytes(  unsigned char reg, unsigned char count, unsigned char *values);
unsigned char PCD_ReadRegister( unsigned char reg);
void PCD_ReadBytes( unsigned char reg, unsigned char count, unsigned char *values, unsigned char rxAlign);
void PCD_SetRegisterBitMask( unsigned char reg, unsigned char mask);
void PCD_ClearRegisterBitMask( unsigned char reg, unsigned char mask);
void PCD_AntennaOn();
enum StatusCode PICC_HaltA();
void PCD_Reset();
void PCD_Init();
void PCD_AntennaOff();
enum StatusCode PCD_CommunicateWithPICC( unsigned char command, unsigned char waitIRq, unsigned char *sendData, unsigned char sendLen, 
										unsigned char *backData, unsigned char *backLen, unsigned char *validBits, unsigned char rxAlign,
										unsigned char checkCRC);
enum StatusCode PCD_TransceiveData(  unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned char *backLen, unsigned char *validBits,
									unsigned char rxAlign, unsigned char checkCRC);
enum StatusCode PICC_RequestA( unsigned char *bufferATQA, unsigned char *bufferSize);
enum StatusCode PICC_REQA_or_WUPA( unsigned char command, unsigned char *bufferATQA, unsigned char *bufferSize);
enum StatusCode PICC_Select( Uid *uid, unsigned char validBits);
enum StatusCode MIFARE_Read( unsigned char blockAddr, unsigned char *buffer, unsigned char *bufferSize);  	
enum StatusCode PCD_MIFARE_Transceive( unsigned char *sendData, unsigned char sendLen, unsigned char acceptTimeout);
enum PICC_Type PICC_GetType(unsigned char sak);
void PCD_DumpVersionToSerial();
void PICC_DumpDetailsToSerial(Uid *uid);
unsigned char PICC_IsNewCardPresent();
unsigned char PICC_ReadCardSerial();
void PICC_DumpToSerial(Uid *uid);

//Http functions

short CallHttpPost(unsigned char request);
short CallHttpGet(unsigned char request); //When using Brijenders Changes
int APConnectInit();
double GetOBjJSON(char *ptr  , unsigned char opt);
int FileIo(int wrtflg, char *SSID, char *PWD);
extern void ConnectToRouter(void);
extern void SlStop(void);

typedef enum{
	GET_CREDIT_TO_PLAY,
	CURRENT_CREDIT_BAL,
	FREEPLAY_FLAG,
	CURRENT_TICKET_BAL,
	CANPLAY_FLAG,
	TICKET_MULTIPLIER,
	UPDATED_TICKET_BALANCE,
	TRANSACTION_ID,
	SSID_PWD,
}JSON_OBJ;
#endif					
