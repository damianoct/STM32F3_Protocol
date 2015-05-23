//Branch senza case

#include "main.h"
#include "stdlib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_DIM 300
#define MAX_HEADER_INDEX 4

//Campi del pacchetto
#define START 0
#define CONF 1
#define CODE 2
#define PLEN_1 3
#define PLEN_2 4

//Codici dei comandi
#define ECHO 0x00

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t TimingDelay = 0;
GPIO_InitTypeDef GPIO_InitStructure;

/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);
uint16_t readPayloadLength(uint8_t firstLen, uint8_t secondLen);
uint8_t isBitSet(uint8_t buffer, int index);

void BAU_PacketClone(uint8_t buffer[]);
void BAU_PacketDispatch(uint8_t pack[]);
void BAU_Echo(uint8_t pack[]);

uint8_t buffer[MAX_DIM], packet[MAX_DIM];
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
                //USART_SendData(USART2,buffer[readProgress]);
                readProgress++;
            }
            //Altrimenti non fa niente
        }
        //Sto assemblando l'header
        else if (readProgress < MAX_HEADER_INDEX + 1) //Fine dell'HEADER all'indice MAX_HEADER
        {
            buffer[readProgress] = USART_ReceiveData(USART2);
            //USART_SendData(USART2,buffer[readProgress]);
            readProgress++;
            if(readProgress == MAX_HEADER_INDEX + 1) //Se abbiamo finito di assemblare l'Header
            {
                pLen = readPayloadLength(buffer[PLEN_1], buffer[PLEN_2]);
                //Se il payload è troppo grande lo considera sbagliato e scarta il pacchetto
                if (pLen > MAX_DIM - 7)
                {
                    readProgress = 0;
                    memset(buffer, 0, MAX_DIM);
                }
            }
        }
        //Sto assemblando il Payload
        else if (readProgress < MAX_HEADER_INDEX + 1 + pLen)
        {
            buffer[readProgress] = USART_ReceiveData(USART2);
            //USART_SendData(USART2,buffer[readProgress]);
            readProgress++;
        }
        //Sto assemblando il CRC
        else if (readProgress < MAX_DIM)
        {
            buffer[readProgress] = USART_ReceiveData(USART2);
            //USART_SendData(USART2,buffer[readProgress]);
            readProgress++;
            if(readProgress == 7 + pLen)
            {
                readProgress = 0;
                BAU_PacketClone(buffer);
                BAU_PacketDispatch(packet);
                memset(buffer, 0, MAX_DIM);
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

uint8_t isBitSet(uint8_t buffer, int index)
{
    uint8_t mask = (1 << index);
    return buffer & mask;
}

//Ritorna void perchè uso packet che è globale; Non servirebbe nemmeno il parametro.
void BAU_PacketClone(uint8_t buffer[])
{
    int i, len = 0;
    memset(packet, 0, MAX_DIM);
    len = readPayloadLength(buffer[PLEN_1], buffer[PLEN_2]);
    //Esiste in alternativa la funzione memcpy
    for(i=0; i < len + 7; i++)
    {
        packet[i] = buffer[i];
    }    
}

void BAU_PacketSend(uint8_t pack[])
{
    int i=0, len = 0;
    len = readPayloadLength(pack[PLEN_1], pack[PLEN_2]);
    for(i=0; i < len + 7; i++)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
            USART_SendData(USART2, pack[i]);
    }
}

void BAU_Echo(uint8_t pack[])
{
    BAU_PacketSend(pack);
}

void BAU_PacketDispatch(uint8_t pack[])
{
    //Controllo l'integrità del pacchetto
    if(isBitSet(pack[1], 4)) //Non ricordo un altro metodo per farlo
    {
        //Qui va il codice per controllare se il CRC corrisponde a quello
        //calcolato. Se non corrisponde si scarta il pacchetto con un return.
    }
    
    if(!isBitSet(pack[1], 6)) //Controllo che il pacchetto sia solo di request.
            return;
    
    //Cambiarlo con uno switch case sul code sarebbe meglio?
    if(pack[CODE] == ECHO)
        BAU_Echo(pack);
        
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
