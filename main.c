//Branch senza case

#include "main.h"
#include "stdlib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_DIM 300
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t TimingDelay = 0;
GPIO_InitTypeDef GPIO_InitStructure;

/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);

uint16_t readPayloadLength(uint8_t firstLen, uint8_t secondLen);

uint8_t buffer[MAX_DIM], pacchetto[MAX_DIM];
uint16_t pLen;
int readProgress = 0;

  int main(void)
{

  //intBuff = (uint8_t *)malloc(6);
         
  /* Setup SysTick Timer for 1 µsec interrupts  */
  if (SysTick_Config(SystemCoreClock / 1000000))
  { 
    /* Capture error */ 
    while (1)
    {}
  }

  //USART1
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);    
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); 
    
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

  /* conf */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //USART configuration
  USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_HardwareFlowControl = 
  USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    
  USART_Init(USART2, &USART_InitStructure);
  
  // enable interrrupt 
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

  //Enable USART Interrrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  
  
  
  USART_Cmd(USART2, ENABLE);

  /* Infinite loop */
  while (1)
  {
    
  
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        //Attendo un nuovo byte di START
        if (readProgress == 0)
        {
            if(USART_ReceiveData(USART2) == 0x2E)
            {
                buffer[readProgress] = USART_ReceiveData(USART2);
                USART_SendData(USART2,buffer[readProgress]);
                readProgress++;
            }
            //Altrimenti non fa niente
        }
        //Sto assemblando l'header
        else if (readProgress < 6)
        {
            buffer[readProgress] = USART_ReceiveData(USART2);
            USART_SendData(USART2,buffer[readProgress]);
            readProgress++;
            if(readProgress == 6)
            {
                pLen = readPayloadLength(buffer[4], buffer[5]);
                //Se il payload è troppo grande lo considera sbagliato e scarta il pacchetto
                if (pLen > MAX_DIM - 8)
                {
                    readProgress = 0;
                    memset(buffer, 0, MAX_DIM);
                }
            }
        }
        //Sto assemblando il Payload
        else if (readProgress < 6 + pLen)
        {
            buffer[readProgress] = USART_ReceiveData(USART2);
            USART_SendData(USART2,buffer[readProgress]);
            readProgress++;
        }
        //Sto assemblando il CRC
        else if (readProgress < MAX_DIM)
        {
            buffer[readProgress] = USART_ReceiveData(USART2);
            USART_SendData(USART2,buffer[readProgress]);
            readProgress++;
            if(readProgress == 8 + pLen)
            {
                readProgress = 0;
                //ElaboraPacchetto()
                //memset(buffer, 0, MAX_DIM);
            }
        }
    }
   
} 

uint16_t readPayloadLength(uint8_t firstLen, uint8_t secondLen)
{
  uint16_t tmpF, tmpS;
  
  tmpF = (uint16_t) firstLen;
  tmpF <<= 8;
  tmpS = (uint16_t) secondLen;
  
  return tmpF | tmpS;
  
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 
