#include "stm32f10x.h"
#include <stdio.h>
 
//#include "delay.h"  da doi sang systemTick

#define FLASH_SAVE_ADDR  0x0801FC00 

#define LED_RED GPIO_Pin_12
#define LED_GREEN GPIO_Pin_13

#define LCD_RS GPIO_Pin_14
#define LCD_EN GPIO_Pin_15
#define LCD_D5 GPIO_Pin_5
#define LCD_D6 GPIO_Pin_6
#define LCD_D7 GPIO_Pin_7
#define LCD_D8 GPIO_Pin_8

#define LCD_PORT GPIOB

#define LEVEL_PPM 1500

void Send_Blynk();
uint16_t ppm = 0;

void delay_ms(uint16_t ms)
{
    SysTick->LOAD = 72000 - 1;  // 72MHz -> 1ms (72,000 tick)
    SysTick->VAL = 0;           
    SysTick->CTRL = 5;         

    while (ms--)
    {
        while (!(SysTick->CTRL & (1 << 16))) {}  
    }

    SysTick->CTRL = 0;  
}

void LCD_Enable(void) {
    GPIO_SetBits(LCD_PORT, LCD_EN);
    delay_ms(1);  
    GPIO_ResetBits(LCD_PORT, LCD_EN);
    delay_ms(1);
}

void LCD_Send4Bit(uint8_t data) {
    GPIO_WriteBit(LCD_PORT, LCD_D5, (data >> 0) & 1);
    GPIO_WriteBit(LCD_PORT, LCD_D6, (data >> 1) & 1);
    GPIO_WriteBit(LCD_PORT, LCD_D7, (data >> 2) & 1);
    GPIO_WriteBit(LCD_PORT, LCD_D8, (data >> 3) & 1);
    LCD_Enable();
}

void LCD_SendCommand(uint8_t cmd) {
    GPIO_ResetBits(LCD_PORT, LCD_RS);  // RS = 0 (Command Mode)
    LCD_Send4Bit(cmd >> 4);  
    LCD_Send4Bit(cmd);       
    delay_ms(5);
}


void LCD_SendData(char data) {
    GPIO_SetBits(LCD_PORT, LCD_RS);  // RS = 1 (Data Mode)
    LCD_Send4Bit(data >> 4);
    LCD_Send4Bit(data);
    delay_ms(5);
}

void Init(void) {
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
		GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_Pin = LCD_RS | LCD_EN | LCD_D5 | LCD_D6 | LCD_D7 | LCD_D8| LED_GREEN| LED_RED | GPIO_Pin_1;
	  GPIO_Init(GPIOB, &GPIO_InitStruct);
	
    delay_ms(50);  

    
    LCD_SendCommand(0x02);  //chuyen sang che do 4 bit
    LCD_SendCommand(0x28);  // chon che do hien thi 2 dong 
    LCD_SendCommand(0x0C);  // bat tat con tro
    LCD_SendCommand(0x06);  // dich con tro sang phai khi ta nhap du lieu 
    LCD_SendCommand(0x01);  // xoa man hinh 
    delay_ms(5);
		
		
	//ADC
}

