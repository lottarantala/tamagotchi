/*
 * project_main.c
 *
 * Authors: Lotta Rantala and Venla Katainen
 *
 */


/* C Standard library */
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>


/* XDCtools files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <driverlib/timer.h>


/* Board Header files */
#include "Board.h"
#include "wireless/comm_lib.h"
#include "sensors/opt3001.h"
#include "sensors/buzzer.h"
#include "sensors/mpu9250.h"
#include "sensors/tmp007.h"


void musiikkiFunktioLiikunta(void);
void musiikkiFunktioEnergia(void);
void musiikkiFunktioVaroitus(void);
void musiikkiFunktioAktivointi(void);

/* Task */
#define STACKSIZE 4096
Char sensorTaskStack[STACKSIZE];
Char uartTaskStack[STACKSIZE];
Char commTaskStack[STACKSIZE];


//Tilakone
enum state {DATA_READ=1, DATA_READY, RUOKINTA_TILA, LIIKUNTA_TILA, ENERGIA_TILA, HOIVA_TILA, AKTIVOINTI_TILA, 
            VAROITUS, PARTY};
enum state programState = DATA_READ;

//Sisainen tilakone
enum tila {TILA_0=1, TILA_PAIKALLAAN, TILA_LIIKKUU, TILA_VALO, TILA_PIMEA, TILA_LAMPO};
enum tila sisainenState = TILA_0;

//Global variables for pins
static PIN_Handle buttonHandle;
static PIN_State buttonState;
static PIN_Handle ledHandle;
static PIN_State ledState;
static PIN_Handle punainen_ledHandle;
static PIN_State punainen_ledState;
static PIN_Handle buzzer;
static PIN_State buzzerState;
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Handle powerButtonHandle;
static PIN_State powerButtonState;


//Global variables
double ambientLight = -1000.0;
double lampotila = 0.0;



//Pin configs
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

PIN_Config buttonConfig[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE, 
    PIN_TERMINATE
};

PIN_Config ledConfig[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

PIN_Config punainen_ledConfig[] = {
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

PIN_Config buzzerConfig[] = {
    Board_BUZZER     | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,     /* Buzzer initially off          */
    PIN_TERMINATE
};

PIN_Config powerButtonConfig[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};
PIN_Config powerButtonWakeConfig[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
   PIN_TERMINATE
};


Void powerFxn(PIN_Handle handle, PIN_Id pinId) {

   // Odotetana hetki ihan varalta..
   Task_sleep(100000 / Clock_tickPeriod);

   // Taikamenot
   PIN_close(powerButtonHandle);
   PINCC26XX_setWakeup(powerButtonWakeConfig);
   Power_shutdown(NULL,0);
}


void buttonFxn(PIN_Handle handle, PIN_Id pinId) {
    //kun painetaan nappia, tamagotchia ruokitaan
    programState = RUOKINTA_TILA;

}


void musiikkiFunktioLiikunta(void){
    //kun tamagotchi liikkuu, soi tama musiikki
    
    buzzerOpen(buzzer);
    buzzerSetFrequency(262);  //c
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(294);  //d
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(349);  //f
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(392);  //g
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);  //g
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(440);  //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(440);   //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);  //g
    Task_sleep(1000000/Clock_tickPeriod);
    buzzerSetFrequency(440);  //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(440);    //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);  //g
    Task_sleep(1000000/Clock_tickPeriod);
    buzzerClose();
}

void musiikkiFunktioEnergia(void){
    //kun tamagotchi nukkuu soi tama musiikki
    
    buzzerOpen(buzzer);
    buzzerSetFrequency(262);  //c
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(262);  //c
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(262);  //c
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(294); //d
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(294);  //d
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(294);  //d
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(349);  //f
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(330); //e
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(294);  //d
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(294);  //d
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(262);  //c
    Task_sleep(500000/Clock_tickPeriod);
    buzzerClose();
}

void musiikkiFunktioVaroitus(void){
    //kun jokin arvo on alle 2, tamagotchi lahettaa taman varoitus aanen
    
    buzzerOpen(buzzer);
    buzzerSetFrequency(277);
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(185);
    Task_sleep(250000/Clock_tickPeriod);
    buzzerClose();
}

void musiikkiFunktioAktivointi(void){
    //kun tamagotchia aktivoidaan soi tama musiikki
    
    buzzerOpen(buzzer);
    buzzerSetFrequency(494);  //h
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(440);  //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);  //g
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(494);   //h
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(440);    //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);    //g
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);    //g
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(392);    //g
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(392);    //g
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(392);    //g
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(440);    //a
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(440);    //a
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(440);    //a
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(440);    //a
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(494);  //h
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(440);  //a
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(392);  //g
    Task_sleep(500000/Clock_tickPeriod);
    buzzerClose();
}

