#define RCC_APB2ENR   (*(volatile unsigned int *)0x40021018)
#define GPIOC_CRH     (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR     (*(volatile unsigned int *)0x4001100C)

void delay_ms(int ms)
{
    for(int i = 0; i < ms; i++)
        for(volatile int j = 0; j < 800; j++);
}

void led_on(void)  { GPIOC_ODR &= ~(1 << 13); }
void led_off(void) { GPIOC_ODR |= (1 << 13); }

unsigned int seed = 12345;

unsigned int random(void)
{
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

int main(void)
{
    RCC_APB2ENR |= (1 << 4);
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x2 << 20);
    led_off();
    
    while(1)
    {
        int on_time = 50 + (random() % 200);
        int off_time = 50 + (random() % 500);
        
        led_on();
        delay_ms(on_time);
        led_off();
        delay_ms(off_time);
    }
}
