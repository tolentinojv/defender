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
//        - A senha sempre começa no endereço 0 da flash
//        - O primeiro numero de telefone esta no endereço 0x0A
//        - O segundo numero de telefone esta no endereço 0x18
//        - O terceiro numero de telefone esta no endereço 0x26
//        - O quarto numero de telefone esta no endereço 0x34
//        - O quinto numero de telefone esta no endereço 0x42


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
char CommandIsValid(void);
char PasswordIsValid(char *);
void TurnSystemON(void);
void TurnSystemOFF(void);
void GetSystemStatus(void);
void ChangePassword(char *);
void CreateNewUsers(char *);
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
        command[0]='1'; command[1]='2'; command[2]='3'; command[3]='4'; command[4]='|';
        char i = 0;
        ChangePassword(&i);
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
        
        if(CommandIsValid() && PasswordIsValid(&i)){
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
                        case '2': 
                                //GetSystemStatus();
                                break;
                        case '3':
                                i+=2; 
                                ChangePassword(&i);
                                break;
                        case '4':
                                i+=2; 
                                CreateNewUsers(&i);
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

// INPUT: Ponteiro para o idx da senha no command[]
// OUTPUT: none
void ChangePassword(char *i)
{
        int address; // Variavel utilizada para caminhar pel
        
        for (address=0; command[*i]!='|' && command[*i]!='%'; address++) {
                EEPROM.write(address, command[*i]);
                //Serial.write("\nEscrevendo "); Serial.write(command[*i]); Serial.write(" no endereco "); Serial.write((char)(((int)'0')+address));
                //Serial.write("\nIndice atual e: "); Serial.write((char)(((int)'0')+*i));
                (*i)++;
        }
        //Serial.write("\nNa saida do loop o indice atual e: "); Serial.write((char)(((int)'0')+*i));
        
        // Escrevendo '|' no ultimo endereço, podendo ser 8 ou anterior
        EEPROM.write(address, '|');
        //Serial.write("\nEscrevendo '|' no endereco "); Serial.write((char)(((int)'0')+address));
        
        //Serial.write("\nO conteudo da eeprom:\n");
        //for(int j=0; EEPROM.read(j)!='|'; j++)
                //Serial.write(EEPROM.read(j));
        
        // Incrementando indice *i para o proximo comando
        if (command[*i]=='|')
                (*i)++;
        //Serial.write("\nA posicao do comando que esta sendo retornada e: "); Serial.write((char)(((int)'0')+*i));
}

char CommandIsValid(void)
{
        char tmp;
        
        if (command[0]=='%')
                tmp = 0;
        else
                tmp = 1;
        
        return tmp;
}

char PasswordIsValid(char *i)
{
        char verified = 1;
        
        for (; (*i)<=9 && command[*i]!='|' && verified; (*i)++) {
                Serial.write("\nComparando command "); Serial.write(command[*i]); Serial.write(" com EEPROM "); Serial.write(EEPROM.read(*i)); Serial.write(" no endereco "); Serial.write((char)(((int)'0')+*i));
                if (command[*i] == EEPROM.read(*i))
                        verified = 1;
                else
                        verified = 0;
        }
        
        Serial.write("\nUltimo teste comparando command "); Serial.write(command[*i]); Serial.write(" com EEPROM "); Serial.write(EEPROM.read(*i)); Serial.write(" no endereco "); Serial.write((char)(((int)'0')+*i));    
        if (verified && (command[*i] != EEPROM.read(*i)) && (*i)!=10)
                verified = 0;
        
        (*i)++;
        Serial.write("\nO retorno da funcao no endereco "); Serial.write((char)(((int)'0')+*i)); Serial.write(" sera "); Serial.write((char)(((int)'0')+verified));
        return verified;
}

void TurnSystemON(void)
{
        systemStatus = ON;
        Serial.write("\nSystem is turning on");
}

void TurnSystemOFF(void)
{
        systemStatus = OFF;
        digitalWrite(BUZZER_ALARM, LOW);
        Serial.write("\nSystem is turning off");
}

// INPUT: Ponteiro para o idx da senha no command[]
// OUTPUT: none
void CreateNewUsers(char *i)
{
        while (command[*i] == '+') {
                
                
                
        }
  
}
