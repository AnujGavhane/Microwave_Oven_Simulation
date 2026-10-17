#include <xc.h>
#include "clcd.h"
#include "matrix_keypad.h"
#include "main.h"
#include "timers.h"

#pragma config WDTE = OFF
int operation_mode, reset_flag;
int min, sec;
int ret, flag = 0, heat_flag = 0;
char sec_arr[3], min_arr[3];

static void init_config(void) {
    init_clcd(); // Initialize LCD
    init_matrix_keypad(); // Initialize keypad
    init_timer2();  // Initialize timer
    /*for the buzzer*/
    BUZZ_DDR = 0;  // Set buzzer as output
    BUZZER = OFF; // Turn off buzzer
    /*for the fan*/
    FAN_DDR = 0; // Set fan as output
    FAN = OFF; // Turn off fan
    
    PEIE = 1; // Enable peripheral interrupts
    GIE = 1;  // Enable global interrupts
}

void main(void) {
    unsigned char key;
    init_config(); // Initialize all modules
    power_on(); // Show startup screen
    clear_dis(); // Clear display
    operation_mode = NOT_PRESSED;
    
    
    while(1)
    {
        key = read_matrix_keypad(STATE); // Read keypad input
        if(operation_mode == MICRO || operation_mode == GRILL || operation_mode == CONVECTION)
        {
            ; // Stay in selected mode
        }
            
        else if(key == 1)
        {
            operation_mode = MICRO; // Select micro mode
            reset_flag = MICRO_RESET;
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            clear_dis();
            
            clcd_print(" Power = 900W   ", LINE2(0)); // Show power
           
           __delay_ms(3000);
           clear_dis();
        }
        else if(key == 2) {
            clear_dis();
            operation_mode = GRILL; // Select grill mode
            reset_flag = GRILL_MODE_RESET;
             
        }
        else if(key == 3) {
            clear_dis();
            operation_mode = CONVECTION; // Select convection mode
            reset_flag = CONVECTION_RESET;
        }
        else if(key == 4) {
            if(operation_mode == NOT_PRESSED)
            {
                min = 0;
                sec = 30; // Default 30 sec
                TMR2ON = ON; // Start timer
                FAN = ON; // Turn on fan
                operation_mode = TIME_DISPLAY;
            }
            //operation_mode = START;
            else if(operation_mode == TIME_DISPLAY)
            {
                sec = sec + 30; // Add 30 sec
                if(sec >= 60)
                {
                    min++;
                    sec = sec - 60;
                }
            }
            else if(operation_mode == PAUSE)
            {
               TMR2ON = ON; // Resume timer
                FAN = ON;
                operation_mode = START;
            }
            
        }
        else if(key == 5) {
            operation_mode = PAUSE; // Pause operation
        }
        else if(key == 6) {
            operation_mode = STOP; // Stop operation
            clear_dis();
        }
        
       mode_select(key); // Handle modes
    }  
}

void mode_select(unsigned char key) {
    switch(operation_mode)
    {
        case NOT_PRESSED:
            modes_display(); // Show menu
            break;
        case START:
        case TIME_DISPLAY: // Show time
            time_dis();
            break;
        case MICRO:
            set_time(key);
            break;
        case GRILL:
            set_time(key);  // Set cooking time
            break;
        case CONVECTION:
            if(heat_flag == 0) 
            {
                ret = set_temp(key); // Set temperature
                if(ret == FAILURE)
                {
                    flag = 1;
                    reset_flag = CONVECTION_RESET;
                }
                else if(ret == SUCCESS)
                {
                    TMR2ON = OFF;
                    flag = 1;
                    heat_flag = 1;
                    clear_dis();
                    reset_flag = RESET_TIME;
                }
                else
                {
                    flag = 0;
                }
            }
            else
            {
                flag = 0;
                set_time(key); // After temp, set time
            }
            break;
        case PAUSE:
            TMR2ON = OFF; // Stop timer
            FAN = OFF;
            break;
        case STOP:
            TMR2ON = OFF;
            FAN = OFF;
            operation_mode = NOT_PRESSED; // Reset system
            break;
    }
    if(flag == 0)
    {
        reset_flag = NONE_RESET;
    }
}

