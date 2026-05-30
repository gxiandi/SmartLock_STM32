#include "stm32f10x.h"                 
#include "Delay.h"
#include "OLED.h"          
#include "Flash.h"
#include "KEY.h"           
#include "matrix_key.h"    
#include "BUZZER.h"
#include "Servo.h"
#include "RC522.h"
#include "AS608.h"
#include <string.h>       

#define CORRECT_PASSWORD "1234"          
#define PWD_LENGTH 4                     
#define OPEN_DOOR_TIME 3000              
#define SERVO_CLOSE_ANGLE 0.0f           
#define SERVO_OPEN_ANGLE 90.0f           

#define OLED_Y1 0     
#define OLED_Y2 16    
#define OLED_Y3 32    
#define OLED_Y4 48    
#define X_CENTER(s) (128 - (s)*8)/2  

uint8_t input_pwd[PWD_LENGTH + 1] = {0};
uint8_t input_index = 0;
uint8_t flag_pwd_input = 0;
uint8_t verify_flag = 0;

uint8_t flag_finger = 0;
uint8_t finger_verify_flag = 0;

uint8_t cardnumber,KeyNum,tempcard,select=0,flag_scan=1,flag_addcard=0,flag_deletecard=0;
extern uint8_t UID[4],Temp[4];
extern uint8_t UI0[4];
extern uint8_t UI1[4];
extern uint8_t UI2[4];
extern uint8_t UI3[4];

void RFID_Check(void);
void Read_Card(void);
void Show_Welcome_UI(void);
void Show_Pwd_Input(void);
uint8_t Verify_Password(void);
void Show_Verify_Result(uint8_t result);
void Close_Door_And_Reset(void);
void Key_Process(uint8_t key_val);
void Add_FR(void);
void press_FR(void);

void Show_Welcome_UI(void)
{
    OLED_Clear();
    OLED_ShowString(X_CENTER(12), OLED_Y1, "Access Control ", OLED_8X16);
    OLED_ShowString(X_CENTER(8), OLED_Y2, "Input PWD:", OLED_8X16);
    OLED_ShowString(8, OLED_Y3, "PWD: ", OLED_8X16);
    OLED_Update();
}

void Show_Pwd_Input(void)
{
    OLED_ShowString(40, OLED_Y3, "    ", OLED_8X16);
    for(uint8_t i = 0; i < input_index; i++)
    {
        OLED_ShowChar(40 + i*8, OLED_Y3, '*', OLED_8X16);
    }
    OLED_Update();
}

uint8_t Verify_Password(void)
{
    return strcmp((char*)input_pwd, CORRECT_PASSWORD) == 0 ? 1 : 0;
}

void Show_Verify_Result(uint8_t result)
{
    OLED_Clear();
    if(result == 1)
    {
        OLED_ShowString(X_CENTER(8), OLED_Y1, "PWD OK", OLED_8X16);
        OLED_ShowString(X_CENTER(8), OLED_Y2, "Door Open", OLED_8X16);
        Buzzer1();
        Servo_SetAngle(SERVO_OPEN_ANGLE);
        verify_flag = 1;
    }
    else
    {
        OLED_ShowString(X_CENTER(8), OLED_Y1, "PWD ERR", OLED_8X16);
        OLED_ShowString(X_CENTER(10), OLED_Y2, "Re-Input PWD", OLED_8X16);
        Buzzer_Alarm();
        Delay_ms(500);
        input_index = 0;
        memset(input_pwd, 0, sizeof(input_pwd));
        verify_flag = 0;
    }
    OLED_Update();
    
    if(result == 0)
    {
        Delay_ms(1500);
        Show_Welcome_UI();
    }
}

void Close_Door_And_Reset(void)
{
    Servo_SetAngle(SERVO_CLOSE_ANGLE);
    Buzzer1();
    OLED_Clear();
    OLED_ShowString(X_CENTER(8), OLED_Y1, "Door Close", OLED_8X16);
    OLED_Update();
    Delay_ms(1000);
    
    input_index = 0;
    memset(input_pwd, 0, sizeof(input_pwd));
    verify_flag = 0;
    flag_pwd_input = 0;
    
    OLED_Clear();
    OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
    OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
    OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
    OLED_Update();
}

