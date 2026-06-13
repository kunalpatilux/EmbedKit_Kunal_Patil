/////////////////////////////////////////////////////////////////////////
//  ->ringbuf.c --Embed Kit: Ring Buffer Module
//  ->Implements a circuler (ring) buffer for unit8_t data with a fixed
//  ->capacity of BUFFER_SIZE bytes
//////////////////////////////////////////////////////////////////////////


#include <stdio.h> //For Printf & Scanf
#include<stdint.h> //For fixed-width types uint8_t & ubt8_t
#include<string.h> //For memset() to clear the buffer
#include<stdlib.h> // Standard library

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//               Configuration
//  ->BUFFER_SIZE must be a power of 2(1,2,4,8,16,....).
//  ->This allows the the fast bitwies-AND wrap trick:
//  ->index & (BUFFER_SIZE-1)
//  ->instead of the slower modulo:
//  ->index % BUFFER_SIZE
//  ->The '%' operater comiles to a hardware division instruction on most MCU.
//    Many low-end microcontrollers (Ex AVR , Cortex-M0) have No hardware divider
//    so the toolchain inserts a software division routine that can take 20-100+ CPU cycles
//    A bitwise AND always takes exactly 1 cycle . In a UART ISR that fires thousands of times per second
//    this difference matters significantly.
//  ->Why only power-of-2? When BUFFER_SIZE = 2^N , (buffer_size-1)is a bitmask of N ones (EX 8-1 = 0B00000111).ANDing any index with this 
//    mask keeps only the lower N bits , which is identical to index % 8 for 
//    non-negative integers.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE 8U    //Must be a power of 2
#define BUFFER_MASK (BUFFER_SIZE -1u) //Wrap mask:0x07
#define RB_OK  0
#define RB_ERR_FULL -1
#define RB_ERR_EMPTY -2
//////////////////////////////////////
//   Data Structure
//////////////////////////////////////
typedef struct
{ 
    uint8_t buf[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;

}RingBuf;

///////////////////////////////////////////////////////////////
// API Implementation
// ->ringbuf_init -- Initialise the buffer to an empty state
///////////////////////////////////////////////////////////////
void ringbuf_init(RingBuf *rb)
{
    memset(rb->buf , 0 , sizeof(rb->buf));
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}
//ringbuf_is_empty -- Return 1 if buffer has no data, 0 otherwise.
uint8_t ringbuf_is_empty(const RingBuf *rb)
{
    return (rb->count ==0u)? 1u : 0u;
}
//ringbuf_is_full -- Return 1 if buffer is at capacity, 0 otherwise.
uint8_t ringbuf_is_full(const RingBuf *rb)
{
    return (rb->count == BUFFER_SIZE)? 1U : 0u;
}
//ringbuf_count -- Return the number of bytes currently in the buffer.
uint8_t ringbuf_count(const RingBuf *rb)
{
    return rb->count;
}
/////////////////////////////////////////////////////////////////////////
//->ringbuf_write --Write one byte into the buffer.
//->Returns RB_OK on success.
//->Returns RB_ERR_FULL IF THE BUFFER is full (byte is NOT written)
//////////////////////////////////////////////////////////////////////////
int8_t ringbuf_write(RingBuf *rb , uint8_t byte)
{
    if(ringbuf_is_full(rb))
    {
        return RB_ERR_FULL;
    }
    rb->buf[rb->head] = byte;

    /////////////////////////////////////////////////////////////////////
    //->Advance head with bitwise-AND wrap (see BUFFER_MASK note above).
    //->Equivalent to: rb->head = (rb->head + 1) % BUFFER_SIZE;
    ///////////////////////////////////////////////////////////////////////
    rb->head = (rb->head+1u) & BUFFER_MASK;
    rb->count++;

    return RB_OK;
}
/////////////////////////////////////////////////////////////////////////////
//->ingbuf_read -- Read one byte from the buffer into *out_byte.
//->Returns RB_OK on success.
//->Returns RB_ERR_EMPTY if the buffer is empty (*out_byte is NOT modified).
/////////////////////////////////////////////////////////////////////////////
int8_t ringbuf_read(RingBuf *rb , uint8_t *out_byte)
{
    if(ringbuf_is_empty(rb))
    {
        return RB_ERR_EMPTY;
    }
    *out_byte = rb->buf[rb->tail];
    ////////////////////////////////////////////////////////////////////
    //->Advance tail with bitwise-AND wrap (see BUFFER_MASK note above).
    //->Equivalent to: rb->tail = (rb->tail + 1) % BUFFER_SIZE;
    ////////////////////////////////////////////////////////////////////

    rb->tail = (rb->tail+1u)& BUFFER_MASK;
    rb->count--;

    return RB_OK;
}

/////////////////////////////
// Demp --main()
/////////////////////////////

int main()
{
    RingBuf rb;
    uint8_t byte_out;
    int8_t result;

    ringbuf_init(&rb);
    printf("== Embed Kit: Ring Buffer Demo ==\n\n");
    ////////////////////////////////////////
    //Step 1: Write 8 bytes (0x41 .. 0x48)
    ////////////////////////////////////////
    printf("Step 1: Write 8 byte \n");
    uint8_t fill_bytes[BUFFER_SIZE] ={
        0x41,0x42, 0x43, 0x44,
        0x45, 0x46, 0x47, 0x48 
    };
    for(uint8_t i = 0; i<BUFFER_SIZE; i++)
    {
        result = ringbuf_write(&rb, fill_bytes[i]);
        if(result == RB_OK)
        {
            printf("[WRITE]0X%02x -> OK (count=%u)%s\n",
                   fill_bytes[i],
                   ringbuf_count(&rb),
                   ringbuf_is_full(&rb)? "FULL" : "");
        } else {
            printf("[WRITE]0x%02x -> FAIL (buffer full)\n",fill_bytes[i]);
        }
    }
    printf("\n");
    ///////////////////////////////////////////////////////////
    //Step 2:Attempt to write one more byte into a full buffer
    ////////////////////////////////////////////////////////////
    printf("Step 2: Write to full buffer \n");
    result = ringbuf_write(&rb , 0x99);
    if(result == RB_ERR_FULL)
    {
        printf(("[WRITE]0x99-> FAIL (buffer full) \n"));
    }
    printf("\n");

     ///////////////////////////////////////////////////////////
    //Step 3:Read 3 bytes (expect 0x41, 0x42, 0x43)
    ////////////////////////////////////////////////////////////
     printf(" Step 3: Read 3 bytes \n");

     for(uint8_t i = 0; i<3u; i++)
     {
        result = ringbuf_read(&rb, &byte_out);
        if(result == RB_OK)
        {
            printf("[READ] -> 0x%02x(count=%u)\n",
                byte_out, ringbuf_count(&rb));
        }
     }
    printf("Count after 3 reads: %u (expected 5)\n", ringbuf_count(&rb));
    printf("\n");
    
    
     ///////////////////////////////////////////////////////////
    //Step 4:Write 3 new bytes reusing freed slots 
    ////////////////////////////////////////////////////////////
    
    uint8_t new_bytes[3] = {0x49, 0x4A, 0x4B};
 
    for (uint8_t i = 0; i < 3u; i++) {
        result = ringbuf_write(&rb, new_bytes[i]);
        if (result == RB_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   new_bytes[i],
                   ringbuf_count(&rb),
                   ringbuf_is_full(&rb) ? " FULL" : "");
        }
    }
 
    printf("\n");
    ///////////////////////////////////////////////////////////
    //Step 5:Read all 8 remaining bytes
    ////////////////////////////////////////////////////////////
    printf(" Step 5: Read all 8 remaining bytes \n");
 
    while (!ringbuf_is_empty(&rb)) {
        result = ringbuf_read(&rb, &byte_out);
        if (result == RB_OK) {
            printf("[READ]  -> 0x%02X (count=%u)%s\n",
                   byte_out,
                   ringbuf_count(&rb),
                   ringbuf_is_empty(&rb) ? " EMPTY" : "");
        }
    }
 
    printf("\n");
   
     ///////////////////////////////////////////////////////////
    //Step 6:Read all 8 remaining bytes
    ////////////////////////////////////////////////////////////
    printf(" Step 6: Read from empty buffer \n");
 
    result = ringbuf_read(&rb, &byte_out);
    if (result == RB_ERR_EMPTY) {
        printf("[READ]  (empty) -> FAIL (buffer empty)\n");
    }
 
    printf("\n=== Demo Complete ===\n");
 
    return 0;
}