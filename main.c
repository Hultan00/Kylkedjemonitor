#include "gd32vf103.h"
#include "gd32vf103.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lcd16.h"
#include "systick.h"
#include "string.h"
#include "gd32v_tf_card_if.h"
#include <time.h>
#include "Real_Time_Clock.h"


uint16_t read_temp();
void init_temp_sense();
float temp_convert(uint16_t rtd);

#define DEGREE 223


/* Since the temperature sensor works with a lot slower SPI than the LCD this is used to temporarily slow the bus down */
#define FCLK_SLOW() { SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & ~0x38) | 0x28; }	/* Set SCLK = PCLK2 / 64 */
#define FCLK_FAST() { SPI_CTL0(SPI1) = (SPI_CTL0(SPI1) & ~0x38) | 0x00; }	/* Set SCLK = PCLK2 / 2 */

void showTemp(float tempature, float max, float min){
    LCD_LINE(1);

    LCD_FLOAT_TO_STRING(tempature);
    LCD_DEC_TO_BIN(DEGREE);
    LCD_STRING_TO_BIN("C  ");

    LCD_LINE(2);

    LCD_FLOAT_TO_STRING(min);
    LCD_DEC_TO_BIN(DEGREE);
    LCD_CHAR_TO_BIN('-');

    LCD_FLOAT_TO_STRING(max);
    LCD_DEC_TO_BIN(DEGREE);
    LCD_STRING_TO_BIN("C  ");
}

void tone(int hz, int ms){
        int ms1 = ((float)ms/1000.0)*(float)hz;
        float calc = (float)hz/1000.0;
        hz = (1000.0/calc);


        for(int x = 0;x <= ms1;x++){
            gpio_bit_set(GPIOA, GPIO_PIN_8);

            delay_1us(hz/2);

            gpio_bit_reset(GPIOA, GPIO_PIN_8);

            delay_1us(hz/2);
        }
}

int checkTemp(float temp, float maxTemp2, float minTemp2, float warInt2){
    if((temp >= maxTemp2) || (temp <= minTemp2)){
        gpio_bit_set(GPIOB, GPIO_PIN_10);
        gpio_bit_reset(GPIOB, GPIO_PIN_11);
        gpio_bit_reset(GPIOB, GPIO_PIN_12);
        gpio_bit_set(GPIOC, GPIO_PIN_15);
        tone(2500*2, 100);
        return 1;

    }else if((temp >= (maxTemp2 - warInt2)) || (temp <= (minTemp2 + warInt2))){
        gpio_bit_reset(GPIOB, GPIO_PIN_10);
        gpio_bit_set(GPIOB, GPIO_PIN_11);
        gpio_bit_reset(GPIOB, GPIO_PIN_12);

        return 0;
    }else{
        gpio_bit_reset(GPIOB, GPIO_PIN_10);
        gpio_bit_reset(GPIOB, GPIO_PIN_11);
        gpio_bit_set(GPIOB, GPIO_PIN_12);
        return 0;
    }
}

int bUIn(){
    int input = 0;
    uint32_t bU;
    bU = gpio_input_bit_get(GPIOB, GPIO_PIN_8);
    if(bU == 0){
        input = 1;
    }else{
        input = 0;
    }
    return input;
}

int bDIn(){
    int input = 0;
    uint32_t bD;
    bD = gpio_input_bit_get(GPIOB, GPIO_PIN_5);
    if(bD == 0){
        input = 1;
    }else{
        input = 0;
    }
    return input;
}

int bMIn(){
    int input = 0;
    uint32_t bM;
    bM = gpio_input_bit_get(GPIOB, GPIO_PIN_9);
    if(bM == 0){
        input = 1;
    }else{
        input = 0;
    }
    return input;
}

