/**
 * @file ringbuffer.h
 * @author BlackSheep (ywz_123xxx@163.com)
 * @brief   环形缓冲区
 * @version 0.1
 * @date 2021-05-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "stdint.h"

#define ALIGN_SIZE 4 // 字节对齐
// * 返回对齐数
#define ALIGN_DOWN(size, align) ((size) & ~((align)-1))

#define ringbuffer_space_len(rb) ((rb)->buffer_size - ringbuffer_data_len(rb))

typedef struct ringbuffer
{
    uint8_t *buffer_ptr;

    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;

    int16_t buffer_size;
} ringbuffer_t;

enum ringbuffer_state
{
    RINGBUFFER_EMPTY,
    RINGBUFFER_FULL,

    RINGBUFFER_HALFFULL,
};

void ringbuffer_init(struct ringbuffer *rb, uint8_t *pool, int16_t size);

uint32_t ringbuffer_data_len(struct ringbuffer *rb);
enum ringbuffer_state ringbuffer_status(struct ringbuffer *rb);
uint32_t ringbuffer_getchar(struct ringbuffer *rb, uint8_t *ch);
uint32_t ringbuffer_putchar(struct ringbuffer *rb, const uint8_t ch);

#endif
