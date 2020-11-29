#include "lcd/lcd.h"
#include "fatfs/tf_card.h"
#include <string.h>


unsigned char image[12800];
FATFS fs;

int _put_char(int ch)
{
    usart_data_transmit(USART0, (uint8_t) ch );
    while ( usart_flag_get(USART0, USART_FLAG_TBE) == RESET) {
    }
    return ch;
}

void init_uart0(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);


    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    usart_interrupt_enable(USART0, USART_INT_RBNE);
}

// PWM for PB10 (Timer1 Ch2: partial remap1, alternate function)
void timer1_pwm_init(void)
{
    timer_parameter_struct tpa;

    rcu_periph_clock_enable(RCU_TIMER1);
    timer_struct_para_init(&tpa);
    tpa.prescaler = 108 - 1;        // prescaler (108MHz -> 1MHz)
    tpa.period = 100 - 1;           // max value of counting up (1MHz -> 10KHz = 100us)
    timer_init(TIMER1, &tpa);
    timer_channel_output_state_config(TIMER1, TIMER_CH_2, TIMER_CCX_ENABLE); // channel output enable
    timer_channel_output_mode_config(TIMER1, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    // default PWM ratio = 50 / 100
    timer_autoreload_value_config(TIMER1, 99);
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, 50);
    timer_enable(TIMER1);

    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_TIMER1_PARTIAL_REMAP1, ENABLE);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
}

// on_term: 0 ~ 100 (%)
void timer1_pwm_set_ratio(uint16_t on_term)
{
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, on_term);
}

int main(void)
{
    int j = 0;
    uint8_t mount_is_ok = 1; /* 0: mount successful ; 1: mount failed */
    FIL fil;
    FRESULT fr;     /* FatFs return code */
    UINT br;

    rcu_periph_clock_enable(RCU_GPIOA);
    //rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1 | GPIO_PIN_2);
    //gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    //gpio_bit_set(GPIOB, GPIO_PIN_10);

    init_uart0();
    timer1_pwm_init();
    timer1_pwm_set_ratio(50);

    Lcd_Init();
    LCD_Clear(WHITE);
    BACK_COLOR = WHITE;
    setRotation(1);

    LEDR(1);
    LEDG(1);
    LEDB(1);

    fr = f_mount(&fs, "", 1);
    if (fr == 0)
        mount_is_ok = 0;
    else
        mount_is_ok = 1;

    if (mount_is_ok == 0) {
        while (1) {
            fr = f_open(&fil, "logo.bin", FA_READ);
            if (fr) {
                printf("open error: %d!\n\r", (int)fr);
                break;
            } else {
                LCD_Address_Set(0, 0, 239, 134);
                while (1) {
                    fr = f_read(&fil, image, sizeof(image), &br);
                    if (br > 0) {
                        for (int i = 0; i < br; i++) {
                            LCD_WR_DATA8(image[i]);
                        }
                    } else {
                        break;
                    }
                    j++;
                    if (j >= 100*4) j = 0;
                    if (j < 50*4) {
                        timer1_pwm_set_ratio(j/4);
                    } else {
                        timer1_pwm_set_ratio(99 - j/4);
                    }
                }
                f_close(&fil);
            }
        }
    } else {
        LCD_ShowString(0,  0, (u8 *)("no card found!"), BLACK);
        LCD_ShowString(0, 16, (u8 *)("no card found!"), BLUE);
        LCD_ShowString(0, 32, (u8 *)("no card found!"), BRED);
        LCD_ShowString(0, 48, (u8 *)("no card found!"), GBLUE);
        LCD_ShowString(0, 64, (u8 *)("no card found!"), RED);
    }
    while (1) {
        LEDR_TOG;
        delay_1ms(200);
        LEDG_TOG;
        delay_1ms(200);
        LEDB_TOG;
        delay_1ms(200);
    }
}


