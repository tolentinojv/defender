//**** 0. Documentation Section
//  Car security system for use with smartphones
//  Author: João Victor Tolentino
//  Email: tolentino.jv@gmail.com
//  Date: 2/20/2014
//  OBS1: System codes:
//        0 - Desliga o sistema
//        1 - Liga o sistema
//        2 - Solicita status do sistema
//        3 - Troca de senha
//        4 - Adicionar novo usuário (telefone 13 digitos)
//        5 - Remover usuário
//        6 - Sincroniza aplicativo com veículo
//        7 - Limpa todas as informaçoes na memoria
//  OBS2: EEPROM Codes
//        - A senha sempre começa no endereço 0 da flash
//        - O primeiro numero de telefone esta no endereço 0x0A
//        - O segundo numero de telefone esta no endereço 0x18
//        - O terceiro numero de telefone esta no endereço 0x26
//        - O quarto numero de telefone esta no endereço 0x34
//        - O quinto numero de telefone esta no endereço 0x42
// OBS3: Pin map
//        0     Bluetooth Rx
//        1     Bluetooth Tx
//        2     GSM module Rx
//        3     GSM module Tx
//        4     GPS module Rx
//        5     GPS module TX
//        6     -
//        7     OPTOCOUPLER
//        8     -
//        9     -
//        10    PIR_SENSOR
//        11    TILT_SENSOR
//        12    BUZZER_ALARM
//        13    -


// 1. Pre-processor Directives Section
//  INCLUDES
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
//  DEFINES
#define COMMAND_SIZE   100
#define ON             1
#define OFF            0
#define OPTOCOUPLER    7
#define PIR_SENSOR     10
#define TILT_SENSOR    11
#define BUZZER_ALARM   12


// 2. Global Declarations section
// GLOBAL VARIABLES
int systemStatus;
char command[COMMAND_SIZE];
char usr[5];
SoftwareSerial gpsSerial(4, 5); // create gps sensor connection
TinyGPS gps; // create gps object

// FUNCTION PROTOTYPES
void GetCommand(char);
char CommandIsValid(void);
char PasswordIsValid(char *);
void TurnSystemON(void);
void TurnSystemOFF(void);
void GetSystemStatus(void);
void ChangePassword(char *);
void CreateNewUsers(char *);
void NewUser(char *, char);
void RemoveUser(char *);
char GetMemoryAddress(char);
void ResetConfig(char *);


// 3. Subroutines Section
// Rotina de configuração do uC
void setup()
{       
        // Defining data inputs
        pinMode(PIR_SENSOR, INPUT);
        pinMode(TILT_SENSOR, INPUT);
  
        // Defining data outputs
        pinMode(OPTOCOUPLER, OUTPUT);
        pinMode(BUZZER_ALARM, OUTPUT);
  
        //Initializing global variables/serial communication
        usr[0] = usr[1] = usr[2] = usr[3] = usr[4] = 0; //
        Serial.begin(9600);
        gpsSerial.begin(4800);
        TurnSystemOFF();
        
        // Reseting configuration to default
        command[0]='1'; command[1]='2'; command[2]='3'; command[3]='4'; command[4]='|';
        char i = 0;
        ChangePassword(&i);
}

