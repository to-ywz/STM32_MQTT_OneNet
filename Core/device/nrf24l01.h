#ifndef HW_NRF24L01_H_
#define HW_NRF24L01_H_

#define NRF_CE_PIN "PB6"
#define NRF_CS_PIN "PB7"
#define NRF_IQR_PIN "PB8"

void nrf24l01_init(void);
void nrf24l01_data_recv(void);
void nrf24l01_data_xmit(float data);

#endif
