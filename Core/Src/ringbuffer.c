#include "ringbuffer.h"

/**
 * @brief           ��ȡ ringbuffer ״̬
 * 
 * @param rb        ����������
 * 
 * @retval           ringbuffer ö��
 *                      ��  :   RINGBUFFER_EMPTY
 *                      ��  :   RINGBUFFER_FULL
 *                      ����:   RINGBUFFER_HALFFULL
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
 * @brief           ��ȡ�����������ݴ�С
 * 
 * @param rb        ����������
 * 
 * @retval          ����������������
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
 * @brief           ��ʼ�� ringbuffer ʵ��
 * 
 * @param rb        ����������
 * @param pool      �����������׵�ַ
 * @param size      �����������С
 * 
 * 
 * @date 2021-05-16
 */
void ringbuffer_init(struct ringbuffer *rb,
                     uint8_t *pool,
                     int16_t size)
{

    /* ��ʼ����д���� */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* ���û������׵�ַ �� ��С*/
    rb->buffer_ptr = pool;
    rb->buffer_size = ALIGN_DOWN(size, ALIGN_SIZE);
}

/**
 * @brief           ���������� 1 �ֽ�
 * 
 * @param rb        ����������
 * @param ch        ���뻺����������
 * 
 * @retval          ִ�н��
 *                      0:  error
 *                      1:  succee
 * @date 2021-05-16
 */
uint32_t ringbuffer_putchar(struct ringbuffer *rb, const uint8_t ch)
{
    /* �������� */
    if (!ringbuffer_space_len(rb))
        return 0;

    rb->buffer_ptr[rb->write_index] = ch;

    /* ��ת��� */
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
 * @brief           �ӻ������ж�ȡ 1 �ֽ�
 * 
 * @param rb        ������ʵ��
 * @param ch        ��ȡ���ֽ�
 * 
 * @retval          ִ�н��:
 *                      0:  error
 *                      1:  succee
 * 
 * @date 2021-05-16
 */
uint32_t ringbuffer_getchar(struct ringbuffer *rb, uint8_t *ch)
{
    /* ������Ϊ NULL */
    if (!ringbuffer_data_len(rb))
        return 0;

    /*�����ֽ� */
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
