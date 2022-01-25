#ifndef RingBuffer_h
#define RingBuffer_h

class RingBuffer
{
  public:
    RingBuffer();
    bool add(uint8_t b);
    bool get(uint8_t *b);
    bool is_empty();
    void commit();
  private:
    static const int RB_BUFF_MAX = 5000;
    uint8_t rb[RB_BUFF_MAX];
    int st, en, len, t_len;
 };

#endif