void musiikkiFunktioParty(){
    
    buzzerOpen(buzzer);
    buzzerSetFrequency(330);  //e
    Task_sleep(1500000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(250000/Clock_tickPeriod);
    buzzerSetFrequency(330);  //e
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(262);  //c
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(262);  //c
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(262);  //c
    Task_sleep(500000/Clock_tickPeriod);
    buzzerSetFrequency(294);  //d
    Task_sleep(1000000/Clock_tickPeriod);
    buzzerClose();
}


/* Task Functions */

Void uartTaskFxn(UArg arg0, UArg arg1) {

    Task_sleep(100000 / Clock_tickPeriod);
    char viesti[80];
    
    while (1) {
        
        if (programState == DATA_READY){
            programState = DATA_READ;
        }
        
        else if (programState == DATA_READ){
            sprintf(viesti, "%s", "MSG1: ,MSG2: ");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            programState = DATA_READ;
        }
                    
        else if(programState == RUOKINTA_TILA){
            //tamagotchi syo
            sprintf(viesti, "%s", "EAT:1");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            
            PIN_setOutputValue(ledHandle, Board_LED0, 1);
            programState = DATA_READ;
        }
        
        else if(programState == LIIKUNTA_TILA){
            //tamagotchi liikkuu
            sprintf(viesti, "%s", "EXERCISE:1");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            
            programState = DATA_READ;
        }
        
        else if(programState == HOIVA_TILA){
            //punainen led syttyy hoivatilan merkiksi
            sprintf(viesti, "%s", "PET:1");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            
            PIN_setOutputValue(punainen_ledHandle, Board_LED1, 1);
            programState = DATA_READ;
        }
        
        else if(programState == ENERGIA_TILA){
            //tamagotchi nukkuu jonka jalkeen data read tilaan
            sprintf(viesti, "%s", "PET:1,MSG1:sleep");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            
            programState = DATA_READ;
        }
        
        else if (programState == AKTIVOINTI_TILA){
            //tamagotchia aktivoidaan jonka jalkeen siirrytaan data read tilaan
            
            sprintf(viesti, "%s", "ACTIVATE:1;1;1");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            
            programState = DATA_READ;
        }
        
        else if (programState == PARTY){
            
            sprintf(viesti, "%s", "EXERCISE:2,EAT:2,MSG2:Party!");
            Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
            StartReceive6LoWPAN();
            
            PIN_setOutputValue(punainen_ledHandle, Board_LED1, 1);
            Task_sleep(500000 / Clock_tickPeriod);
            PIN_setOutputValue(ledHandle, Board_LED0, 1);
            
            musiikkiFunktioParty();
            
            programState = DATA_READ;
        }
        
        else if (programState == VAROITUS){
            //tamagotchi karkaa
            PIN_setOutputValue(punainen_ledHandle, Board_LED1, 1);
            PIN_setOutputValue(ledHandle, Board_LED0, 1);
            
            musiikkiFunktioVaroitus();
            
            programState = DATA_READ;
        }
        
        // Once per second, you can modify this
        Task_sleep(1000000 / Clock_tickPeriod);
    }
}

Void sensorTaskFxn(UArg arg0, UArg arg1) {
    //valosensori
    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    
    //liikesensori
    float ax, ay, az, gx, gy, gz;

	I2C_Handle i2cMPU; // liikesensorin oma i2c vayla
	I2C_Params i2cMPUParams;

    I2C_Params_init(&i2cMPUParams);
    i2cMPUParams.bitRate = I2C_400kHz;
    
	// Note the different configuration below
    i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;
    
    // MPU power on
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_ON);
    
    // Wait 100ms for the MPU sensor to power up
    Task_sleep(100000 / Clock_tickPeriod);
    System_printf("MPU9250: Power ON\n");
    System_flush();
    
    i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
    if (i2cMPU == NULL) {
        System_abort("Error Initializing I2CMPU\n");
    }

    // MPU setup and calibration
	System_printf("MPU9250: Setup and calibration...\n");
	System_flush();
	
	mpu9250_setup(&i2cMPU);
	I2C_close(i2cMPU);

	System_printf("MPU9250: Setup and calibration OK\n");
	System_flush();

	//valosensorin i2c vaylan avaus
	i2c = I2C_open(Board_I2C, &i2cParams);
	if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    
    // OPT3001 setup and calibration
	System_printf("OPT3001: Setup and calibration...\n");
	System_flush();
    opt3001_setup(&i2c);
	I2C_close(i2c);
    System_printf("OPT3001: Setup and calibration OK\n");
	System_flush();

	//lampotilasensorin i2c vaylan avaus
	i2c = I2C_open(Board_I2C, &i2cParams);
	if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
	
	//TMP007 setup and calibration
	System_printf("TMP007: Setup and calibration...\n");
	System_flush();
    tmp007_setup(&i2c);
	I2C_close(i2c);
    System_printf("TMP007: Setup and calibration OK\n");
	System_flush();
	
	char merkkijono_liike[80];
	char viesti[80];
	char valo[16];
	double arvot[10];
    int i = 0;
    
    while (1) {
        
        if(programState == DATA_READ){
            
            PIN_setOutputValue(punainen_ledHandle, Board_LED1, 0);
            PIN_setOutputValue(ledHandle, Board_LED0, 0);
            
            //avataan valosensorin i2c vayla
            i2c = I2C_open(Board_I2C, &i2cParams);
            
            //kerataan dataa valosensorilta
            ambientLight = opt3001_get_data(&i2c);
            
            //suljetaan valosensorin i2c vayla
            I2C_close(i2c);
            
            //avataan lampotilasensorin i2c vayla
            i2c = I2C_open(Board_I2C, &i2cParams);
            
            //kerataan dataa lampotilasensorilta
            lampotila = tmp007_get_data(&i2c);
            arvot[i] = lampotila;
            //suljetaan lampotilasensorin i2c vayla
            I2C_close(i2c);
            
            //avataan liikesensorin i2c vayla
            i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
    
    	    // kerataan dataa liikesensorilta
    		mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
    		
    		//suljetaan liikesensorin i2c vayla
            I2C_close(i2cMPU);
            
            //lampotilan raakadatan tulostus taustajarjestelmaan
            if (i == 9){
                int n=0;
                sprintf(viesti, "%s", "session:start");
                Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
                StartReceive6LoWPAN();

                for(n=0; n<10; n++){
                    sprintf(viesti, "time:%d,temp:%lf", n, arvot[n]);
                    Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
                    StartReceive6LoWPAN();
                }

                sprintf(viesti, "%s", "session:end");
                Send6LoWPAN(IEEE80154_SERVER_ADDR, viesti, strlen(viesti));
                StartReceive6LoWPAN();
                
                int i = 0;
                
            }
            
            i++;
            
    	    //asetetaan tilakone tilaan data_ready, kun sensoreiden mittausdata on saatu
    		programState = DATA_READY;
        }
        
    	if(programState == DATA_READY){
    	    // asetetaan sisainen tilakone alkutilaa, kun data on valmis
    	    sisainenState = TILA_0;

    	} 
    	
        if(ax < -0.5 && sisainenState == TILA_0){
            //sensor tag on liikkeessa, joten vaihdetaan tilaa
            sisainenState = TILA_LIIKKUU;
        }
        
        if (sisainenState == TILA_LIIKKUU){
            //tunnistettu, että tamagotchi on liikuntatilassa
        	musiikkiFunktioLiikunta();
        	programState = LIIKUNTA_TILA;
        }
        
        if ((gy > 120 || gy < -120 || gx < -120 || gx > 120) && sisainenState == TILA_0 ){
            programState = PARTY;
        }
        
        if (sisainenState == TILA_0 && ax < 5 && ay < 10 && az < 5 && gx < 10 && gy < 10 && gz < 10){
            //sensor tag ei liiku
            sisainenState = TILA_PAIKALLAAN;
        }
        
        if(sisainenState == TILA_PAIKALLAAN && lampotila > 35.0){
            //sensor tagilla mitattu lampotila on haluttu
            sisainenState = TILA_LAMPO;
        }
        
        if (sisainenState == TILA_LAMPO){
            //tamagotchia hoivataan
        	programState = HOIVA_TILA;
        }
        
        if(ambientLight < 15 && sisainenState == TILA_PAIKALLAAN){
            //sensor tag on pimeassa
            sisainenState = TILA_PIMEA;
        }
        
        if (sisainenState == TILA_PIMEA){
            //tamagotchi nukkuu
            musiikkiFunktioEnergia();
            programState = ENERGIA_TILA;
            sisainenState = TILA_0;
    	}
    	
    	if (sisainenState == TILA_PAIKALLAAN && ambientLight > 100){
    	    // sensor tag on valoisassa
    	    sisainenState = TILA_VALO;
    	}
    	
    	if (sisainenState == TILA_VALO){
    	    //tamagotchia aktivoidaan
    	    musiikkiFunktioAktivointi();
    	    programState = AKTIVOINTI_TILA;
    	}
    	
        Task_sleep(1000000 / Clock_tickPeriod);
    }

    //liikesensori pois paalta
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_OFF);
}