void Key_Process(uint8_t key_val)
{
    if(verify_flag != 0 || flag_pwd_input == 0) return;
    
    switch(key_val)
    {
        case 1:  input_pwd[input_index] = '1'; break;
        case 2:  input_pwd[input_index] = '2'; break;
        case 3:  input_pwd[input_index] = '3'; break;
        case 4:  input_pwd[input_index] = '4'; break;
        case 5:  input_pwd[input_index] = '5'; break;
        case 6:  input_pwd[input_index] = '6'; break;
        case 7:  input_pwd[input_index] = '7'; break;
        case 8:  input_pwd[input_index] = '8'; break;
        case 9:  input_pwd[input_index] = '9'; break;
        case 10: input_pwd[input_index] = '0'; break;
        
        case 14:
            if(input_index > 0)
            {
                input_index--;
                input_pwd[input_index] = 0;
                Show_Pwd_Input();
                Buzzer1();
            }
            break;
            
        case 15:
            if(input_index == PWD_LENGTH)
            {
                uint8_t res = Verify_Password();
                Show_Verify_Result(res);
                if(res == 1)
                {
                    Delay_ms(OPEN_DOOR_TIME);
                    Close_Door_And_Reset();
                }
            }
            break;
            
        case 16:
            flag_pwd_input = 0;
            input_index = 0;
            memset(input_pwd, 0, sizeof(input_pwd));
            OLED_Clear();
            OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
            OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
            OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
            OLED_Update();
            break;
            
        default:
            break;
    }
    
    if((key_val >=1 && key_val <=9) || key_val ==10)
    {
        if(input_index < PWD_LENGTH)
        {
            input_index++;
            Show_Pwd_Input();
            Buzzer1();
        }
    }
}

void Add_FR(void)
{
	uint8_t i=0,ensure ,processnum=0;
	uint16_t ID;
    OLED_Clear();
    OLED_ShowString(X_CENTER(8), OLED_Y1, "Add Finger", OLED_8X16);
    OLED_Update();
  
	while(1)
	{
		switch (processnum)
		{
			case 0:
				i++;
				OLED_ShowString(X_CENTER(8), OLED_Y2, "Press Finger", OLED_8X16);
				OLED_Update();
				ensure=GZ_GetImage();
				if(ensure==0x00) 
				{
					ensure=GZ_GenChar(CharBuffer1);
					if(ensure==0x00)
					{
						i=0;
						processnum=1;					
					}			
				}						
				break;
			
			case 1:
				i++;
				OLED_ShowString(X_CENTER(8), OLED_Y2, "Press Again", OLED_8X16);
				OLED_Update();
				ensure=GZ_GetImage();
				if(ensure==0x00) 
				{
					ensure=GZ_GenChar(CharBuffer2);			
					if(ensure==0x00)
					{
						i=0;
						processnum=2;
					}
				}	
				break;

			case 2:
				OLED_ShowString(X_CENTER(8), OLED_Y2, "Matching...", OLED_8X16);
				OLED_Update();
				ensure=GZ_Match();
				if(ensure==0x00) 
				{
					processnum=3;
				}
				else 
				{
					OLED_ShowString(X_CENTER(8), OLED_Y2, "Match ERR", OLED_8X16);
					OLED_Update();
					Delay_ms(1000);
					i=0;
					processnum=0;		
				}
				Delay_ms(1000);
				break;

			case 3:
				OLED_ShowString(X_CENTER(8), OLED_Y2, "Generating...", OLED_8X16);
				OLED_Update();
				ensure=GZ_RegModel();
				if(ensure==0x00) 
				{
					processnum=4;
				}else {
				  OLED_ShowString(X_CENTER(8), OLED_Y2, "Gen ERR", OLED_8X16);
				  OLED_Update();
				  Delay_ms(1000);
				  processnum=0;
				}
				Delay_ms(1000);
				break;
				
			case 4:	
				ID = ValidN; 
				if(ID >= 300)
				{
					OLED_Clear();
					OLED_ShowString(X_CENTER(8), OLED_Y1, "Finger Full", OLED_8X16);
					OLED_Update();
					Buzzer_Alarm();
					Delay_ms(2000);
					break;
				}
				ensure=GZ_StoreChar(CharBuffer2,ID);
				if(ensure==0x00) 
				{			
					OLED_Clear();
					OLED_ShowString(X_CENTER(8), OLED_Y1, "Add OK!", OLED_8X16);
					OLED_ShowString(X_CENTER(8), OLED_Y2, "ID: ", OLED_8X16);
					OLED_ShowNum(X_CENTER(8)+24, OLED_Y2, ID, 3, OLED_8X16);
					OLED_Update();
					GZ_ValidTempleteNum(&ValidN);
					Buzzer1();
					Delay_ms(2000);
					return ;
				}else {
				  OLED_ShowString(X_CENTER(8), OLED_Y2, "Store ERR", OLED_8X16);
				  OLED_Update();
				  Buzzer_Alarm();
				  Delay_ms(1000);
				  processnum=0;
				}					
				break;				
		}
		Delay_ms(80);
		if(i==10)
		{
			OLED_Clear();
			OLED_ShowString(X_CENTER(8), OLED_Y1, "Time Out", OLED_8X16);
			OLED_Update();
			Buzzer_Alarm();
			Delay_ms(2000);
			break;	
		}				
	}
}

