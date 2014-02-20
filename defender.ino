//**** 0. Documentation Section
//  Car security system for use with smartphones
//  Author: João Victor Tolentino
//  Date: 2/16/2014
//  OBS1: System codes:
//        0 - Desliga o sistema
//        1 - Liga o sistema
//        2 - Solicita status do sistema
//        3 - Troca de senha
//        4 - Adicionar novo usuário (telefone 13 digitos)
// OBS2: EEPROM Codes
//         - A senha sempre começa no endereço 0 da flash



// 1. Pre-processor Directives Section
//  INCLUDES
#include <EEPROM.h>
#include <SoftwareSerial.h>
//#include <TinyGPS.h>
//  DEFINES
#define COMMAND_SIZE   100
#define ON             1
#define OFF            0
#define PIR_SENSOR     10
#define TILT_SENSOR    11
#define BUZZER_ALARM   12


// 2. Global Declarations section
// GLOBAL VARIABLES
int systemStatus;
char command[COMMAND_SIZE];
SoftwareSerial gps = SoftwareSerial(4, 5); // RX, TX

// FUNCTION PROTOTYPES
char VerifyPassword(char *i);
void TurnSystemON(void);
void TurnSystemOFF(void);
void GetSystemStatus(void);
char ChangePassword(char arrayPosition);
char NewUser(char arrayPosition);
void GetCommand(char serialSource);


// 3. Subroutines Section
void setup()
{       
        // Defining data inputs
        pinMode(PIR_SENSOR, INPUT);
        pinMode(TILT_SENSOR, INPUT);
  
        // Defining data outputs
        pinMode(BUZZER_ALARM, OUTPUT);
  
        //Initializing global variables/serial communication
        Serial.begin(115200);
        TurnSystemOFF();
        
        // Defining initial password
        command[0]='1'; command[1]='1'; command[2]='1'; command[3]='1'; command[4]='|';
        ChangePassword(0);
}

void loop()
{ 
        char i = 0; // Reset array position
        command[i] = '%'; // Clear command buffer
           
        //Teste é feito de acordo com disponibilidade dos meios de comunicação, caso bluetooth esteja disponível, enviamos 0 para a função getCommand
        //caso o GSM esteja disponível, enviamos 1.
        if (Serial.available()) {
                Serial.write("Bluetooth is available.."); 
                GetCommand(0); 
                Serial.write("Saiu do metodo de leitura");
        } /*else if (gsm.available()) {
                GetCommand(1);
        } */
        
        if(VerifyPassword(&i)){
                do {
                        switch (command[i]) {
                        case '0': 
                                TurnSystemOFF();
                                i++;
                                break;
                        case '1': 
                                TurnSystemON();
                                i++;
                                break;
                        case '2': //GetSystemStatus();
                                break;
                        case '3': //ChangePassword();
                                break;
                        case '4': i = NewUser(i+1);
                                break;
                        default: ;
                        }
                } while (command[i]!='%');
        }
            
        if (systemStatus) {
                if (digitalRead(PIR_SENSOR) || digitalRead(TILT_SENSOR)) {
                        // Send message to car owner
                        digitalWrite(BUZZER_ALARM, HIGH);
                }
        }
}

void GetCommand(char serialSource)
{
        char character;
        char i;
    
        if (serialSource == 0) {
                Serial.write("Iniciando leitura Bluetooth...\n");
        
                for (i=0; character != '%'; i++) {
                        do {
                                character = Serial.read();
                        } while (character==-1);
                        command[i] = character; 
                        Serial.write("Caracter lido:");
                        Serial.write(character);
                        Serial.write("\n");
                }
                command[i] = '%';
                Serial.write("Leitura finalizada...\n");
                Serial.flush();
        } else {
                /*character = sms.read();
                while(character==-1){character = sms.read();}
                
                command[0] = character;
                for(i=1; character != '%'; i++){
                        while(!sms.available());
                        command[i]  = sms.read();  
                }
                command[i] = '%'; 
                sms.flush();*/
        }
}

char ChangePassword(char arrayPosition)
{
        unsigned short i;
        
        for (i=0; command[arrayPosition]!='|'; i++) {
                EEPROM.write(i, command[arrayPosition]);
                Serial.write("\nEscrevendo "); Serial.write(command[arrayPosition]); Serial.write(" no endereco "); Serial.write(i);
                arrayPosition++;
        }
        
        if (i!=9) {
                EEPROM.write(i, '|');
                Serial.write("\nEscrevendo "); Serial.write(command[arrayPosition]); Serial.write(" no endereco "); Serial.write(i);
        }
        
        Serial.write("O conteudo da eeprom:\n");
        for(int j=0; EEPROM.read(i)!='|'; j++)
                Serial.write(EEPROM.read(i));
        
        return arrayPosition+1;
}

char VerifyPassword(char *i)
{
        char verified = 0;
    
        if (command[*i]!='%') {
                for (; *i<=9 && command[*i]!='|'; *i++) {
                        if (command[*i] == EEPROM.read(*i))
                                verified = 1;
                        else
                                verified = 0;
                }
             
                if(verified && (command[*i] != EEPROM.read(*i)))
                        verified = 0;   
        }
        
        *i++;
        return verified;
}

void TurnSystemON(void)
{
        systemStatus = ON;
        Serial.write("System is turning on");
}

void TurnSystemOFF(void)
{
        systemStatus = OFF;
        digitalWrite(BUZZER_ALARM, LOW);
        Serial.write("System is turning off");
}

char NewUser(char arrayPosition)
{
    
  
}
