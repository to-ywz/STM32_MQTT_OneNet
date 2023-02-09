#include "ringbuffer.h"

/**
 * @brief           获取 ringbuffer 状态
 * 
 * @param rb        缓冲区对象
 * 
 * @retval           ringbuffer 枚举
 *                      空  :   RINGBUFFER_EMPTY
 *                      满  :   RINGBUFFER_FULL
 *                      半满:   RINGBUFFER_HALFFULL
 * @date 2021-05-16
 */
inline enum ringbuffer_state ringbuffer_status(struct ringbuffer *rb)
{
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
        {
            return RINGBUFFER_EMPTY;
        }
        else
        {
            return RINGBUFFER_FULL;
        }
    }
    return RINGBUFFER_HALFFULL;
}

/**
 * @brief           获取缓冲区中数据大小
 * 
 * @param rb        缓冲区对象
 * 
 * @retval          缓冲区中现有数据
 * 
 * @date 2021-05-16
 */
uint32_t ringbuffer_data_len(struct ringbuffer *rb)
{
    switch (ringbuffer_status(rb))
    {
    case RINGBUFFER_EMPTY:
        return 0;
    case RINGBUFFER_FULL:
        return rb->buffer_size;
    case RINGBUFFER_HALFFULL:
    default:
        if (rb->write_index > rb->read_index)
        {
            return rb->write_index - rb->read_index;
        }
        else
        {
            return rb->buffer_size - (rb->read_index - rb->write_index);
        }
    };
}

/**
 * @brief           初始化 ringbuffer 实例
 * 
 * @param rb        缓冲区对象
 * @param pool      缓冲区数组首地址
 * @param size      缓冲区数组大小
 * 
 * 
 * @date 2021-05-16
 */
void ringbuffer_init(struct ringbuffer *rb,
                     uint8_t *pool,
                     int16_t size)
{

    /* 初始化读写索引 */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* 配置缓冲区首地址 和 大小*/
    rb->buffer_ptr = pool;
    rb->buffer_size = ALIGN_DOWN(size, ALIGN_SIZE);
}

/**
 * @brief           缓冲区存入 1 字节
 * 
 * @param rb        缓冲区对象
 * @param ch        存入缓冲区的数据
 * 
 * @retval          执行结果
 *                      0:  error
 *                      1:  succee
 * @date 2021-05-16
 */
uint32_t ringbuffer_putchar(struct ringbuffer *rb, const uint8_t ch)
{
    /* 缓冲区满 */
    if (!ringbuffer_space_len(rb))
        return 0;

    rb->buffer_ptr[rb->write_index] = ch;

    /* 翻转检测 */
    if (rb->write_index == rb->buffer_size - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * @brief           从缓冲区中读取 1 字节
 * 
 * @param rb        缓冲区实体
 * @param ch        读取的字节
 * 
 * @retval          执行结果:
 *                      0:  error
 *                      1:  succee
 * 
 * @date 2021-05-16
 */
uint32_t ringbuffer_getchar(struct ringbuffer *rb, uint8_t *ch)
{
    /* 缓存区为 NULL */
    if (!ringbuffer_data_len(rb))
        return 0;

    /*存入字节 */
    *ch = rb->buffer_ptr[rb->read_index];

    if (rb->read_index == rb->buffer_size - 1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}