void ADC1_Init(){
	// Initialization struct
	ADC_InitTypeDef ADC_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	// Step 1: Initialize ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_NbrOfChannel = 1;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &ADC_InitStruct);
	ADC_Cmd(ADC1, ENABLE);
	// Select input channel for ADC1
	// ADC1 channel 0 (PA0)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
	
	// Step 2: Initialize GPIOA (PA0)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void LCD_Print(char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}


uint16_t Read_ADC(void)
{
	
    // Start ADC conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// Wait until ADC conversion finished
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

	return ADC_GetConversionValue(ADC1);

}

/*
void TurnOnOff(){
	
	  GPIO_SetBits(GPIOB, LED_RED);  
    GPIO_ResetBits(GPIOB, LED_GREEN);
    delay_ms(500);
	
	  GPIO_SetBits(GPIOB, LED_GREEN);
		GPIO_ResetBits(GPIOB, LED_RED);
		delay_ms(500);
		
	
}*/


void Control_Led(float ppm){
    if( ppm > LEVEL_PPM)
    {
        GPIO_SetBits(GPIOB, LED_RED);
        GPIO_ResetBits(GPIOB, LED_GREEN);
    }
    else {
        GPIO_SetBits(GPIOB, LED_GREEN);
        GPIO_ResetBits(GPIOB, LED_RED);
    }
}

void On_Off_Buzzer(){
	if(ppm > LEVEL_PPM){
	GPIO_SetBits(LCD_PORT, GPIO_Pin_1);
	}
	else {
	GPIO_ResetBits(LCD_PORT, GPIO_Pin_1);
	}
}


float ADC_To_PPM(uint16_t sensorValue)
{
    float ppm_temp = (float)sensorValue / 4095 * 5000;
    return (uint16_t)ppm_temp;
}


void Print_PPM_LCD()
{   
		char line1[16] = "Gia tri PPM la: ";
		char line2[16];
    sprintf(line2, " %u PPM", ppm);
    
    LCD_SendCommand(0x01);
    delay_ms(5);
	
		LCD_SendCommand(0x80);
		LCD_Print(line1);
    
    LCD_SendCommand(0xC0);
    LCD_Print(line2);
    
    Control_Led(ppm);
		On_Off_Buzzer();
		Send_Blynk();
}




void Flash_Unlock(void) {
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}

void Flash_Lock(void) {
    FLASH_Lock(); 
}

void Flash_ErasePage(uint32_t pageAddress) {
    FLASH_ErasePage(pageAddress); 
}

void Flash_Write(uint32_t address, uint32_t data) {
    FLASH_ProgramWord(address, data); 
}

uint32_t Flash_Read(uint32_t address) {
    return *(volatile uint32_t*)address;
}

// Hàm m?u: luu m?t s? vào Flash
void Save_Value_To_Flash(uint32_t value) {
    Flash_Unlock();
    Flash_ErasePage(FLASH_SAVE_ADDR); // c?n xóa tru?c khi ghi
    Flash_Write(FLASH_SAVE_ADDR, value);
    Flash_Lock();
}

// Hàm m?u: d?c giá tr? dã luu
uint32_t Read_Value_From_Flash(void) {
    return Flash_Read(FLASH_SAVE_ADDR);
}

void UART_Configure(){ //UART1 PA2, PA3
	
USART_InitTypeDef uart;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  
    uart.USART_BaudRate = 9600;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_WordLength = USART_WordLength_8b;  
    USART_Init(USART2, &uart);
    USART_Cmd(USART2, ENABLE);
}

void UART_Pin_Config(){
	GPIO_InitTypeDef gpio;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// cau hinh chan TX PA2;
	gpio.GPIO_Mode=GPIO_Mode_AF_PP;
	gpio.GPIO_Pin=GPIO_Pin_2;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	// cau hinh chan RX Pa3;
	gpio.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin=GPIO_Pin_3;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
}

// ham truyen ky tu 
void uart_SendChar(char _chr){
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
	USART_SendData(USART2, _chr);
}

// ham truyen chuoi ky tu 
void uart_SendStr(char *str){
		while(*str ){
			uart_SendChar(*str++);	
	}
}

void Send_Blynk(){
	if(ppm > LEVEL_PPM){
		uart_SendStr("FIRE\n");
	}
}

int main()
{
    Init();
		ADC1_Init();
	  UART_Configure();
		UART_Pin_Config();
		
		int count = 0;
		ppm = Read_Value_From_Flash();
		Print_PPM_LCD();
	
		for( int i = 0; i < 5; i++){
		delay_ms(1000*1);
		}
    while(1){
				uint16_t sensor_value = Read_ADC();
				ppm = ADC_To_PPM(sensor_value);
			
        Print_PPM_LCD();
	
        delay_ms(1000);
				count++;
			
				if(count % 5 == 0){
					Save_Value_To_Flash( (uint32_t)ppm );
				}
   }
}







