/* Elehobica Sample
   Modified by Elehobica (elehobica@gmail.com) for Android_HP_Button sample source
*/

/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_unit_t adc_unit = ADC_UNIT_2; // ADC_UNIT_1 or ADC_UNIT_2
static const adc_channel_t channel = ADC_CHANNEL_6; // GPIO34 for ADC1, GPIO14 for ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_11; // 3.3V full scale

#define NUM_OF_SAMPLES 32 // For Multisampling
#define DEFAULT_VREF 1100 // For better estimation of adc2_vref_to_gpio()
#define HP_BUTTON_OPEN     0
#define HP_BUTTON_CENTER   1
#define HP_BUTTON_D        2
#define HP_BUTTON_PLUS     3
#define HP_BUTTON_MINUS    4
#define NUM_BTN_HISTORY    30 // for count_center_clicks()

uint8_t button_prv[NUM_BTN_HISTORY] = {}; // initialized as HP_BUTTON_OPEN
uint32_t button_repeat_count = 0;

// Task Handles
TaskHandle_t th;

static uint32_t adc_get_hp_button()
{
  uint32_t adc_reading = 0;
  int raw;
  uint32_t voltage;
  uint32_t ret;
  //Multisampling
  for (int i = 0; i < NUM_OF_SAMPLES; i++) {
    if (adc_unit == ADC_UNIT_1) {
      raw = adc1_get_raw((adc1_channel_t) channel);
    } else {
      adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
    }
    adc_reading += raw;
  }
  adc_reading /= NUM_OF_SAMPLES;
  //Convert adc_reading to voltage in mV
  voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  //Serial.println(voltage);
  // Android Headphone button conditions
  // 3.3V pull-up
  if (voltage < 140) { // < 140mV (CENTER: 0mV)
      ret = HP_BUTTON_CENTER;
  } else if (voltage >= 142 && voltage < 238) { // 142mv ~ 238mV (D: 190mV)
      ret = HP_BUTTON_D;
  } else if (voltage >= 240 && voltage < 400) { // 240mV ~ 400mV (PLUS: 320mV)
      ret = HP_BUTTON_PLUS;
  } else if (voltage >= 435 && voltage < 725) { // 435mV ~ 725mV (MINUS: 580mV)
      ret = HP_BUTTON_MINUS;
  } else { // others
      ret = HP_BUTTON_OPEN;
  }
  return ret;
}

static int count_center_clicks(void)
{
  int i;
  int detected_fall = 0;
  int count = 0;
  for (i = 0; i < 4; i++) {
    if (button_prv[i] != HP_BUTTON_OPEN) {
      return 0;
    }
  }
  for (i = 4; i < NUM_BTN_HISTORY; i++) {
    if (detected_fall == 0 && button_prv[i-1] == HP_BUTTON_OPEN && button_prv[i] == HP_BUTTON_CENTER) {
      detected_fall = 1;
    } else if (detected_fall == 1 && button_prv[i-1] == HP_BUTTON_CENTER && button_prv[i] == HP_BUTTON_OPEN) {
      count++;
      detected_fall = 0;
    }
  }
  if (count > 0) {
    for (i = 0; i < NUM_BTN_HISTORY; i++) button_prv[i] = HP_BUTTON_OPEN;
  }
  return count;
}

void task_get_hp_button_status(void *pvParameters)
{
  int i;
  int center_clicks;
  char str[256];
  // Center Button: event timing is at button release
  // Other Buttons: event timing is at button push
  for (int count = 0; ; count++) {
    uint8_t button = adc_get_hp_button();
    if (button == HP_BUTTON_OPEN) { // count center clicks
      button_repeat_count = 0;
      center_clicks = count_center_clicks(); // must be called once per tick because button_prv[] status has changed
      if (center_clicks > 0) {
        sprintf(str, "CENTER clicks =  %d", center_clicks);
        Serial.println(str);
      }
    } else if (button_prv[0] == HP_BUTTON_OPEN) { // push
      if (button == HP_BUTTON_D || button == HP_BUTTON_PLUS) {
        Serial.println("PLUS/D");
      } else if (button == HP_BUTTON_MINUS) {
        Serial.println("MINUS");
      }
    } else if (button_repeat_count == 10) { // long push
      if (button == HP_BUTTON_CENTER) {
        button_repeat_count++; // only once and step to longer push event
      } else if (button == HP_BUTTON_D || button == HP_BUTTON_PLUS) {
        // keep long push
        Serial.println("Long push PLUS/D");
      } else if (button == HP_BUTTON_MINUS) {
        // keep long push
        Serial.println("Long push MINUS");
      }
    } else if (button_repeat_count == 30) { // long long push
      if (button == HP_BUTTON_CENTER) {
        Serial.println("Long Long push CENTER");
      }
      button_repeat_count++; // only once and step to longer push event
    } else if (button == button_prv[0]) {
      button_repeat_count++;
    }
    // Button status shift
    for (i = NUM_BTN_HISTORY-2; i >= 0; i--) {
      button_prv[i+1] = button_prv[i];
    }
    button_prv[0] = button;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

static void init_adc()
{
  //Configure ADC
  if (adc_unit == ADC_UNIT_1) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten((adc1_channel_t) channel, atten);
  } else {
    adc2_config_channel_atten((adc2_channel_t) channel, atten);
  }

  //Characterize ADC
  adc_chars = (esp_adc_cal_characteristics_t *) calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(adc_unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
  //print_char_val_type(val_type);
}

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200);
  init_adc();  
  xTaskCreate(task_get_hp_button_status, "task_get_hp_button_status", 2048, NULL, 5, NULL);
}

// the loop function runs over and over again forever
void loop()
{
}
