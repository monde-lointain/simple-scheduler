static void ledOn(u32 pin)
{
    SET_BIT(GPIOD->ODR, 1UL << pin);
}

static void ledOff(u32 pin)
{
    CLEAR_BIT(GPIOD->ODR, 1UL << pin);
}

static void ledInitAll(void)
{
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN_Msk);

    SET_BIT(GPIOD->MODER, GPIO_MODER_MODER12_0); // green
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODER13_0); // orange
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODER14_0); // red
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODER15_0); // blue

    ledOff(LED_GREEN);
    ledOff(LED_ORANGE);
    ledOff(LED_RED);
    ledOff(LED_BLUE);
}