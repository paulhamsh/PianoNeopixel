#include "RingBuffer.h" 

RingBuffer::RingBuffer() {
  st = 0;
  en = 0;
  len = 0;
  t_len = 0;
}

bool RingBuffer::add(uint8_t b) {
  if (len + t_len < RB_BUFF_MAX) {
    rb[en] = b;
    t_len++;
    en++; 
    if (en >= RB_BUFF_MAX) en = 0; 
    return true;
  }
  else
    return false;
}

bool RingBuffer::get(uint8_t *b) {
  if (len > 0) {
    *b = rb[st];
    len--;
    st++; 
    if (st >= RB_BUFF_MAX) st = 0; 
    return true;
  }
  else
    return false;  
}

void RingBuffer::commit() {
  len += t_len;
  t_len = 0;
}

bool RingBuffer::is_empty() {
  return (len == 0);
}