void storeTemp(float temp2, float maxTemp3, float minTemp3){

                FATFS fs;

                volatile FRESULT fr; 

                FIL fil;

                UINT bw = 999;
                UINT br = 0;
    
                delay_1ms(100);

                set_fattime(get_time_year(), get_time_month(), get_time_day(), get_time_hour(), get_time_min(), get_time_sec());
                delay_1ms(100);

                fr = f_mount(&fs, "", 1);       //Mount storage device

                delay_1ms(50);

                f_sync(&fil);

                char buf[255] = "[";

                char stYear[10];
                char stMonth[10];
                char stDay[10];

                char stHour[10];
                char stMin[10];
                char stSec[10];

                itoa(get_time_year(), stYear, 10);

                itoa(get_time_month(), stMonth, 10);

                itoa(get_time_day(), stDay, 10);

                itoa(get_time_hour(), stHour, 10);

                itoa(get_time_min(), stMin, 10);

                itoa(get_time_sec(), stSec, 10);

                strcat(buf, &stYear);

                strcat(buf, "-");

                if(get_time_month() < 10){
                    strcat(buf, "0");
                }

                strcat(buf, &stMonth);

                strcat(buf, "-");

                if(get_time_day() < 10){
                    strcat(buf, "0");
                }

                strcat(buf, &stDay);

                strcat(buf, " ");

                if(get_time_hour() < 10){
                    strcat(buf, "0");
                }

                strcat(buf, &stHour);

                strcat(buf, ":");

                if(get_time_min() < 10){
                    strcat(buf, "0");
                }

                strcat(buf, &stMin);

                strcat(buf, ":");

                if(get_time_sec() < 10){
                    strcat(buf, "0");
                }

                strcat(buf, &stSec);

                strcat(buf, "] Temp:");

                char tempbuf[10];
                
                gcvt(temp2, 3, tempbuf);

                strcat(buf, &tempbuf);

                if((temp2 >= maxTemp3) || (temp2 <= minTemp3)){

                    strcat(buf, " (TEMPATURE OUT OF RANGE!)\n");

                    fr = f_open(&fil, "templog.txt", FA_OPEN_APPEND | FA_WRITE);

                    delay_1ms(20);

                    fr = f_write(&fil, buf, strlen(buf), &bw);

                    delay_1ms(50);

                }else if((temp2 >= (maxTemp3 - 1)) || (temp2 <= (minTemp3 + 1))){

                    strcat(buf, " (Warning!)\n");

                    fr = f_open(&fil, "templog.txt", FA_OPEN_APPEND | FA_WRITE);

                    delay_1ms(20);

                    fr = f_write(&fil, buf, strlen(buf), &bw);

                    delay_1ms(50);

                }else{

                    strcat(buf, " (Good)\n");
                    fr = f_open(&fil, "templog.txt", FA_OPEN_APPEND | FA_WRITE);

                    delay_1ms(20);

                    fr = f_write(&fil, buf, strlen(buf), &bw);

                    delay_1ms(50);
                }

                f_sync(&fil);

                delay_1ms(200);

                f_close(&fil);
    
                delay_1ms(100);
}

int DecBaseHex(int dec1){

    int first = dec1;
    int second = dec1;

    if(dec1 < 10){
        
    }else{
        first = first / 10;
        second = (second - (10 * first));
    }
    dec1 = (first<<4) & 240;
    dec1 = dec1 | second;
    
    return dec1;
}

void ShowTimeAndDate(int s){

    for(int x = 0;x < (s * 10);x++){

        delay_1ms(100);

        LCD_LINE(1);

        LCD_DEC_TO_STRING(get_time_year());

        LCD_STRING_TO_BIN("/");

        if(get_time_month() < 10){
            LCD_STRING_TO_BIN("0");
        }

        LCD_DEC_TO_STRING(get_time_month());

        LCD_STRING_TO_BIN("/");

        LCD_DEC_TO_STRING(get_time_day());

        LCD_LINE(2);

        LCD_DEC_TO_STRING(get_time_hour());

        LCD_STRING_TO_BIN(":");

        if(get_time_min() < 10){
            LCD_STRING_TO_BIN("0");
        }

        LCD_DEC_TO_STRING(get_time_min());

        LCD_STRING_TO_BIN(":");

        if(get_time_sec() < 10){
            LCD_STRING_TO_BIN("0");
        }

        LCD_DEC_TO_STRING(get_time_sec());

        LCD_STRING_TO_BIN(" ");
    }
}

