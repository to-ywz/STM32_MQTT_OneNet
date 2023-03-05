#ifndef HW_NRF24L01_H_
#define HW_NRF24L01_H_

#define NRF_CE_PIN      "PB6"
#define NRF_CS_PIN      "PB7"
#define NRF_IQR_PIN     "PB8"

void nrf24l01_init(void);
void NRF24L01DataExchange(void);

#endif

