
# The compressed instruction set is disabled here in order to force the
# assembler to encode all jumps in 4 bytes, rather than shortening "j ." to 2
# bytes.
.option norelax
.option norvc

.section .text._vectorJumpArea, "ax"
.global _vectorJumpArea
.type _vectorJumpArea, @object

_vectorJumpArea:
	j     _start                  #  0: (entry point)
	j     .                       #  1:
	j     .                       #  2: IRQ_NMI
	j     .                       #  3: IRQ_HARDFAULT
	j     .                       #  4:
	j     .                       #  5:
	j     .                       #  6:
	j     .                       #  7:
	j     .                       #  8:
	j     .                       #  9:
	j     .                       # 10:
	j     .                       # 11:
	j     .                       # 12: IRQ_SYSTICK
	j     .                       # 13:
	j     .                       # 14: IRQ_SW
	j     .                       # 15:
	j     .                       # 16: IRQ_WWDG
	j     .                       # 17: IRQ_PVD
	j     .                       # 18: IRQ_FLASH
	j     .                       # 19: IRQ_RCC
	j     .                       # 20: IRQ_EXTI
	j     .                       # 21: IRQ_AWU
	j     .                       # 22: IRQ_DMA1_CH1
	j     .                       # 23: IRQ_DMA1_CH2
	j     .                       # 24: IRQ_DMA1_CH3
	j     .                       # 25: IRQ_DMA1_CH4
	j     .                       # 26: IRQ_DMA1_CH5
	j     .                       # 27: IRQ_DMA1_CH6
	j     .                       # 28: IRQ_DMA1_CH7
	j     .                       # 29: IRQ_ADC
	j     handleI2CEventInterrupt # 30: IRQ_I2C1_EV
	j     handleI2CErrorInterrupt # 31: IRQ_I2C1_ER
	j     .                       # 32: IRQ_USART1
	j     .                       # 33: IRQ_SPI1
	j     .                       # 34: IRQ_TIM1BRK
	j     .                       # 35: IRQ_TIM1UP
	j     .                       # 36: IRQ_TIM1TRG
	j     .                       # 37: IRQ_TIM1CC
	j     .                       # 38: IRQ_TIM2
