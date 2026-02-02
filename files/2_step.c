#define RCC_APB2ENR   (*(volatile unsigned int *)0x40021018)
#define GPIOC_CRH     (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR     (*(volatile unsigned int *)0x4001100C)

void delay_ms(int ms)
{
    // Aproximado a 8MHz (clock default)
    for(int i = 0; i < ms; i++)
        for(volatile int j = 0; j < 800; j++);
}

void led_on(void)
{
    GPIOC_ODR &= ~(1 << 13);  // Active low
}

void led_off(void)
{
    GPIOC_ODR |= (1 << 13);
}

int main(void)
{
    RCC_APB2ENR |= (1 << 4);
    
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x2 << 20);
    
    led_off();
    
    while(1)
    {
        // Parpadeo 1
        led_on();
        delay_ms(250);
        led_off();
        delay_ms(250);
        
        // Parpadeo 2
        led_on();
        delay_ms(250);
        led_off();
        delay_ms(250);
        
        // Espera 1 segundo apagado
        delay_ms(1000);
    }
}