void press_FR(void)
{
    SearchResult seach;
    uint8_t ensure;
    OLED_Clear();
    OLED_ShowString(X_CENTER(8), OLED_Y1, "Press Finger", OLED_8X16);
    OLED_Update();
    
    Delay_ms(1000);
    ensure=GZ_GetImage();
    
    if(ensure==0x00)
    {	
        ensure=GZ_GenChar(CharBuffer1);
        if(ensure==0x00)
        {		
            ensure=GZ_HighSpeedSearch(CharBuffer1,0,300,&seach);
            if(ensure==0x00)
            {	
                if(seach.mathscore>100)
                {
                    OLED_Clear();
                    OLED_ShowString(X_CENTER(8), OLED_Y1, "Finger OK", OLED_8X16);
                    OLED_ShowString(X_CENTER(8), OLED_Y2, "Door Open", OLED_8X16);
                    OLED_Update();
                    Buzzer1();
                    Servo_SetAngle(SERVO_OPEN_ANGLE);
                    Delay_ms(OPEN_DOOR_TIME);
                    Servo_SetAngle(SERVO_CLOSE_ANGLE);
                }
                else
                {
                    OLED_Clear();
                    OLED_ShowString(X_CENTER(8), OLED_Y1, "Finger ERR", OLED_8X16);
                    OLED_Update();
                    Buzzer_Alarm();
                }
            }
            else
            {
                OLED_Clear();
                OLED_ShowString(X_CENTER(8), OLED_Y1, "Finger ERR", OLED_8X16);
                OLED_Update();
                Buzzer_Alarm();
            }
        }
        else
        {
            OLED_Clear();
            OLED_ShowString(X_CENTER(8), OLED_Y1, "Gen Char ERR", OLED_8X16);
            OLED_Update();
            Buzzer_Alarm();
        }
    }
    else
    {
        OLED_Clear();
        OLED_ShowString(X_CENTER(8), OLED_Y1, "No Finger", OLED_8X16);
        OLED_Update();
        Buzzer_Alarm();
    }
    Delay_ms(2000);
    flag_finger = 0;
    finger_verify_flag = 0;
    OLED_Clear();
    OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
    OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
    OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
    OLED_Update();
}

