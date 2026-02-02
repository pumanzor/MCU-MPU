#define RCC_APB2ENR   (*(volatile unsigned int *)0x40021018)
#define GPIOC_CRH     (*(volatile unsigned int *)0x40011004)
#define GPIOC_ODR     (*(volatile unsigned int *)0x4001100C)

#define PWM_PERIOD    1000
#define FADE_STEPS    100

void delay(volatile int count)
{
    while(count--);
}

void pwm_cycle(int duty)
{
    int on_time = duty;
    int off_time = PWM_PERIOD - duty;
    
    if(on_time > 0)
    {
        GPIOC_ODR |= (1 << 13);
        delay(on_time);
    }
    
    if(off_time > 0)
    {
        GPIOC_ODR &= ~(1 << 13);
        delay(off_time);
    }
}

int main(void)
{
    RCC_APB2ENR |= (1 << 4);
    
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x2 << 20);
    
    int duty, step, repeat;
    
    while(1)
    {
        for(step = 0; step <= FADE_STEPS; step++)
        {
            duty = (step * PWM_PERIOD) / FADE_STEPS;
            for(repeat = 0; repeat < 50; repeat++)
                pwm_cycle(duty);
        }
        
        delay(20000);
        
        for(step = FADE_STEPS; step >= 0; step--)
        {
            duty = (step * PWM_PERIOD) / FADE_STEPS;
            for(repeat = 0; repeat < 50; repeat++)
                pwm_cycle(duty);
        }
        
        delay(20000);
    }
}
