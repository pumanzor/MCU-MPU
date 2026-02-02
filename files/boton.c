#define RCC_APB2ENR   (*(volatile unsigned int *)0x40021018)
#define GPIOA_CRL     (*(volatile unsigned int *)0x40010800)
#define GPIOA_IDR     (*(volatile unsigned int *)0x40010808)
#define GPIOC_CRH     (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR     (*(volatile unsigned int *)0x4001100C)

void delay_ms(int ms)
{
    for(int i = 0; i < ms; i++)
        for(volatile int j = 0; j < 800; j++);
}

int main(void)
{
    // Habilitar GPIOA y GPIOC
    RCC_APB2ENR |= (1 << 2) | (1 << 4);
    
    // PA0 como entrada con pull-up
    GPIOA_CRL &= ~(0xF << 0);
    GPIOA_CRL |= (0x8 << 0);  // Input pull-up/pull-down
    
    // PC13 como salida
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x2 << 20);
    
    while(1)
    {
        if(GPIOA_IDR & (1 << 0))  // PA0 HIGH
        {
            GPIOC_ODR |= (1 << 13);   // LED OFF
        }
        else  // PA0 LOW (botón presionado)
        {
            GPIOC_ODR &= ~(1 << 13);  // LED ON
        }
        delay_ms(10);  // Debounce
    }
}
