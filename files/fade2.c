/**
 * PWM Fade en STM32F103 - Bare Metal
 * Timer1 Canal 2 - Pin PA9
 *
 * Este código genera una señal PWM con duty cycle variable
 * para crear un efecto de fade (respiración) en un LED
 *
 * Diplomado MPU/CPU - ACAPOMIL
 */

// Registros RCC (Reset and Clock Control)
#define RCC_APB2ENR     (*(volatile unsigned int *)0x40021018)

// Registros GPIO Puerto A
#define GPIOA_CRH       (*(volatile unsigned int *)0x40010804)

// Registros Timer1
#define TIM1_CR1        (*(volatile unsigned int *)0x40012C00)  // Control register 1
#define TIM1_CCMR1      (*(volatile unsigned int *)0x40012C18)  // Capture/compare mode register 1
#define TIM1_CCER       (*(volatile unsigned int *)0x40012C20)  // Capture/compare enable register
#define TIM1_PSC        (*(volatile unsigned int *)0x40012C28)  // Prescaler
#define TIM1_ARR        (*(volatile unsigned int *)0x40012C2C)  // Auto-reload register
#define TIM1_CCR2       (*(volatile unsigned int *)0x40012C38)  // Capture/compare register 2
#define TIM1_BDTR       (*(volatile unsigned int *)0x40012C44)  // Break and dead-time register

// Parámetros del PWM
#define PWM_FREQUENCY   1000    // 1 kHz - fácil de ver en osciloscopio
#define PWM_RESOLUTION  1000    // 1000 pasos de duty cycle (0.1% resolución)
#define FADE_DELAY      5000    // Velocidad del fade

void delay(volatile int count)
{
    while(count--);
}

void setup_clock(void)
{
    // Habilitar reloj para GPIOA y Timer1
    // Bit 2 = IOPAEN (GPIOA)
    // Bit 11 = TIM1EN (Timer1)
    RCC_APB2ENR |= (1 << 2);    // GPIOA
    RCC_APB2ENR |= (1 << 11);   // TIM1
}

void setup_gpio(void)
{
    // PA9 como Alternate Function Push-Pull
    // CRH controla pines 8-15
    // PA9 está en bits 4-7 del CRH
    // Modo: 10 = Output 2MHz
    // CNF:  10 = Alternate function push-pull
    // Valor: 1010 = 0xA

    GPIOA_CRH &= ~(0xF << 4);   // Limpiar bits 4-7
    GPIOA_CRH |= (0xA << 4);    // AF push-pull, 2MHz
}

void setup_timer1_pwm(void)
{
    // Timer1 corre a 72MHz (asumiendo clock por defecto del sistema)
    // Queremos PWM de 1kHz con resolución de 1000 pasos
    //
    // Frecuencia PWM = Timer_Clock / ((PSC+1) * (ARR+1))
    // 1000 = 72000000 / ((PSC+1) * 1000)
    // PSC+1 = 72000000 / (1000 * 1000) = 72
    // PSC = 71

    // Prescaler: divide el clock
    TIM1_PSC = 71;              // 72MHz / 72 = 1MHz timer clock

    // Auto-reload: define el período (y resolución)
    TIM1_ARR = PWM_RESOLUTION - 1;  // 1000 - 1 = 999, período de 1ms = 1kHz

    // Configurar Canal 2 en modo PWM
    // CCMR1 bits 12-14 = OC2M (Output Compare 2 Mode)
    // 110 = PWM mode 1 (activo mientras CNT < CCR2)
    // Bit 11 = OC2PE (Output Compare 2 Preload Enable)
    TIM1_CCMR1 &= ~(0x7 << 12);     // Limpiar OC2M
    TIM1_CCMR1 |= (0x6 << 12);      // PWM mode 1
    TIM1_CCMR1 |= (1 << 11);        // Preload enable

    // Habilitar salida del Canal 2
    // Bit 4 = CC2E (Capture/Compare 2 output Enable)
    TIM1_CCER |= (1 << 4);

    // Timer1 es "advanced timer" - necesita habilitar MOE
    // BDTR bit 15 = MOE (Main Output Enable)
    TIM1_BDTR |= (1 << 15);

    // Duty cycle inicial = 0%
    TIM1_CCR2 = 0;

    // Habilitar el timer
    // CR1 bit 0 = CEN (Counter Enable)
    TIM1_CR1 |= (1 << 0);
}

void set_duty_cycle(unsigned int duty)
{
    // duty: 0 a 1000 (0% a 100%)
    if (duty > PWM_RESOLUTION) duty = PWM_RESOLUTION;
    TIM1_CCR2 = duty;
}

int main(void)
{
    setup_clock();
    setup_gpio();
    setup_timer1_pwm();

    unsigned int duty = 0;
    int direction = 1;  // 1 = subiendo, -1 = bajando

    while(1)
    {
        set_duty_cycle(duty);

        // Fade up/down
        if (direction == 1)
        {
            duty += 10;
            if (duty >= PWM_RESOLUTION)
            {
                duty = PWM_RESOLUTION;
                direction = -1;
            }
        }
        else
        {
            duty -= 10;
            if (duty == 0 || duty > PWM_RESOLUTION)  // underflow check
            {
                duty = 0;
                direction = 1;
            }
        }

        delay(FADE_DELAY);
    }
}