Void commTask(UArg arg0, UArg arg1) {

   char payload[80]; // viestipuskuri
   uint16_t senderAddr;

   // Radio alustetaan vastaanottotilaan
   int32_t result = StartReceive6LoWPAN();
   if(result != true) {
      System_abort("Wireless receive start failed");
   }
   
   // Vastaanotetaan viestejä loopissa
   while (true) {

        // jos true, viesti odottaa
        if (GetRXFlag()) {

           // Tyhjennetään puskuri (ettei sinne jäänyt edellisen viestin jämiä)
           memset(payload,0,80);
           // Luetaan viesti puskuriin payload
           Receive6LoWPAN(&senderAddr, payload, 80);
           
           //tarkistetaan onko lemmikki karkaamassa
           if ((strncmp(payload, "12,BEEP", 7)) == 0 ){
               programState = VAROITUS;
           }
           
           // Tulostetaan vastaanotettu viesti konsoli-ikkunaan
           System_printf(payload);
           System_flush();
        }          
    }
}

int main(void) {

    // Task variables
    Task_Handle sensorTaskHandle;
    Task_Params sensorTaskParams;
    Task_Handle uartTaskHandle;
    Task_Params uartTaskParams;
    Task_Handle commTaskHandle;
    Task_Params commTaskParams;
    
    // Initialize board
    Board_initGeneral();
    Init6LoWPAN();

    //otetaan i2c vayla kayttoon
    Board_initI2C();
    
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
    	System_abort("Pin open failed!");
    }
    
    //otetaan painonappi ja led kayttoon
    ledHandle = PIN_open(&ledState, ledConfig);
    if(!ledHandle){
        System_abort("Error initializing Led pin\n");
    }
    
    buttonHandle = PIN_open(&buttonState, buttonConfig);
    if(!buttonHandle){
        System_abort("Error initializing button pin\n");
    }
    
    //virtakytkimen kayttoonotto
    powerButtonHandle = PIN_open(&powerButtonState, powerButtonConfig);
    if(!powerButtonHandle) {
        System_abort("Error initializing power button\n");
    }
    if (PIN_registerIntCb(powerButtonHandle, &powerFxn) != 0) {
        System_abort("Error registering power button callback");
    }
    
    if(PIN_registerIntCb(buttonHandle, &buttonFxn) != 0){
        System_abort("Error registering button callback function\n");
    }
    
    buzzer = PIN_open(&buzzerState, buzzerConfig);
    if (buzzer == NULL) {
    	System_abort("Pin open failed!");
    }
    
    /* Task */
    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = STACKSIZE;
    sensorTaskParams.stack = &sensorTaskStack;
    sensorTaskParams.priority=2;
    sensorTaskHandle = Task_create(sensorTaskFxn, &sensorTaskParams, NULL);
    
    if (sensorTaskHandle == NULL) {
        System_abort("Task create failed!");
    }
    
    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;
    commTaskHandle = Task_create(commTask, &commTaskParams, NULL);
    
    if (commTaskHandle == NULL) {
        System_abort("Task create failed!");
    }

    Task_Params_init(&uartTaskParams);
    uartTaskParams.stackSize = STACKSIZE;
    uartTaskParams.stack = &uartTaskStack;
    uartTaskParams.priority=2;
    uartTaskHandle = Task_create(uartTaskFxn, &uartTaskParams, NULL);
    
    if (uartTaskHandle == NULL) {
        System_abort("Task create failed!");
    }

    /* Start BIOS */
    BIOS_start();

    return (0);
}
