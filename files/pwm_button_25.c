/**
 * PWM controlado por botón - Versión 3
 *
 * PA9  = Salida PWM (Timer1 Canal 2) - ver en osciloscopio
 * PA10 = Entrada botón (pull-up interno, conectado a GND)
 * PC13 = LED indica el nivel con parpadeos
 *
 * Cada presión incrementa duty cycle:
 * 0% → 25% → 50% → 75% → 100% → 0%
 *
 * El LED parpadea N veces según el nivel (1-4 parpadeos)
 * Nivel 0 = LED apagado
 *
 * Diplomado MPU/CPU - ACAPOMIL
 */

#define RCC_APB2ENR     (*(volatile unsigned int *)0x40021018)
#define GPIOA_CRH       (*(volatile unsigned int *)0x40010804)
#define GPIOA_IDR       (*(volatile unsigned int *)0x40010808)
#define GPIOA_ODR       (*(volatile unsigned int *)0x4001080C)
#define GPIOC_CRH       (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR       (*(volatile unsigned int *)0x4001100C)

#define TIM1_CR1        (*(volatile unsigned int *)0x40012C00)
#define TIM1_CCMR1      (*(volatile unsigned int *)0x40012C18)
#define TIM1_CCER       (*(volatile unsigned int *)0x40012C20)
#define TIM1_PSC        (*(volatile unsigned int *)0x40012C28)
#define TIM1_ARR        (*(volatile unsigned int *)0x40012C2C)
#define TIM1_CCR2       (*(volatile unsigned int *)0x40012C38)
#define TIM1_BDTR       (*(volatile unsigned int *)0x40012C44)

void delay(volatile unsigned int count)
{
    while(count--);
}

void led_on(void)
{
    GPIOC_ODR &= ~(1 << 13);
}

void led_off(void)
{
    GPIOC_ODR |= (1 << 13);
}

void blink_times(int n)
{
    for(int i = 0; i < n; i++)
    {
        led_on();
        delay(100000);
        led_off();
        delay(100000);
    }
}

int main(void)
{
    // Clocks
    RCC_APB2ENR |= (1 << 2);    // GPIOA
    RCC_APB2ENR |= (1 << 4);    // GPIOC
    RCC_APB2ENR |= (1 << 11);   // Timer1

    // PA9 = AF Push-Pull (PWM)
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |= (0xB << 4);

    // PA10 = Input pull-up
    GPIOA_CRH &= ~(0xF << 8);
    GPIOA_CRH |= (0x8 << 8);
    GPIOA_ODR |= (1 << 10);

    // PC13 = Output
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x2 << 20);
    led_off();

    // PWM setup
    TIM1_PSC = 71;
    TIM1_ARR = 999;
    TIM1_CCMR1 |= (0x6 << 12);
    TIM1_CCMR1 |= (1 << 11);
    TIM1_CCER |= (1 << 4);
    TIM1_BDTR |= (1 << 15);
    TIM1_CCR2 = 0;
    TIM1_CR1 |= (1 << 0);

    // Test inicial
    blink_times(3);
    delay(300000);

    unsigned int duty_level = 0;
    unsigned int duty_values[] = {0, 250, 500, 750, 1000};
    int last_button = 1;

    while(1)
    {
        int button = (GPIOA_IDR >> 10) & 0x01;

        if (last_button == 1 && button == 0)
        {
            duty_level++;
            if (duty_level > 4) duty_level = 0;

            TIM1_CCR2 = duty_values[duty_level];

            // Feedback: parpadear según nivel
            if (duty_level == 0)
            {
                led_off();
            }
            else
            {
                blink_times(duty_level);
            }

            delay(200000);
        }

        last_button = button;
    }
}
