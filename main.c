/*!******************************************************************
 * \file main.c
 * \brief Generic main template
 * \author Sens'it Team
 * \copyright Copyright (c) 2017 Sigfox, All Rights Reserved.
 *******************************************************************/
/******* INCLUDES **************************************************/
#include "sensit_api.h"
#include "click.h"
#include "battery.h"


/*******************************************************************/

void get_temp_humid_light(u8* payload)
{
	/* activate temp and humid sensor */
	SENSIT_ERR_NONE != SENSIT_API_temp_sensor_mode_active();
	/* activate light sensor */
	SENSIT_ERR_NONE != SENSIT_API_light_sensor_mode_active(LIGHT_SENSOR_SETTING_GAINx1);

	SENSIT_API_sleep(20);

	/* get temp and humid */
	s16 temp = 0;
	u16 humid = 0;
	SENSIT_ERR_NONE != SENSIT_API_temp_sensor_measure(&temp, &humid);

	union {
		float v;
		u8 c[4];
	} t;
	t.v = ((float)temp) / 8.0;
	union {
		u16 v;
		u8 c[2];
	} h;
	h.v = humid / 2;

	/* get light */
	u16 light_ch0 = 0;
	u16 light_ch1 = 0;	
	SENSIT_ERR_NONE != SENSIT_API_light_sensor_measure(&light_ch0, &light_ch1);

	union {
		u16 v;
		u8 c[2];
	} l0;
	l0.v = light_ch0;
	union {
		u16 v;
		u8 c[2];
	} l1;
	l1.v = light_ch1;

	payload[0] = t.c[0];
	payload[1] = t.c[1];
	payload[2] = t.c[2];
	payload[3] = t.c[3];
	payload[4] = h.c[0];
	payload[5] = h.c[1];
	payload[6] = l0.c[0];
	payload[7] = l0.c[1];
	payload[8] = l1.c[0];
	payload[9] = l1.c[1];

	/* sleep temp and humid sensor */
	SENSIT_ERR_NONE != SENSIT_API_temp_sensor_mode_sleep();
	/* sleep light sensor */
	SENSIT_ERR_NONE != SENSIT_API_light_sensor_mode_sleep()
}

int main()
{
    /* Start of variables declaration */

    u8 click;

    /* End of variables declaration */


    /* Start of initialization */

    /* Enable button */
    SENSIT_API_button_init(TRUE);

    /* Check the battery level at initialization */
    BATTERY_check(BATT_INIT_LEVEL);

    /* End of initialization */

    while (TRUE)
    {
        /* Start of application execution loop */

        if (WhoWakeMe == SRC_BUTTON)
        {
            /* The button was pressed, start count click */
            click = CLICK_count();
            if (click == 4)
            {
                /* Reset the device with a quadruple click */
                SENSIT_API_reset();
            }
        }


        /* Add your applicative code here */
		u8 payload[10];
    	u8 customer_response[8];
    	get_temp_humid_light(payload);

		SENSIT_API_send_sfx_frame(SFX_RC3,RED,payload,10,customer_response,3,FALSE);
		

        /* Put the device in deep sleep mode */
        //SENSIT_API_sleep(MAX_SLEEP_DURATION);
		SENSIT_API_sleep(300);

        /* Check the battery level is not too low */
        BATTERY_check(BATT_LOW_LEVEL);

        /* End of application execution loop */
    }
}

/*******************************************************************/
