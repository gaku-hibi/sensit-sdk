# How to use Sen'it SDK -First Step-
[Sens'it](https://sensit.io/) is IoT device which have 5 sensors and one button, released by Sigfox.  
If you want to get it, please contact to [Sigfox Operator](https://www.sigfox.com/en/coverage) on your located. and then you can download Sens'it SDK.  
[download site](https://build.sigfox.com/sensit-for-developers/#sensit-sdk)
## Preparing for using Sens'it SDK
After downloding Sens'it SDK, unzip a sensit_sdk_v.1.0.0.zip and read a README.txt which shows the way of installing [GNU Arm Embedded Toolchain (gcc-arm-none-eabi)](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) and [dfu-util](https://sourceforge.net/projects/dfu-util/).
## Upgrade sens'it
For porting your original program on sens'it, you **MUST** upgrade your sens'it.  
To upgrade your Sens'it:
1. Connect the device to your PC using a USB cable.
2. Put your device in bootloader mode.
3. Use the 'make upgrade' command to upgrade your Sens'it.
### bootloader mode
You can change the mode by pushing the button on the front of sens'it.
1. Click 4 times on the Sens'it front button quickly.<br>if successed the white LED at left side will be blinking.
2. During blinking, press the button until both LEDs (top and left side) become white.
If it sccess, you can see the sens'it changed to bootloader mode, shown below.  
![sens it boot loader mode](https://user-images.githubusercontent.com/11493192/35894092-85d2df6a-0bf4-11e8-8aca-ba2035688c62.JPG)
### make upgrade
To upgrade your Sens'it, command "make upgrade" on your PC.  
```>make upgrade```  
If upgrading is successed first time, Sen'it LED will be "GREEN".
## Editing source code
The source code, I commited, is one edited "main.c" in the "src" folder of sens'it sdk.
Original one is following...
```c
#include "sensit_api.h"
#include "click.h"
#include "battery.h"

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


        /* Put the device in deep sleep mode */
        SENSIT_API_sleep(MAX_SLEEP_DURATION);

        /* Check the battery level is not too low */
        BATTERY_check(BATT_LOW_LEVEL);

        /* End of application execution loop */
    }
}
```
Let look this code.
Outline of the original code consist "intializing" and "while loop".  
Basically, you can write your custom code at the line after "/*Add your applicative code here*/".
```SENSIT_API_sleep(MAX_SLEEP_DURATION);``` means that sens'it sleep deeply. but when the button is clicked, sens'it wakes up. ```if (WhoWakeMe == SRC_BUTTON)```
If you click 4 times on the sens'it button, Sen'it reset. In other words, it's **"bootloader mode"**!  

## Sample code
Sample code shows the way to get some sensor data and send the data via Sigtox Network.
#### Send a Sigfox message
> Sigfox messages are designed to be smaller, optimised for sensors and require only a small amount of power to transmit. The Sigfox payload is limited to 12 bytes (excluding the payload headers).  
So you define array of 12 bytes or less ```u8 payload[12]```.  
For your information, downlink payload size is 8 bytes or less.
```c
/* Add your applicative code here */
u8 payload[10];
u8 customer_response[8];
get_temp_humid_light(payload);

SENSIT_API_send_sfx_frame(SFX_RC1,RED,payload,10,customer_response,3,FALSE);

/* Put the device in deep sleep mode */
//SENSIT_API_sleep(MAX_SLEEP_DURATION);
SENSIT_API_sleep(300);
```
You can send a sigfox message by "SENSIT_API_send_sfx_frame" defined in /inc/sdk/sensit_api.h file.  

```error_t SENSIT_API_send_sfx_bit(sfx_rc_e rc, color_e led_toggle, bool bit_value, u8* customer_response, u8 tx_repeat, bool initiate_downlink_flag)```  

Param sfx_rc_e means ["Radio Configuration Zone"](http://makers.sigfox.com/about/) which is configuration for each regional reguration.
You must set a proper RCZ for your countries. If you located in Europe, it set "RCZ1".  
If you get sonor data and send it continouslly, you set actual time (for exapmle: 300 sec) at SENSIT_API_sleep param.  
### Get Sensor data
Sens'it is equipped with the follwing sensors.  
- Temparature and Humidity : ST HTS221
- Light : LiteON LTR329
- Accelerometer : NXP FXOS8700  

This sample code gets temparature, humidity and light. (for easy understanding, I removed all exception handling code.)  
```c
void get_temp_humid_light(u8* payload)
{
	/* activate temp and humid sensor */
	SENSIT_API_temp_sensor_mode_active();
	/* activate light sensor */
	SENSIT_API_light_sensor_mode_active(LIGHT_SENSOR_SETTING_GAINx1);

	SENSIT_API_sleep(20);

	/* get temp and humid */
	s16 temp = 0;
	u16 humid = 0;
	SENSIT_API_temp_sensor_measure(&temp, &humid);

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
	SENSIT_API_light_sensor_measure(&light_ch0, &light_ch1);

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
	SENSIT_API_temp_sensor_mode_sleep();

	/* sleep light sensor */
	SENSIT_API_light_sensor_mode_sleep();
}
```
To access the API documentation, open the file 'doc/doxygen/index.html' in your browser.  

## Compile your source code
You just only command ```make```.

## Flash your program into Sens'it
To flash your program to Sens'it,
1. Connect the device to your PC using a USB cable.  
2. Put your Sens'it in bootloader mode.  
3. Use the 'make prog' command to flash your firmware into your device.

The message sent by this sample is stored in [Sigfox backend cloud](https://backend.sigfox.com), and you can see in MESSAGES page.
