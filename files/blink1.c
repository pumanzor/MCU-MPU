#define RCC_APB2ENR   (*(volatile unsigned int *)0x40021018)
#define GPIOC_CRH     (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR     (*(volatile unsigned int *)0x4001100C)

int main(void)
{
    RCC_APB2ENR |= (1 << 4);

    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x2 << 20);

    while(1)
    {
        GPIOC_ODR |= (1 << 13);   // LED apagado (activo bajo)
        for(volatile int i = 0; i < 800000; i++);

        GPIOC_ODR &= ~(1 << 13);  // LED encendido
        for(volatile int i = 0; i < 800000; i++);
    }
}
