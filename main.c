//EREOOOOTOOOOOOOO

#include "main.h"
#include "stdlib.h"

#define MAX_PAYLOAD 65536

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t TimingDelay = 0;
GPIO_InitTypeDef GPIO_InitStructure;
int data_arrived = 0;
int nibble_high  = 0;
int nibble_low   = 0;


/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);
void initPacket2(uint8_t *header);

uint16_t readPayloadLength(uint8_t firstLen, uint8_t secondLen);
uint8_t * initPacket(uint8_t *header); 

  
int crcCounter;

uint8_t *intBuff, *packet, pacchetto[3000] ;
uint16_t pLen, tmpF, tmpS;
int readProgress = 0;
int flagHeader = 1;



  int main(void)
{

  intBuff = (uint8_t *)malloc(6);
         
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
      //se ho letto l'header
      if(!flagHeader)
      {
        //payload....
        
        if(pLen != 0)
        {
           pacchetto[readProgress] = USART_ReceiveData(USART2);
           USART_SendData(USART2,pacchetto[readProgress]);
           readProgress++;
           pLen--;
        }
        else 
        {
          if(crcCounter != 2)
          {
              pacchetto[readProgress + crcCounter] = USART_ReceiveData(USART2);
              USART_SendData(USART2,(char) pacchetto[readProgress + crcCounter]);
              crcCounter++;
              if(crcCounter == 2)
              {
                readProgress = 0;
                crcCounter = 0;
                flagHeader = 1;
              }
          }  
        }
          
      }
      
      else
      {
        //leggo l'header
      
          switch(readProgress)
          {
            case 0:
            {
                  if(USART_ReceiveData(USART2) == 0x2E)
                  {
                    *(intBuff+readProgress) = USART_ReceiveData(USART2);
                    USART_SendData(USART2,(char) *(intBuff+readProgress));
                    readProgress++;
                  }
                    break;
            }
            case 1: //CONF (8bit)
            {
                  *(intBuff+readProgress) = USART_ReceiveData(USART2);
                  USART_SendData(USART2,(char) *(intBuff+readProgress));
                  readProgress++;
                  break;
            }
            
            case 2: // CODE (prima parte 8 bit)
            {
                  *(intBuff+readProgress) = USART_ReceiveData(USART2);
                  USART_SendData(USART2,(char) *(intBuff+readProgress));
                  readProgress++;
                  break;
            }
            
            case 3: // CODE (seconda parte 8 bit)
            {
                  *(intBuff+readProgress) = USART_ReceiveData(USART2);
                  USART_SendData(USART2,(char) *(intBuff+readProgress));
                  readProgress++;
                  break;
            }
            
            case 4: // CODE (prima parte 8 bit)
            {
                  *(intBuff+readProgress) = USART_ReceiveData(USART2);
                  USART_SendData(USART2,(char) *(intBuff+readProgress));
                  readProgress++; // vale 5
                  break;
            }
            
            case 5: // PLEN (seconda parte 8 bit)
            {
                  *(intBuff+readProgress) = USART_ReceiveData(USART2);
                  USART_SendData(USART2,(char) *(intBuff+readProgress));
                  pLen = readPayloadLength(*(intBuff+4), *(intBuff+5));
                  //packet = initPacket(intBuff);
                  initPacket2(intBuff);
                  readProgress++; //vale 6
                  flagHeader = 0;
                  
                  
                  break;
            }
       
                
          }
          
        }
    }
   
} 

uint16_t readPayloadLength(uint8_t firstLen, uint8_t secondLen)
{
  tmpF = (uint16_t) firstLen;
  tmpF <<= 8;
  tmpS = (uint16_t) secondLen;
  
  return tmpF | tmpS;
  
}

uint8_t * initPacket(uint8_t *header)
{
    uint8_t *myPacket;
    myPacket = (uint8_t *) malloc(pLen + 8);
    
    for(int i = 0; i < 6; i++)
      *(myPacket + i) = *(header + i);
    
    
    return myPacket;
    
}

void initPacket2(uint8_t *header)
{
  //memset(pacchetto,0,8 + pLen);
  for(int i = 0; i < 8; i++)
    pacchetto[i] = *(header + i);
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