void power_on(void)
{
    for(int i = 0; i<16; i++)
    {
        clcd_putch(BLOCK, LINE1(i)); // Fill top line
    }
    clcd_print("  Powering ON   ", LINE2(0));
    clcd_print(" Microwave Oven ", LINE3(0));
    for(int i = 0; i<16; i++)
    {
        clcd_putch(BLOCK, LINE4(i)); // Fill bottom line
    }
    __delay_ms(1500);
    
}
void modes_display(void)
{
    clcd_print("1.Micro", LINE1(0));
    clcd_print("2.Grill", LINE2(0));
    clcd_print("3.Convection", LINE3(0));
    clcd_print("4.Start", LINE4(0));
    
}
void clear_dis(void) 
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);  // Clear LCD
}
void set_time(unsigned char key) {
    static int wait, blink, blink_pos, key_count;
    if(reset_flag >= 0x11)
    {
        
        key = ALL_RELEASED;
        sec = 0;
        min = 0;
        key_count = 0;
        blink = 0;
        blink_pos = 0;
        //wait = 0;
       
        clcd_print("SET TIME (MM:SS)", LINE1(0));
        clcd_print("TIME- 00:00", LINE2(0));
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
    }
    /*if(wait++ == 20) {      /non blocking delay/
        wait = 0;
        blink = !blink;
    }*/
    
    if(key != ALL_RELEASED && key != '*' && key != '#')
    {
        
        key_count++;
        if(key_count <= 2)
        {
            sec = sec * 10 + key;
            blink_pos = 0;
        }
        else if(key_count > 2 && key_count < 5)
        {
            min = min *10 + key;
            blink_pos = 1;
        }
        if(key_count < 2) {
            blink_pos = 0;
        }
        else if(key_count >= 2 && key_count < 5) {
            blink_pos = 1;
        }
    }
    
    else if(key == '*')
    {
        if(key_count <= 2)
        {
            sec = 0;
            key_count = 0;
        }
        else if(key_count > 2 && key_count < 5)
        {
            min = 0;
            key_count = 2;
        }
    }
    else if(key == '#')
    {
        clear_dis();
        TMR2ON = 1;
        FAN = ON;
        operation_mode = TIME_DISPLAY;
    }
    
    
    sec_arr[0] = sec / 10 + '0'; //19---> 1
    sec_arr[1] = sec % 10 + '0';//19---> 9
    sec_arr[2] = '\0';
    
    min_arr[0] = min / 10 + '0'; //19---> 1
    min_arr[1] = min % 10 + '0';//19---> 9
    min_arr[2] = '\0';
            
    
    
    if(wait++ == 25)
    {
        wait = 0;
        blink = !blink;
        clcd_print(min_arr, LINE2(6));
        clcd_print(sec_arr, LINE2(9));
        
    }
    
    if(blink)
    {
        switch(blink_pos)
        {
            case 0:
                clcd_print("  ", LINE2(9));
                break;
            case 1:
                clcd_print("  ", LINE2(6));
                break;
        }
    }
    
        
}
void door_status_check(void) {
    if(DOOR == OPEN) 
    {
        BUZZER = ON;
        FAN = OFF;
        TMR2ON = 0;
        clear_dis();
        clcd_print("Door Status:OPEN", LINE2(0));
        clcd_print("Please Close", LINE3(0));
        while(DOOR == OPEN)
        {
            ;
        }
        clear_dis();
        
    }
        TMR2ON = 1;
        BUZZER = OFF;
        FAN = ON;
        
}

void time_dis(void) 
{
    door_status_check();
    clcd_print(" TIME =  ", LINE1(0));
  
    min_arr[0] = min / 10 + '0'; //19---> 1
    min_arr[1] = min % 10 + '0';//19---> 9
    min_arr[2] = '\0';
            
    clcd_print(min_arr, LINE1(9));
    
    clcd_putch(':', LINE1(11));
        
            
    sec_arr[0] = sec / 10 + '0'; //19---> 1
    sec_arr[1] = sec % 10 + '0';//19---> 9
    sec_arr[2] = '\0';
    /*
     clcd_putch(sec / 10 + '0', LINE1(12));
      clcd_putch(sec % 10 + '0', LINE1(13));  //taking each number and converting them to char and printing at a time
     */
    
    clcd_print(sec_arr, LINE1(12));
    
    clcd_print(" 4.Start/Resume ", LINE2(0));
    clcd_print(" 5.Pause        ", LINE3(0));
    clcd_print(" 6.Stop         ", LINE4(0));
    
    if((min == 0) && (sec == 0))
    {
        TMR2ON = 0;
        FAN = OFF;
        clear_dis();
        clcd_print("Cooking Time UP", LINE2(0));
        //Turn on buzzer
        BUZZER = ON;
        __delay_ms(2000);
        //Turn off buzzer
        BUZZER = OFF;
        
        clear_dis();
        operation_mode = NOT_PRESSED;
    }
    
    
}

char set_temp(unsigned char key) {
    static int wait, blink, key_count, temp;
    if(reset_flag == CONVECTION_RESET)
    {
        
        key = ALL_RELEASED;
        key_count = 0;
        blink = 0;
        temp = 0;
        wait = 0;
        clcd_print("SET TEMP. (  C )", LINE1(0));
        clcd_putch(DEGREE, LINE1(12));
        clcd_print("TEMP :  000", LINE2(0));
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
    }
    if(key != ALL_RELEASED && key != '*' && key != '#')
    {
        
        key_count++;
        if(key_count <= 3)
        {
            temp = temp * 10 + key;
        }
        
    }
    else if(key == '*')
    {
        key_count = 0;
        temp = 0;
    }
    else if(key == '#')
    {
        clear_dis();
        if(temp > 250) 
        {
            clcd_print(" Invalid Temp.", LINE2(0));
            BUZZER = ON;
            __delay_ms(2000);
            BUZZER = OFF;
            clear_dis();
            return FAILURE;
        }
        else
        {
            clcd_print("  Pre-Heating", LINE1(0));
            sec = 180; // 3-min
            TMR2ON = ON;
            FAN = ON;
            
            clcd_print("Time Rem.= ",LINE3(0));
            while(sec)
            {
                clcd_putch((sec / 100 + '0'), LINE3(11));
                clcd_putch(((sec/10) % 10 + '0'), LINE3(12));
                clcd_putch((sec%10 + '0'), LINE3(13));   
            }
            return SUCCESS;
        }
    }
    if(wait++ == 25)
    {
        wait = 0;
        blink = !blink;
        clcd_putch((temp / 100 + '0'), LINE2(8));
        clcd_putch(((temp/10) % 10 + '0'), LINE2(9));
        clcd_putch((temp%10 + '0'), LINE2(10));    
    }
    if(blink)
    {
        clcd_print("   ", LINE2(8));
    }
    return 0x11;
}
