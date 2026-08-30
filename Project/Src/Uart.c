#include "Uart.h"
#include "string.h"
#define UE_BIT 13
#define TE_BIT 3
#define RE_BIT 2
#define USART2NE 17
#define CR1_UE (1U<<UE_BIT)
#define CR1_TE (1U<<TE_BIT)
#define CR1_RE (1U<<RE_BIT)
#define SR_TXE (1U<<7)
#define SR_RXNE (1U<<5)
#define APB1EN_USART2EN (1U<<USART2NE)
#define UART2_TX (0xB<<8)
#define FLOATING_INPUT 0x4
#define ALTARNATE_FUNCTION_PP_50MHZ 0xb
#define CRL_PIN2  8
#define CRL_PIN3  12
#define GPIOA_PIN2_UART2TX_ALF     (ALTARNATE_FUNCTION_PP_50MHZ<<CRL_PIN2)
#define GPIOA_PIN3_USART2RX_FL     (FLOATING_INPUT<<CRL_PIN3)

void USART2_INIT(void)
{
	//RCC->APB2ENR |=GPIOAEN;
	GPIOA->CRL &=~(0xFFFFFFFF<<0); //Clear CRL register related to GPIOA
	GPIOA->CRL |= (GPIOA_PIN2_UART2TX_ALF | GPIOA_PIN3_USART2RX_FL);//Configurati
	RCC->APB1ENR |= APB1EN_USART2EN;
	USART2->BRR =0x0EA6; //BaudRate=9600  APB1=36Mhz
	USART2->CR1 |= CR1_TE;
	USART2->CR1 |= CR1_RE;
	USART2->CR1 |= CR1_UE;
	return;
}

void usart_write(int ch)
{
	while(!(USART2->SR & SR_TXE));
	USART2->DR =(ch & 0xff);
	return;
}
void usart_write_string (char *str)
{
	for (uint8_t i=0; i<=strlen(str); i++)
	{
		while(!(USART2->SR & SR_TXE));
		USART2->DR =(str[i] & 0xff);
	}
	return;
}
void usart_Write_char (char ch)
{
	while(!(USART2->SR & SR_TXE));
	USART2->DR =(ch & 0xff);
	return;
}
char Read_Uasrt(void)
{
	while(!(USART2->SR & SR_RXNE));
	return USART2->DR;
}