int main(){

    rcu_periph_clock_enable(RCU_GPIOB);                                   
	rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);

    //LCD
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2); //Register Select
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_1); //Enable

	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0); //D0
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_1); //D1
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2); //D2
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_3); //D3
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_4); //D4
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5); //D5
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6); //D6
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7); //D7

    //LED LIGHTS
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10); //Red Light
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11); //Yellow light
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); //Green light

    //Buttons
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_9); //UP Button
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_8); //Down Button
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_5); //Shift mode Button

    //Speaker
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8); //Speaker

    //Real Time Clock
    rcu_periph_clock_enable(RCU_I2C0);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    //Brightness control pin
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    i2c_clock_config(I2C0, 100000, I2C_DTCY_2);

    i2c_enable(I2C0);

	LCD_INIT();

    init_temp_sense();

    delay_1ms(10);

    int32_t temp_read = 0;
    float temp_c = 0.0;

    int mode = 0;

    float maxTempSet = 30.0;
    float minTempSet = 25.0;

    float maxTemp = -273.0;
    float minTemp = 600.0;

    float warnInt = 0.0;

    int release = 0;

    short prOnce = 0;
    int hold = 0;

    int count = 0;
    int count1 = 0;
    int setClock = 0;

    int tempyear = 0;
    int tempmonth = 0;
    int tempday = 0;
    int temphour = 0;
    int tempmin = 0;
    int tempsec = 0;

    int powerSave = 0;

    short powerSaveStop = 0;

    short logInterval = 0;

    tone(1800, 200);
    tone(930, 200);

    gpio_bit_set(GPIOC, GPIO_PIN_15);

    ShowTimeAndDate(5);

    LCD_CLEAR();

    while(1){

        if((powerSave < 60) && (mode == 3) && (setClock == 0)){
            delay_1ms(1);
            powerSave++;
        }if(powerSave >= 60){
            if(powerSaveStop == 0){
                gpio_bit_reset(GPIOC, GPIO_PIN_15);
            }else{
                powerSave = 0;
            }
        }

        if((bMIn() == 1) && (release == 0)){
            if(powerSave < 60){
                mode++;
            }
            if(setClock == 1){
                if(mode >= 6){
                    set_time_sec(DecBaseHex(tempsec));
                    LCD_CLEAR();
                    LCD_STRING_TO_BIN("Clock Set!");
                    delay_1ms(1500);
                    setClock = 0;
                    mode = 0;
                }
            }else{
                if(mode >= 4){
                    mode = 0;
                }
            }
            release = 1;
            prOnce = 0;
            LCD_CLEAR();
        }

        if(release == 1){
            if(count1 <= 1000){
                count1++;
                delay_1ms(1);
            }if(count1 > 1000){
                setClock = 1;
                mode = 0;
                prOnce = 0;
                release = 2;
                LCD_CLEAR();
            }
        }

        if(bMIn() == 0){
            release = 0;
            count1 = 0;
        }

        if((bMIn() == 1) || (bUIn() == 1) || (bDIn() == 1)){
            gpio_bit_set(GPIOC, GPIO_PIN_15);
            powerSave = 0;
        }
        
        if(setClock == 1){
            switch(mode){
                //------------ Set Year -------------
                case 0:
                    if(prOnce == 0){
                        LCD_LINE(1);
                        LCD_STRING_TO_BIN("Input Year");
                        LCD_LINE(2);
                        LCD_DEC_TO_STRING(get_time_year());
                        LCD_STRING_TO_BIN(" ");
                        tempyear = get_time_year();
                        prOnce = 1;
                    }

                    if(bUIn() == 1){
                    if(hold == 0){
                        tempyear = tempyear + 1;
                    
                        
                        if(tempyear > 2099){
                            tempyear = 2099;
                        }

                        LCD_LINE(2);

                        LCD_DEC_TO_STRING(tempyear);
                        LCD_STRING_TO_BIN("  ");
                        hold = 1;
                    }else{
                        if(count < 1000){
                            count++;
                            delay_1ms(1);
                        }
                        else{
                            tempyear = tempyear + 1;
                            if(tempyear > 2099){
                                tempyear = 2099;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempyear);
                            LCD_STRING_TO_BIN("  ");
                        }
                    }
                    }

                if(bDIn() == 1){
                    if(hold == 0){
                        tempyear = tempyear - 1;
                        if(tempyear < 2000){
                            tempyear = 2000;
                        }
                        LCD_LINE(2);

                        LCD_DEC_TO_STRING(tempyear);
                        LCD_STRING_TO_BIN("  ");
                        hold = 1;
                    }else{
                        if(count <= 1000){
                            count++;
                            delay_1ms(1);
                        }
                        else{
                            tempyear = tempyear - 1;
                            if(tempyear < 2000){
                                tempyear = 2000;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempyear);
                            LCD_STRING_TO_BIN("  ");
                        }
                    }
                }

                if((bUIn() == 0) && (bDIn() == 0)){
                    count = 0;
                    hold = 0;
                }
                break;

                //------------- Set Month -------------
                case 1:
                    if(prOnce == 0){
                        LCD_LINE(1);
                        LCD_STRING_TO_BIN("Input Month");
                        LCD_LINE(2);
                        LCD_DEC_TO_STRING(get_time_month());
                        LCD_STRING_TO_BIN(" ");
                        tempmonth = get_time_month();
                        set_time_year(DecBaseHex(tempyear-2000));
                        prOnce = 1;
                    }
                    
                    if(bUIn() == 1){
                        if(hold == 0){
                            tempmonth = tempmonth + 1;


                            if(tempmonth > 12){
                                tempmonth = 12;
                            }

                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempmonth);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count < 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempmonth = tempmonth + 1;
                                if(tempmonth > 12){
                                    tempmonth = 12;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempmonth);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if(bDIn() == 1){
                        if(hold == 0){
                            tempmonth = tempmonth - 1;
                            if(tempmonth < 1){
                                tempmonth = 1;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempmonth);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count <= 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempmonth = tempmonth - 1;
                                if(tempmonth < 0){
                                    tempmonth = 0;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempmonth);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if((bUIn() == 0) && (bDIn() == 0)){
                        count = 0;
                        hold = 0;
                    }
                    
                    break;

                // ----------- Set Day ------------
                case 2:
                    if(prOnce == 0){
                        LCD_LINE(1);
                        LCD_STRING_TO_BIN("Input Day");
                        LCD_LINE(2);
                        LCD_DEC_TO_STRING(get_time_day());
                        LCD_STRING_TO_BIN(" ");
                        tempday = get_time_day();
                        if(tempmonth < 10){
                            set_time_month(DecBaseHex(tempmonth)&15);
                        }else{
                            set_time_month(DecBaseHex(tempmonth));
                        }
                        prOnce = 1;
                    }

                    if(bUIn() == 1){
                        if(hold == 0){
                            tempday = tempday + 1;


                            if(tempday > 31){
                                tempday = 31;
                            }

                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempday);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count < 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempday = tempday + 1;
                                if(tempday > 31){
                                    tempday = 31;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempday);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if(bDIn() == 1){
                        if(hold == 0){
                            tempday = tempday - 1;
                            if(tempday < 1){
                                tempday = 1;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempday);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count <= 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempday = tempday - 1;
                                if(tempday < 1){
                                    tempday = 1;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempday);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if((bUIn() == 0) && (bDIn() == 0)){
                        count = 0;
                        hold = 0;
                    }

                    break;

                    // ------------- Set Hour -------------
                case 3:
                    if(prOnce == 0){
                        LCD_LINE(1);
                        LCD_STRING_TO_BIN("Input Hour");
                        LCD_LINE(2);
                        LCD_DEC_TO_STRING(get_time_hour());
                        LCD_STRING_TO_BIN(" ");
                        temphour = get_time_hour();
                        set_time_day(DecBaseHex(tempday));
                        prOnce = 1;
                    }

                    if(bUIn() == 1){
                        if(hold == 0){
                            temphour = temphour + 1;


                            if(temphour > 23){
                                temphour = 23;
                            }

                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(temphour);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count < 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                temphour = temphour + 1;
                                if(temphour > 23){
                                    temphour = 23;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(temphour);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if(bDIn() == 1){
                        if(hold == 0){
                            temphour = temphour - 1;
                            if(temphour < 0){
                                temphour = 0;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(temphour);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count <= 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                temphour = temphour - 1;
                                if(temphour < 0){
                                    temphour = 0;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(temphour);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if((bUIn() == 0) && (bDIn() == 0)){
                        count = 0;
                        hold = 0;
                    }

                    break;

                    // -------------- Set Minutes -------------
                case 4:
                    if(prOnce == 0){
                        LCD_LINE(1);
                        LCD_STRING_TO_BIN("Input Minutes");
                        LCD_LINE(2);
                        LCD_DEC_TO_STRING(get_time_min());
                        LCD_STRING_TO_BIN(" ");
                        tempmin = get_time_min();
                        set_time_hour(DecBaseHex(temphour));
                        prOnce = 1;
                    }

                    if(bUIn() == 1){
                        if(hold == 0){
                            tempmin = tempmin + 1;


                            if(tempmin > 59){
                                tempmin = 59;
                            }

                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempmin);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count < 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempmin = tempmin + 1;
                                if(tempmin > 59){
                                    tempmin = 59;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempmin);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if(bDIn() == 1){
                        if(hold == 0){
                            tempmin = tempmin - 1;
                            if(tempmin < 0){
                                tempmin = 0;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempmin);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count <= 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempmin = tempmin - 1;
                                if(tempmin < 0){
                                    tempmin = 0;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempmin);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if((bUIn() == 0) && (bDIn() == 0)){
                        count = 0;
                        hold = 0;
                    }

                    break;

                    // -------------- Set Seconds --------------
                case 5:
                    if(prOnce == 0){
                        LCD_LINE(1);
                        LCD_STRING_TO_BIN("Input Seconds");
                        LCD_LINE(2);
                        LCD_DEC_TO_STRING(get_time_sec());
                        LCD_STRING_TO_BIN(" ");
                        tempsec = get_time_sec();
                        if((tempsec > 59) || (1 < tempsec)){
                            tempsec = 1;
                        }
                        set_time_min(DecBaseHex(tempmin));
                        prOnce = 1;
                    }


                    if(bUIn() == 1){
                        if(hold == 0){
                            tempsec = tempsec + 1;


                            if(tempsec > 59){
                                tempsec = 59;
                            }

                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempsec);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count < 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempsec = tempsec + 1;
                                if(tempsec > 59){
                                    tempsec = 59;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempsec);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if(bDIn() == 1){
                        if(hold == 0){
                            tempsec = tempsec - 1;
                            if(tempsec < 0){
                                tempsec = 0;
                            }
                            LCD_LINE(2);

                            LCD_DEC_TO_STRING(tempsec);
                            LCD_STRING_TO_BIN("  ");
                            hold = 1;
                        }else{
                            if(count <= 1000){
                                count++;
                                delay_1ms(1);
                            }
                            else{
                                tempsec = tempsec - 1;
                                if(tempsec < 0){
                                    tempsec = 0;
                                }
                                LCD_LINE(2);

                                LCD_DEC_TO_STRING(tempsec);
                                LCD_STRING_TO_BIN("  ");
                            }
                        }
                    }

                    if((bUIn() == 0) && (bDIn() == 0)){
                        count = 0;
                        hold = 0;
                    }

                    break;
            }
        }else{

        switch(mode){
            case 2: //------------- Set warning interval -----------------

                if(prOnce == 0){
                    LCD_LINE(1);
                    LCD_STRING_TO_BIN("Input Warning");
                    LCD_LINE(2);
                    LCD_FLOAT_TO_STRING(warnInt);
                    LCD_STRING_TO_BIN("  ");
                    prOnce = 1;
                }

                

                if(bUIn() == 1){
                    if(hold == 0){
                        warnInt = warnInt + 0.1;
                    
                        
                        if(warnInt > 600.0){
                            warnInt = 600.0;
                        }

                        LCD_LINE(2);

                        LCD_FLOAT_TO_STRING(warnInt);
                        LCD_STRING_TO_BIN("  ");
                        hold = 1;
                    }else{
                        if(count < 1000){
                            count++;
                            delay_1ms(1);
                        }
                        else{
                            warnInt = warnInt + 0.1;
                            if(warnInt > 600.0){
                                warnInt = 600.0;
                            }
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(warnInt);
                            LCD_STRING_TO_BIN("  ");
                        }
                    }
                }

                if(bDIn() == 1){
                    if(hold == 0){
                        warnInt = warnInt - 0.1;
                        if(warnInt < 0){
                            warnInt = 0;
                        }
                        LCD_LINE(2);

                        LCD_FLOAT_TO_STRING(warnInt);
                        LCD_STRING_TO_BIN("  ");
                        hold = 1;
                    }else{
                        if(count <= 1000){
                            count++;
                            delay_1ms(1);
                        }
                        else{
                            warnInt = warnInt - 0.1;
                            if(warnInt < 0){
                                warnInt = 0;
                            }
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(warnInt);
                            LCD_STRING_TO_BIN("  ");
                        }
                    }
                }

                if((bUIn() == 0) && (bDIn() == 0)){
                    count = 0;
                    hold = 0;
                }

                break;
            case 3: //--------------- Show temp + max and min -------------------

                gpio_bit_reset(GPIOB, GPIO_PIN_0);

                if(bUIn() == 1){
                    maxTemp = -273.0;
                }if(bDIn() == 1){
                    minTemp = 600.0;
                }

                temp_read = read_temp();
                temp_c = temp_convert(temp_read);
                if(temp_c > maxTemp){
                    maxTemp = temp_c;
                }
                if(temp_c < minTemp){
                    minTemp = temp_c;
                }

                gpio_bit_set(GPIOB, GPIO_PIN_0);

                delay_1ms(1);

                showTemp(temp_c, maxTemp, minTemp);

                powerSaveStop = checkTemp(temp_c, maxTempSet, minTempSet, warnInt);

                logInterval++;
                if(logInterval>120){
                    gpio_bit_reset(GPIOC, GPIO_PIN_14);
                    delay_1ms(1);
                    storeTemp(temp_c, maxTempSet, minTempSet);
                    gpio_bit_set(GPIOC, GPIO_PIN_14);
                    logInterval = 0;
                }else{
                    delay_1ms(500);
                }

                break;
            case 0: //-------------- Set Max ----------------

                if(prOnce == 0){
                    LCD_LINE(1);
                    LCD_STRING_TO_BIN("Input Max");
                    LCD_LINE(2);
                    LCD_FLOAT_TO_STRING(maxTempSet);
                    LCD_DEC_TO_BIN(DEGREE);
                    LCD_STRING_TO_BIN("C  ");
                    prOnce = 1;
                }

                if(bUIn() == 1){
                    if(hold == 0){
                        maxTempSet = maxTempSet + 0.1;
                    
                        if(maxTempSet > 600.0){
                            maxTempSet = 600.0;
                        }

                        LCD_LINE(2);

                        LCD_FLOAT_TO_STRING(maxTempSet);
                        LCD_DEC_TO_BIN(DEGREE);
                        LCD_STRING_TO_BIN("C  ");
                        hold = 1;
                    }else{
                        if(count <= 1000){
                            count++;
                            delay_1ms(1);
                        }else{
                            maxTempSet = maxTempSet + 0.1;
                    
                            if(maxTempSet > 600.0){
                                maxTempSet = 600.0;
                            }

                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(maxTempSet);
                            LCD_DEC_TO_BIN(DEGREE);
                            LCD_STRING_TO_BIN("C  ");
                        }
                    }
                }

                if(bDIn() == 1){
                    if(hold == 0){
                        if((maxTempSet - 0.1) < minTempSet){

                        }else{
                            maxTempSet = maxTempSet - 0.1;
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(maxTempSet);
                            LCD_DEC_TO_BIN(DEGREE);
                            LCD_STRING_TO_BIN("C  ");
                        }

                        if(maxTempSet < -273.0){
                            maxTempSet = -273.0;
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(maxTempSet);
                            LCD_DEC_TO_BIN(DEGREE);
                            LCD_STRING_TO_BIN("C  ");
                        }
                        hold = 1;
                    }else{
                        if(count <= 1000){
                            count++;
                            delay_1ms(1);
                        }else{
                            if((maxTempSet - 0.1) < minTempSet){

                            }else{
                                maxTempSet = maxTempSet - 0.1;
                                LCD_LINE(2);

                                LCD_FLOAT_TO_STRING(maxTempSet);
                                LCD_DEC_TO_BIN(DEGREE);
                                LCD_STRING_TO_BIN("C  ");
                            }

                            if(maxTempSet < -273.0){
                                maxTempSet = -273.0;
                                LCD_LINE(2);

                                LCD_FLOAT_TO_STRING(maxTempSet);
                                LCD_DEC_TO_BIN(DEGREE);
                                LCD_STRING_TO_BIN("C  ");
                            }
                        }
                    }
                }

                if((bUIn() == 0) && (bDIn() == 0)){
                    count = 0;
                    hold = 0;
                }
                
                break;
            case 1: //----------- Set min --------------

                if(prOnce == 0){
                    LCD_LINE(1);
                    LCD_STRING_TO_BIN("Input Min");
                    LCD_LINE(2);
                    LCD_FLOAT_TO_STRING(minTempSet);
                    LCD_DEC_TO_BIN(DEGREE);
                    LCD_STRING_TO_BIN("C  ");
                    prOnce = 1;
                }

                if(bDIn() == 1){
                    if(hold == 0){
                        minTempSet = minTempSet - 0.1;  //If user presses on the down button, decrease the mintemp in increments of 0.1
                        if(minTempSet < -273.0){         //User can't go below -273
                            minTempSet = -273.0;
                        }
                        LCD_LINE(2);

                        LCD_FLOAT_TO_STRING(minTempSet);
                        LCD_DEC_TO_BIN(DEGREE);
                        LCD_STRING_TO_BIN("C  ");
                        hold = 1;
                    }else{
                        if(count <= 1000){
                            count++;
                            delay_1ms(1);
                        }else{
                            minTempSet = minTempSet - 0.1;  //If user presses on the down button, decrease the mintemp in increments of 0.1
                            if(minTempSet < -273.0){         //User can't go below -273
                                minTempSet = -273.0;
                            }
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(minTempSet);
                            LCD_DEC_TO_BIN(DEGREE);
                            LCD_STRING_TO_BIN("C  ");
                        }
                    }
                }

                if(bUIn() == 1){
                    if(hold == 0){
                        if((minTempSet + 0.1) > maxTempSet){

                        }else{
                            minTempSet = minTempSet + 0.1;
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(minTempSet);
                            LCD_DEC_TO_BIN(DEGREE);
                            LCD_STRING_TO_BIN("C  ");
                        }

                        if(minTempSet > 600.0){
                            minTempSet = 600.0;
                            LCD_LINE(2);

                            LCD_FLOAT_TO_STRING(minTempSet);
                            LCD_DEC_TO_BIN(DEGREE);
                            LCD_STRING_TO_BIN("C  ");
                        }
                        hold = 1;
                    }else{
                        if(count <= 1000){
                            count++;
                            delay_1ms(1);
                        }else{
                            if((minTempSet + 0.1) > maxTempSet){

                            }else{
                                minTempSet = minTempSet + 0.1;
                                LCD_LINE(2);

                                LCD_FLOAT_TO_STRING(minTempSet);
                                LCD_DEC_TO_BIN(DEGREE);
                                LCD_STRING_TO_BIN("C  ");
                            }

                            if(minTempSet > 600.0){
                                minTempSet = 600.0;
                                LCD_LINE(2);

                                LCD_FLOAT_TO_STRING(minTempSet);
                                LCD_DEC_TO_BIN(DEGREE);
                                LCD_STRING_TO_BIN("C  ");
                            }
                        }
                    }
                }

                if((bUIn() == 0) && (bDIn() == 0)){
                    count = 0;
                    hold = 0;
                }

                break;
        }
    
    }

    }

}





uint8_t read_write_spi ( uint8_t data){
    //Block if already sending
	while(spi_i2s_flag_get(SPI1, SPI_FLAG_TBE) != SET);
    
    spi_i2s_data_transmit(SPI1, data);

    //Block until receive
	while(spi_i2s_flag_get(SPI1, SPI_FLAG_RBNE) != SET);
    
    return(spi_i2s_data_receive(SPI1));
}



void temp_spi_config(void)
{
    rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_GPIOB);
	
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 |GPIO_PIN_14| GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    gpio_bit_set(GPIOB, GPIO_PIN_0);

    spi_parameter_struct spi_init_struct;
    /* deinitilize SPI and the parameters */
    spi_struct_para_init(&spi_init_struct);

    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_2;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);

	spi_crc_polynomial_set(SPI1,7);
	spi_enable(SPI1);
}


void init_temp_sense(){

    FCLK_FAST();

    uint8_t data, conf;
    
    //Enable CS-pin
    temp_spi_config();

    delay_1ms(100);


    //Read config register
    gpio_bit_reset(GPIOB, GPIO_PIN_0);

    data = 0x00;
    //Set register
    read_write_spi(data);
    //Read register
    data = read_write_spi(data);

    gpio_bit_set(GPIOB, GPIO_PIN_0);

    delay_1ms(100);

    //Write config register, 3-wire, auto updata
    gpio_bit_reset(GPIOB, GPIO_PIN_0);

    conf = data;
    conf = ~0x2C;

    data = 0x80;
    read_write_spi(data);
    data = read_write_spi(conf);

    gpio_bit_set(GPIOB, GPIO_PIN_0);

    FCLK_FAST();
    
}

#define RTD_A 0.00385
#define RTD_0 100.0
#define RTD_REF 430.0

float temp_convert(uint16_t rtd){
    float Rt, temp;

    Rt = rtd;
    Rt /= 32768;
    Rt *= RTD_REF;

    temp = ((Rt/RTD_0)-1.0)/RTD_A;
    return temp;
}




uint16_t read_temp(){

    uint8_t temperature[2];
    uint8_t data;

    FCLK_SLOW();

    gpio_bit_reset(GPIOB, GPIO_PIN_0);

    data = 0x01;
    read_write_spi(data);
    temperature[0] = read_write_spi(data);

    gpio_bit_set(GPIOB, GPIO_PIN_0);
    delay_1us(500);
    gpio_bit_reset(GPIOB, GPIO_PIN_0);

    data = 0x02;
    read_write_spi(data);
    temperature[1] = read_write_spi(data);

    gpio_bit_set(GPIOB, GPIO_PIN_0);

    uint16_t rtd_reg = ((0x7F & temperature[0]) << 7) | ((0xFE & temperature[1]) >> 1);

    uint32_t rtd_value = rtd_reg * 430;

    rtd_value = rtd_value >> 15;

    FCLK_FAST();

    return rtd_reg;
}