int main(void)
 {	
    Servo_Init();
    Buzzer_Init();
    Key_Init();
	Key_Init1(); 
    OLED_Init();
    Delay_ms(100);
    RFID_Init();
    AS608_Init();

    while(GZ_HandShake(&AS608Addr))
    {
        OLED_Clear();
        OLED_ShowString(X_CENTER(12), OLED_Y1, "Finger Init...", OLED_8X16);
        OLED_Update();
        Delay_ms(1000);
    }
    GZ_ValidTempleteNum(&ValidN);
    GZ_ReadSysPara(&AS608Para);

    Servo_SetAngle(0);
    Read_Card();

    OLED_Clear();
    OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
    OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
    OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
    OLED_Update();

    while (1)
    {
        uint8_t MatrixKeyNum = Key_GetNum1(); 
        if(MatrixKeyNum != 0)
        {
            if(MatrixKeyNum == 15 && flag_pwd_input == 0)
            {
                flag_scan = 0;
                flag_pwd_input = 1;
                input_index = 0;
                memset(input_pwd, 0, sizeof(input_pwd));
                Show_Welcome_UI();
            }
            else if(flag_pwd_input == 1)
            {
                Key_Process(MatrixKeyNum);
            }
        }

        KeyNum = 0;
        if(flag_pwd_input == 0 && flag_finger == 0)
        {
            KeyNum = Key_Scan(); 
        }

        if(flag_pwd_input == 0 && flag_finger == 0)
        {
            if(KeyNum==1)
            {
                flag_finger = 1;
            }
            if(KeyNum==2)
            {
                flag_scan=0;
                flag_addcard=1;
                flag_deletecard=0;
            }
            if(KeyNum==3)
            {
                flag_scan=0;
                flag_addcard=0;
                flag_deletecard=1;
            }
            if(KeyNum==4)
            {
                flag_scan=0;
                flag_addcard=0;
                flag_deletecard=0;
                flag_pwd_input=0;
                
                OLED_Clear();
                OLED_ShowString(X_CENTER(12), OLED_Y1, "Finger Mode", OLED_8X16);
                OLED_ShowString(X_CENTER(12), OLED_Y2, "1:Add 2:Verify", OLED_8X16);
                OLED_ShowString(X_CENTER(12), OLED_Y3, "3:Back", OLED_8X16);
                OLED_Update();
                
                uint8_t key_sel=0;
                while(key_sel==0)
                {
                    key_sel=Key_Scan();
                    Delay_ms(150);
                    if(key_sel==1){flag_finger=1;break;}
                    if(key_sel==2){flag_finger=2;break;}
                    if(key_sel==3)
                    {
                        flag_finger=0;
                        OLED_Clear();
                        OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
                        OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
                        OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
                        OLED_Update();
                        break;
                    }
                }
            }
        }

        if(flag_scan==1 && flag_pwd_input ==0 && flag_finger ==0)
        {
            RFID_Check();
        }

        if(flag_pwd_input ==0 && flag_finger ==0)
        {
            while(flag_addcard==1)	
            {
                KeyNum = 0;
                KeyNum = Key_Scan();
                if(KeyNum==1)		
                {
                    flag_scan=1;
                    flag_addcard=0;
                    flag_deletecard=0;
                }
                if(KeyNum==3)		
                {
                    flag_scan=0;
                    flag_addcard=0;
                    flag_deletecard=1;
                }
                 OLED_Clear();
                OLED_ShowString(X_CENTER(8), OLED_Y1, "Add Card", OLED_8X16);
                OLED_Update();

                if (PcdRequest(REQ_ALL, Temp) == MI_OK)
                {
                    if (PcdAnticoll(UID) == MI_OK)
                    {
                        if(UI0[0] == 0xFF && UI0[1] == 0xFF && UI0[2] == 0xFF && UI0[3] == 0xFF) tempcard = 0;	
                        else if(UI1[0] == 0xFF && UI1[1] == 0xFF && UI1[2] == 0xFF && UI1[3] == 0xFF) tempcard = 1;
                        else if(UI2[0] == 0xFF && UI2[1] == 0xFF && UI2[2] == 0xFF && UI2[3] == 0xFF) tempcard = 2;
                        else if(UI3[0] == 0xFF && UI3[1] == 0xFF && UI3[2] == 0xFF && UI3[3] == 0xFF) tempcard = 3;
                        else tempcard = 4;
                        
                        if(UID[0]==UI0[0] && UID[1]==UI0[1] && UID[2]==UI0[2] && UID[3]==UI0[3])	tempcard = 5;
                        if(UID[0]==UI1[0] && UID[1]==UI1[1] && UID[2]==UI1[2] && UID[3]==UI1[3])	tempcard = 5;
                        if(UID[0]==UI2[0] && UID[1]==UI2[1] && UID[2]==UI2[2] && UID[3]==UI2[3])	tempcard = 5;
                        if(UID[0]==UI3[0] && UID[1]==UI3[1] && UID[2]==UI3[2] && UID[3]==UI3[3])	tempcard = 5;
                        
                        switch(tempcard)
                        {
                            case 0:
                            {
                                UI0[0] = UID[0];	
                                UI0[1] = UID[1];
                                UI0[2] = UID[2];
                                UI0[3] = UID[3];
                                FLASH_W(FLASH_ADDR1,UI0[0],UI0[1],UI0[2],UI0[3]);	
                                OLED_ShowString(X_CENTER(10), OLED_Y1, "Card 1 OK", OLED_8X16);
                                OLED_Update();
                                Buzzer1();
                                WaitCardOff();	
                            }break;
                            case 1:
                            {
                                UI1[0] = UID[0];
                                UI1[1] = UID[1];
                                UI1[2] = UID[2];
                                UI1[3] = UID[3];
                                FLASH_W(FLASH_ADDR2,UI1[0],UI1[1],UI1[2],UI1[3]);
                                OLED_ShowString(X_CENTER(10), OLED_Y1, "Card 2 OK", OLED_8X16);
                                OLED_Update();
                                Buzzer1();
                                WaitCardOff();
                            }break;
                            case 2:
                            {
                                UI2[0] = UID[0];
                                UI2[1] = UID[1];
                                UI2[2] = UID[2];
                                UI2[3] = UID[3];
                                FLASH_W(FLASH_ADDR3,UI2[0],UI2[1],UI2[2],UI2[3]);
                                OLED_ShowString(X_CENTER(10), OLED_Y1, "Card 3 OK", OLED_8X16);
                                OLED_Update();
                                Buzzer1();
                                WaitCardOff();
                            }break;
                            case 3:
                            {
                                UI3[0] = UID[0];
                                UI3[1] = UID[1];
                                UI3[2] = UID[2];
                                UI3[3] = UID[3];
                                FLASH_W(FLASH_ADDR4,UI3[0],UI3[1],UI3[2],UI3[3]);
                                OLED_ShowString(X_CENTER(10), OLED_Y1, "Card 4 OK", OLED_8X16);
                                OLED_Update();
                                Buzzer1();
                                WaitCardOff();
                            }break;
                            case 4:
                            {	
                                OLED_ShowString(X_CENTER(10), OLED_Y1, "No Memory", OLED_8X16);
                                OLED_Update();
                                Buzzer_Alarm();
                                WaitCardOff();
                            }
                            case 5:
                            {	
                                OLED_ShowString(X_CENTER(12), OLED_Y1, "Card Exists", OLED_8X16);
                                OLED_Update();
                                Buzzer_Alarm();
                                WaitCardOff();
                            }
                            default:break;
                         }
                    }
                }
            }
        }

        if(flag_pwd_input ==0 && flag_finger ==0)
        {   
            OLED_Clear();
            while(flag_deletecard==1)	
            {     
                OLED_ShowString(X_CENTER(10), OLED_Y1, "Delete Card", OLED_8X16);
                OLED_Update();
                KeyNum = 0;
                KeyNum = Key_Scan();
                
                if(KeyNum==4)	
                {
                    flag_scan=1;
                    flag_addcard=0;
                    flag_deletecard=0;
                    OLED_Clear();
                    OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
                    OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
                    OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
                    OLED_Update();
                    break;
                }
                
                if(KeyNum==1)	
                {
                    select++;
                    if(select>=4 || select<1)select=0;
                }
                if(KeyNum==2)	
                {
                    select--;
                    if(select>=4 || select<1)select=0;
                }
                
                switch (select)
                {
                    case 0:
                    {
                        OLED_ShowString(X_CENTER(12), OLED_Y2, "Del Card 1?", OLED_8X16);
                        OLED_Update();
                        if(KeyNum==3)	
                        {
                            FLASH_Clear(FLASH_ADDR1);	
                            UI0[0]=0xFF;UI0[1]=0xFF;UI0[2]=0xFF;UI0[3]=0xFF;
                            OLED_ShowString(X_CENTER(12), OLED_Y2, "Card 1 OK", OLED_8X16);
                            OLED_Update();
                            Buzzer1();	
                            Delay_ms(1500);
                        }
                    }break;
                    case 1:
                    {
                        OLED_ShowString(X_CENTER(12), OLED_Y2, "Del Card 2?", OLED_8X16);
                        OLED_Update();
                        if(KeyNum==3)
                        {
                            FLASH_Clear(FLASH_ADDR2);
                            UI1[0]=0xFF;UI1[1]=0xFF;UI1[2]=0xFF;UI1[3]=0xFF;
                            OLED_ShowString(X_CENTER(12), OLED_Y2, "Card 2 OK", OLED_8X16);
                            OLED_Update();
                            Buzzer1();
                            Delay_ms(1500);
                        }
                    }break;
                    case 2:
                    {
                        OLED_ShowString(X_CENTER(12), OLED_Y2, "Del Card 3?", OLED_8X16);
                        OLED_Update();
                        if(KeyNum==3)
                        {
                            FLASH_Clear(FLASH_ADDR3);
                            UI2[0]=0xFF;UI2[1]=0xFF;UI2[2]=0xFF;UI2[3]=0xFF;
                            OLED_ShowString(X_CENTER(12), OLED_Y2, "Card 3 OK", OLED_8X16);
                            OLED_Update();
                            Buzzer1();
                            Delay_ms(1500);
                        }
                    }break;
                    case 3:
                    {
                        OLED_ShowString(X_CENTER(12), OLED_Y2, "Del Card 4?", OLED_8X16);
                        OLED_Update();
                        if(KeyNum==3)
                        {
                            FLASH_Clear(FLASH_ADDR4);
                            UI3[0]=0xFF;UI3[1]=0xFF;UI3[2]=0xFF;UI3[3]=0xFF;
                            OLED_ShowString(X_CENTER(12), OLED_Y2, "Card 4 OK", OLED_8X16);
                            OLED_Update();
                            Buzzer1();
                            Delay_ms(1500);
                        }
                    }break;
                    default:break;
                }
            }
        }

        if(flag_finger == 1)
        {
            Add_FR();
            flag_finger = 0;
            OLED_Clear();
            OLED_ShowString(X_CENTER(10), OLED_Y1, "Welcome Home", OLED_8X16);
            OLED_ShowString(X_CENTER(12), OLED_Y2, "KEY15=PWD Mode", OLED_8X16);
            OLED_ShowString(X_CENTER(8), OLED_Y3, "(PB6+PA5)", OLED_8X16);
            OLED_Update();
        }
        else if(flag_finger == 2)
        {
            press_FR();
        }
    }
 }

