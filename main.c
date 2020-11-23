#include "main.h"
#define FISHLEN 40
void main (void)
{
    int i;
    char  CG_fish[FISHLEN]={0x00,0x11,0x1A,0x14,0x14,0x1A,0x11,0x00,	//	Fish tail (right)
                            0x00,0x1C,0x02,0x01,0x01,0x02,0x1C,0x00,	//	Fish head (right)
                            0x00,0x08,0x14,0x08,0x00,0x02,0x05,0x02,	//	Bubbles
                            0x00,0x07,0x08,0x10,0x10,0x08,0x07,0x00,	//	Fish head (left)
                            0x00,0x11,0x0B,0x05,0x05,0x0B,0x11,0x00		//	Fish tail (left)
                            };
    char fishR[3] = {0, 1, 2};
    char fishL[3] = {2, 3, 4};
	
    char control[7] = {0x38, 0x38, 0x38, 0xe, 0x6, 0x1, 0x40};
    char control2[3] = {0x01, 0x38, 0x14};
    // set CGRAM=0x40
    TRISFbits.TRISF3 = 1; // RF3 (SW0) configured as input
    TRISFbits.TRISF5 = 1; // RF5 (SW1) configured as input
    TRISBbits.TRISB9 = 1; // RB9 (SW7) configured as input
    ANSELBbits.ANSB9 = 0; // RB9 (SW7) disabled analog
    TRISBbits.TRISB14 = 0; //speaker configured output
    ANSELBbits.ANSB14 = 0; //speaker disabled analog
    
    TRISBbits.TRISB15 = 0; // RB15 (DISP_RS) set as an output
    ANSELBbits.ANSB15 = 0; // disable analog functionality on RB15 (DISP_RS)
    TRISDbits.TRISD5 = 0; // RD5 (DISP_RW) set as an output
    TRISDbits.TRISD4 = 0; // RD4 (DISP_EN) set as an output
    TRISE &= 0xff00;
    ANSELEbits.ANSE2 = 0;
    ANSELEbits.ANSE4 = 0;
    ANSELEbits.ANSE5 = 0;
    ANSELEbits.ANSE6 = 0;
    ANSELEbits.ANSE7 = 0;
    PORTBbits.RB15 = 0;		//rs=0
    PORTDbits.RD5 = 0;		//w=0
	
	//	iterate over the control array
    for(i = 0; i < 7; i++)
    {
        PORTE=control[i];
        PORTDbits.RD4=1;
        PORTDbits.RD4=0;
        busy();
    }
	
	//	writing the fish characters to the CGRAM
    PORTBbits.RB15 = 1;		//rs
    for(i = 0; i < FISHLEN; i++)
    {
        PORTE=CG_fish[i];
        PORTDbits.RD4 = 1;	//enable=1
        PORTDbits.RD4 = 0;	//enable=0
        busy();
    }
		
    PORTBbits.RB15 = 0;	//rs control 
    PORTE = 0x80;			// change to DDRAM
    PORTDbits.RD4 = 1;	//enable=1
    PORTDbits.RD4 = 0;	//enable=0
    busy();
    
    int counter = 0;
    while(1)
    {
        if (PORTBbits.RB9)				// if SW7 is ON -> generate sound
        {
            int num = 0; 
            while(num++ < 10)
            {
				PORTBbits.RB14--;
				for(i = 0; i < 500 ; i++)   // delay between the two beeps
					;
				PORTBbits.RB14++;
				for(i = 0; i < 500; i++)   // delay between the two beeps
					;
			}
        }
        
        PORTBbits.RB15 = 0;	//rs=0
        PORTDbits.RD5 = 0;	//w=0
        for(i = 0; i < 2; i++)		//	clear the LCD
        {
           PORTE = control2[i];
           PORTDbits.RD4 = 1;
           PORTDbits.RD4 = 0;
           busy();
        }
        
        if(PORTFbits.RF5)			// SW1 is ON  -> move the cursor to the bottom line of LCD
        {
           PORTE = 0xc0;
           PORTDbits.RD4 = 1;
           PORTDbits.RD4 = 0;
           busy();
        }
        
        for(i = 0; i < counter; i++)	//	shift the cursor to the right position on the LCD
        {
            PORTE = control2[2];		//	control2[2] = 0x14 which means shift the cursor to the right
            PORTDbits.RD4 = 1;
            PORTDbits.RD4 = 0;
            busy();
        }
		
        PORTBbits.RB15 = 1;
        for(i = 0; i < 3; i++)			//	iterate over the char array to write to the LCD
		{
            if(PORTFbits.RF3)			//	if SW0 is ON -> print the right characters of the fish
                PORTE = fishR[i];
            else						//	otherwise -> print the left characters of the fish
                PORTE=fishL[i];
            PORTDbits.RD4 = 1;			//enable=1
            PORTDbits.RD4 = 0;			//enable=0
            busy();
        }
		
		counter = PORTFbits.RF3 ? counter + 1 : counter - 1;	//	if SW0 is ON -> shift right
																//	otherwise -> shift left
		//	reset the counter when fish is out of line
        if (counter == 16 )
            counter = 0;
        
        if (counter == -1)
            counter = 15;
        
		//	delay
        for(i = 0; i < 200000; i++)
			;
    }
}

void busy(void)
{
    char RD,RS;
    int STATUS_TRISE;
    int portMap;
    RD = PORTDbits.RD5;
    RS = PORTBbits.RB15;
    STATUS_TRISE = TRISE;
	PORTDbits.RD5 = 1;		//w/r
	PORTBbits.RB15 = 0;		//rs 
    portMap = TRISE;
	portMap |= 0x80;
	TRISE = portMap;
    do
    {
        PORTDbits.RD4 = 1;	//enable=1
        PORTDbits.RD4 = 0;	//enable=0
    } while(PORTEbits.RE7); 	// BF ?????
	
    PORTDbits.RD5 = RD; 
    PORTBbits.RB15 = RS;
    TRISE = STATUS_TRISE;   
}