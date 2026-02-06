/**
 * PWM controlado por botón - Bare Metal
 * 
 * PA9  = Salida PWM (Timer1 Canal 2)
 * PA10 = Entrada botón (pull-up interno, conectado a GND)
 * PC13 = LED onboard (muestra el nivel actual)
 * 
 * Cada presión del botón incrementa el duty cycle en 25%:
 * 0% → 25% → 50% → 75% → 100% → 0% → ...
 * 
 * Incluye debounce básico para evitar múltiples lecturas
 * 
 * Diplomado MPU/CPU - ACAPOMIL
 */

// Registros RCC
#define RCC_APB2ENR     (*(volatile unsigned int *)0x40021018)

// Registros GPIO Puerto A
#define GPIOA_CRH       (*(volatile unsigned int *)0x40010804)
#define GPIOA_IDR       (*(volatile unsigned int *)0x40010808)
#define GPIOA_ODR       (*(volatile unsigned int *)0x4001080C)

// Registros GPIO Puerto C
#define GPIOC_CRH       (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR       (*(volatile unsigned int *)0x4001100C)

// Registros Timer1
#define TIM1_CR1        (*(volatile unsigned int *)0x40012C00)
#define TIM1_CCMR1      (*(volatile unsigned int *)0x40012C18)
#define TIM1_CCER       (*(volatile unsigned int *)0x40012C20)
#define TIM1_PSC        (*(volatile unsigned int *)0x40012C28)
#define TIM1_ARR        (*(volatile unsigned int *)0x40012C2C)
#define TIM1_CCR2       (*(volatile unsigned int *)0x40012C38)
#define TIM1_BDTR       (*(volatile unsigned int *)0x40012C44)

// Parámetros PWM
#define PWM_RESOLUTION  1000    // 1000 pasos (0-1000)
#define DUTY_STEP       250     // 25% = 250 de 1000

void delay(volatile int count)
{
    while(count--);
}

void setup_clocks(void)
{
    RCC_APB2ENR |= (1 << 2);    // GPIOA
    RCC_APB2ENR |= (1 << 4);    // GPIOC
    RCC_APB2ENR |= (1 << 11);   // Timer1
}

void setup_gpio(void)
{
    // PA9 = Alternate Function Push-Pull (PWM output)
    // Bits 4-7 en CRH
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |= (0xB << 4);    // 1011 = AF push-pull, 50MHz
    
    // PA10 = Input con pull-up
    // Bits 8-11 en CRH
    GPIOA_CRH &= ~(0xF << 8);
    GPIOA_CRH |= (0x8 << 8);    // 1000 = Input pull-up/pull-down
    GPIOA_ODR |= (1 << 10);     // Activar pull-up
    
    // PC13 = Output (LED onboard)
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);
}

void setup_pwm(void)
{
    // Prescaler: 72MHz / 72 = 1MHz
    TIM1_PSC = 71;
    
    // Período: 1000 pasos = 1kHz
    TIM1_ARR = PWM_RESOLUTION - 1;
    
    // Canal 2 en modo PWM 1
    TIM1_CCMR1 &= ~(0x7 << 12);
    TIM1_CCMR1 |= (0x6 << 12);  // PWM mode 1
    TIM1_CCMR1 |= (1 << 11);    // Preload enable
    
    // Habilitar salida canal 2
    TIM1_CCER |= (1 << 4);
    
    // MOE - Main Output Enable (obligatorio para Timer1)
    TIM1_BDTR |= (1 << 15);
    
    // Duty inicial = 0%
    TIM1_CCR2 = 0;
    
    // Encender timer
    TIM1_CR1 |= (1 << 0);
}

void set_duty(unsigned int duty)
{
    if (duty > PWM_RESOLUTION) duty = PWM_RESOLUTION;
    TIM1_CCR2 = duty;
}

int read_button(void)
{
    // Retorna 1 si botón presionado (PA10 = LOW porque pull-up a GND)
    return !((GPIOA_IDR >> 10) & 0x01);
}

int main(void)
{
    setup_clocks();
    setup_gpio();
    setup_pwm();
    
    unsigned int duty_level = 0;    // 0, 1, 2, 3, 4 (representa 0%, 25%, 50%, 75%, 100%)
    unsigned int duty_values[] = {0, 250, 500, 750, 1000};  // Valores de duty cycle
    
    int button_pressed = 0;
    int last_button = 0;
    
    // LED inicial apagado
    GPIOC_ODR |= (1 << 13);
    
    while(1)
    {
        button_pressed = read_button();
        
        // Detectar flanco de bajada (botón recién presionado)
        if (button_pressed && !last_button)
        {
            // Incrementar nivel
            duty_level++;
            if (duty_level > 4)
            {
                duty_level = 0;
            }
            
            // Aplicar nuevo duty cycle
            set_duty(duty_values[duty_level]);
            
            // Feedback visual con LED
            // Parpadear según nivel (0=apagado, 4=encendido fijo)
            if (duty_level == 0)
            {
                GPIOC_ODR |= (1 << 13);     // LED off
            }
            else
            {
                GPIOC_ODR &= ~(1 << 13);    // LED on
            }
            
            // Debounce: esperar a que se estabilice
            delay(100000);
        }
        
        last_button = button_pressed;
    }
}