void RFID_Check()						
{
    cardnumber = Rc522Test();	
    if(cardnumber == 0)		
    {
        OLED_ShowString(X_CENTER(8), OLED_Y1, "Card ERR", OLED_8X16);
        OLED_Update();
        Buzzer_Alarm();		
        WaitCardOff();		
    }
    else if(cardnumber==1||cardnumber==2||cardnumber==3||cardnumber == 4)		
    {	
        OLED_ShowString(X_CENTER(8), OLED_Y1, "Card OK", OLED_8X16);
        OLED_ShowString(X_CENTER(6), OLED_Y2, "ID: ", OLED_8X16);
        OLED_ShowNum(X_CENTER(6)+24, OLED_Y2, cardnumber, 1, OLED_8X16);
        OLED_Update();
        Buzzer2();			
        Servo_SetAngle(90);	
        Delay_ms(1500);
        Servo_SetAngle(0);
        WaitCardOff();		
    }	
}

void Read_Card()
{
    UI0[0]=FLASH_R(FLASH_ADDR1);
    UI0[1]=FLASH_R(FLASH_ADDR1+2);
    UI0[2]=FLASH_R(FLASH_ADDR1+4);
    UI0[3]=FLASH_R(FLASH_ADDR1+6);
    
    UI1[0]=FLASH_R(FLASH_ADDR2);
    UI1[1]=FLASH_R(FLASH_ADDR2+2);
    UI1[2]=FLASH_R(FLASH_ADDR2+4);
    UI1[3]=FLASH_R(FLASH_ADDR2+6);
    
    UI2[0]=FLASH_R(FLASH_ADDR3);
    UI2[1]=FLASH_R(FLASH_ADDR3+2);
    UI2[2]=FLASH_R(FLASH_ADDR3+4);
    UI2[3]=FLASH_R(FLASH_ADDR3+6);
    
    UI3[0]=FLASH_R(FLASH_ADDR4);
    UI3[1]=FLASH_R(FLASH_ADDR4+2);
    UI3[2]=FLASH_R(FLASH_ADDR4+4);
    UI3[3]=FLASH_R(FLASH_ADDR4+6);
}
