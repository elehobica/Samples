#include "gd32vf103_rcu.h"
#include "gd32vf103_gpio.h"
#include "gd32vf103_spi.h"
#include "gd32vf103_dma.h"

#define SIZE_OF_SAMPLES 512  // samples for 2ch
#define SAMPLE_RATE     (44100)
#define WAVE_FREQ_HZ    (440)
#define PI              (3.14159265)
#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)
#define DELTA 2.0*PI*WAVE_FREQ_HZ/SAMPLE_RATE

int32_t audio_buf0[SIZE_OF_SAMPLES];
int32_t audio_buf1[SIZE_OF_SAMPLES];

static double ang = 0;
static int count = 0;
static double triangle_float = 0.0;

union U {
    uint32_t i;
    uint16_t s[2];
} u;

// 16bit swap
uint32_t swap16b(uint32_t in_val)
{
    u.i = in_val;
    return ((uint32_t) u.s[0] << 16) | ((uint32_t) u.s[1]);
}

double _square_wave(void)
{
    double dval;
    if (ang >= 2.0*PI) {
        ang -= 2.0*PI;
        triangle_float = -(double) pow(2, 22);
    }
    if (ang < PI) {
        dval = 1.0;
    } else {
        dval = -1.0;
    }
    return dval;
}

// Generate Triangle Wave
void setup_triangle_sine_waves(int32_t *samples_data)
{
    unsigned int i;
    double square_float;
    double triangle_step = (double) pow(2, 23) / SAMPLE_PER_CYCLE;

    for(i = 0; i < SIZE_OF_SAMPLES/2; i++) {
        square_float = _square_wave();
        ang += DELTA;
        if (square_float >= 0) {
            triangle_float += triangle_step;
        } else {
            triangle_float -= triangle_step;
        }

        square_float *= (pow(2, 23) - 1);
        samples_data[i*2+0] = swap16b((int) square_float * 256);
        samples_data[i*2+1] = swap16b((int) triangle_float * 256);
    }
}

void prepare_audio_buf(void)
{
    setup_triangle_sine_waves(audio_buf0);
    setup_triangle_sine_waves(audio_buf1);

    init_i2s2();
    init_dma_i2s2(audio_buf0, SIZE_OF_SAMPLES*2);

    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
    dma_channel_enable(DMA1, DMA_CH1);
    count = 0;
}

void run_audio_buf(void)
{
    if (SET == dma_flag_get(DMA1, DMA_CH1, DMA_FLAG_FTF)) {
        dma_flag_clear(DMA1, DMA_CH1, DMA_FLAG_FTF);
        dma_channel_disable(DMA1, DMA_CH1);
        if (count % 2 == 0) {
            init_dma_i2s2(audio_buf1, SIZE_OF_SAMPLES*2);
        } else {
            init_dma_i2s2(audio_buf0, SIZE_OF_SAMPLES*2);
        }
        dma_channel_enable(DMA1, DMA_CH1);
        if (count % 2 == 0) {
           setup_triangle_sine_waves(audio_buf0);
        } else {
           setup_triangle_sine_waves(audio_buf1);
        }
        count++;
    }
}

dma_parameter_struct dma_param;

void init_dma_i2s2(uint32_t memory_addr, uint32_t trans_number)
{
    rcu_periph_clock_enable(RCU_DMA1);

    dma_struct_para_init(&dma_param);
    dma_param.periph_addr = &SPI_DATA(SPI2);
    dma_param.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_param.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_param.memory_addr = memory_addr;
    dma_param.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_param.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_param.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_param.number = trans_number;
    dma_param.priority = DMA_PRIORITY_HIGH;
    dma_init(DMA1, DMA_CH1, &dma_param);
}

void init_i2s2(void)
{
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI2);

    RCU_CTL &= ~(RCU_CTL_PLL1EN | RCU_CTL_PLL2EN);
    rcu_predv1_config(RCU_PREDV1_DIV2);
    rcu_pll2_config(RCU_PLL2_MUL12);
    rcu_i2s2_clock_config(RCU_I2S2SRC_CKPLL2_MUL2);
    RCU_CTL |= (RCU_CTL_PLL1EN | RCU_CTL_PLL2EN);

    gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP, ENABLE);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);    

    i2s_init(SPI2, I2S_MODE_MASTERTX, I2S_STD_PHILLIPS, I2S_CKPL_HIGH);
    i2s_psc_config(SPI2, I2S_AUDIOSAMPLE_44K, I2S_FRAMEFORMAT_DT24B_CH32B, I2S_MCKOUT_DISABLE);
    i2s_enable(SPI2);
}

int main(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1|GPIO_PIN_2);

    prepare_audio_buf();

    while (1) {
        run_audio_buf();
    }
    return 0;
}
