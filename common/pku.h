#ifndef __PKU_H__
#define __PKU_H__
void Delay(int k);
void SYSCLK_Init();
void EMIF_Low(void);
void Timer4_Init (int counts);
void Timer3_Init (int counts);
void ADC0_Init (int mux);
void UART0_Init ();

#endif