// Loop principal do sistema
void loop()
{ 
        char i = 0; // Reset array position
        command[i] = '%'; // Clear command buffer
        
        //Teste é feito de acordo com disponibilidade dos meios de comunicação, caso bluetooth esteja disponível, enviamos 0 para a função getCommand
        //caso o GSM esteja disponível, enviamos 1.
        if (Serial.available()) {
                //Serial.write("Bluetooth is available.."); 
                GetCommand(0); 
                //Serial.write("Saiu do metodo de leitura");
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
                                i++;
                                GetSystemStatus();
                                break;
                        case '3':
                                i+=2; 
                                ChangePassword(&i);
                                break;
                        case '4':
                                i+=2; 
                                CreateNewUsers(&i);
                                break;
                        case '5':
                                i+=2;
                                RemoveUser(&i);
                                break;
                        case '6':
                                //Sync()
                                break;
                        case '7':
                                ResetConfig(&i);// Comando so pode ser utilizado individualmente
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

// INPUT: Porta serial utilizada
// OUTPUT: none
void GetCommand(char serialSource)
{
        char character;
        char i;
    
        if (serialSource == 0) {
                Serial.write("\nIniciando leitura Bluetooth...\n");
        
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
                Serial.write("\nEscrevendo "); Serial.write(command[*i]); Serial.write(" no endereco "); Serial.write((char)(((int)'0')+address));
                Serial.write("\nIndice atual e: "); Serial.write((char)(((int)'0')+*i));
                (*i)++;
        }
        Serial.write("\nNa saida do loop o indice atual e: "); Serial.write((char)(((int)'0')+*i));
        
        // Escrevendo '|' no ultimo endereço, podendo ser 8 ou anterior
        EEPROM.write(address, '|');
        Serial.write("\nEscrevendo '|' no endereco "); Serial.write((char)(((int)'0')+address));
        
        Serial.write("\nO conteudo da eeprom:\n");
        for(int j=0; EEPROM.read(j)!='|'; j++)
                Serial.write(EEPROM.read(j));
        
        // Incrementando indice *i para o proximo comando
        if (command[*i]=='|')
                (*i)++;
        Serial.write("\nA posicao do comando que esta sendo retornada e: "); Serial.write((char)(((int)'0')+*i));
}

// INPUT: none
// OUTPUT: Valor booleano referente a validade ou não do comando enviado
char CommandIsValid(void)
{
        char tmp;
        
        if (command[0]=='%')
                tmp = 0;
        else
                tmp = 1;
        
        return tmp;
}

// INPUT: Ponteiro para o idx da senha no command[]
// OUTPUT: none
char PasswordIsValid(char *i)
{
        char verified = 1;
        
        for (; (*i)<=9 && command[*i]!='|' && verified; (*i)++) {
                //Serial.write("\nComparando command "); Serial.write(command[*i]); Serial.write(" com EEPROM "); Serial.write(EEPROM.read(*i)); Serial.write(" no endereco "); Serial.write((char)(((int)'0')+*i));
                if (command[*i] == EEPROM.read(*i))
                        verified = 1;
                else
                        verified = 0;
        }
        
        //Serial.write("\nUltimo teste comparando command "); Serial.write(command[*i]); Serial.write(" com EEPROM "); Serial.write(EEPROM.read(*i)); Serial.write(" no endereco "); Serial.write((char)(((int)'0')+*i));    
        if (verified && (command[*i] != EEPROM.read(*i)) && (*i)!=10)
                verified = 0;
        
        (*i)++;
        //Serial.write("\nO retorno da funcao no endereco "); Serial.write((char)(((int)'0')+*i)); Serial.write(" sera "); Serial.write((char)(((int)'0')+verified));
        return verified;
}

// INPUT: none
// OUTPUT: none
void TurnSystemON(void)
{
        systemStatus = ON;
        digitalWrite(OPTOCOUPLER, LOW);
        Serial.write("\nSystem is turning on");
}

// INPUT: none
// OUTPUT: none
void TurnSystemOFF(void)
{
        systemStatus = OFF;
        digitalWrite(BUZZER_ALARM, LOW);
        digitalWrite(OPTOCOUPLER, HIGH);
        Serial.write("\nSystem is turning off");
}

// INPUT: Ponteiro para o idx no command[] onde o primeiro usuário a ser adicionado está localizado
// OUTPUT: none
void CreateNewUsers(char *i)
{
        char usrPos;

        usrPos = FindEmptyUsr();
        NewUser(i, usrPos);
        
        if(usrPos == 5 && command[(*i)+13]=='%')
                (*i)+=13;
        else if (usrPos == 5 && command[(*i)+13]=='|')
                (*i)+=14;
        else if(usrPos == 5 && command[(*i)+14]=='%')
                (*i)+=14;
        else if (usrPos == 5 && command[(*i)+14]=='|')
                (*i)+=15;
}

// INPUT:  Ponteiro para o idx no command[]
//         Indice de usuário
// OUTPUT: none
void NewUser(char *i, char usrPos)
{
        char address;
        char temp;

        address = GetMemoryAddress(usrPos);
        
        Serial.write("\nCadastrando novo usuario no endereco de memoria ");Serial.print(address);
        
        if (address != 0) {
                temp = address;
                // Erro no while 
                while (command[*i]!='|' && command[*i]!='%' && command[*i]!=',') {
                        EEPROM.write(address, command[*i]);
                        Serial.write("\nEscrevendo "); Serial.write(command[*i]); Serial.write(" no endereco "); Serial.write(address);
                        address++;
                        (*i)++;
                }

                if (address-temp == 13) {// +xx xx xxxxx xxxx 
                        EEPROM.write(address, 'x');
                        Serial.write("\nEscrevendo x no endereco "); Serial.write((char)(((int)'0')+address));
                }

                usr[usrPos] = 1;
                
                if(command[*i] == '|' || command[*i] == ',')
                        (*i)++;
        }
}

// INPUT:  Ponteiro para o idx no command[] onde esta localizado o indice do usuário a ser apagado
// OUTPUT: none
void RemoveUser(char *i)
{
        char idx = command[*i] - '0';
        int address, lastAddress;

        if (idx>=0 && idx<=4) {
                Serial.write("\nRemovendo o usuario: "); Serial.write(command[*i]);
                usr[idx] = 0;

                address = GetMemoryAddress(idx);
                lastAddress = address + 13; // O espaço reservado para um usuario na 
                                        // memória é de 14 bytes, ou seja, o endereço 
                                        // apontado atualmente mais 13 bytes

                while (address != lastAddress) {
                        EEPROM.write(address, 0);
                        address++;
                }
        }

        if(command[(*i)+1] == '%')
                (*i)++;
        else
                (*i)+=2;
}

// INPUT:  Indice do usuário
// OUTPUT: Endereço de memória
char GetMemoryAddress(char idx)
{
        char address;

        switch (idx) {
        case 0:
                address = 10;
                break;
        case 1:
                address = 24;
                break;
        case 2:
                address = 38;
                break;
        case 3:
                address = 52;
                break;
        case 4:
                address = 66;
                break;
        default:
                address = 0;
        }

        return address;
}

// INPUT:  none
// OUTPUT: Indice do usuário
char FindEmptyUsr(void) {
        char tmp = 5;

        for (int i=0; i<5 && tmp==5; i++) {
                if (usr[i] == 0)
                        tmp = i;
        }
        
        Serial.write("\nO usuario vazio encontrado foi: ");Serial.write((char)(((int)'0')+tmp));
        
        return tmp;
}

// INPUT:  none
// OUTPUT: none
void ResetConfig(char *i){
        for (int address=0; address<1024; address++)
                EEPROM.write(address, 0);
        
        //Serial.write("\nEEPROM was clared");
        
        usr[0] = usr[1] = usr[2] = usr[3] = usr[4] = 0; // Reset users
        command[0]='1'; command[1]='2'; command[2]='3'; command[3]='4'; command[4]='|'; 
        *i = 0; // Changing value to reset
        ChangePassword(i); // Reset password
        *i = 0; // Reset command index
        command[*i] = '%';
}

// INPUT:  none
// OUTPUT: none
void GetSystemStatus(void)
{
        long lat,lon; // create variable for latitude and longitude object
        float speed;
        char c;
        char *cardinal;
        
        gpsSerial.listen();
        
        Serial.println("Entrou no get status");

        while (!gpsSerial.available());
        
        Serial.println("GPS esta disponivel.");
        
        do {
                c = gpsSerial.read();                
        } while(!gps.encode(c)); // encode gps data
                
        gps.get_position(&lat,&lon); // get latitude and longitude
        speed = gps.f_speed_kmph();
        *cardinal = *gps.cardinal(gps.f_course());
        // display position
        Serial.print("Position: ");
        Serial.print("lat: ");Serial.print(lat);Serial.print(" ");// print latitude
        Serial.print("lon: ");Serial.println(lon); // print longitude
        Serial.print("Course (degrees): "); Serial.println(gps.f_course()); 
        // And same goes for speed
        Serial.print("Speed(kmph): "); Serial.println();
}

