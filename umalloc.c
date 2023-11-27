#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep; 
}

void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}

/**
 * TODO: ssualloc2 를 syscall 화 하고, 아래 코드를 전부 커널영역으로 옮기기. 
 * 그래야 거기서 free 호출 가능, hp->s.size = nu 와 free연산이 pgfault를 일으키기에 이를
 * 그냥 반환하고, pgfault가 일어나면 hp->s.size등의 연산 ㄱ
*/

// static Header*
// morecoressu(uint nu) 
// {
//   char *p;
//   Header *hp;

//   // if(nu % 4096 != 0)
//   //   return 0;
//   p = ssualloc(nu * sizeof(Header));
//   // printf(1, "SHHS");
//   if(p == (char*)-1)
//     return 0;
//   // printf(1, "tt");
//   hp = (Header*)p;
//   // hp->s.size = nu;
//   // free((void*)(hp + 1));
//   freep = p;
//   return freep;
// }
// /**
//  * TODO: kallo형식으로 바꿔야함
// */
// int ssualloc2(uint nbytes) { 
//   Header *p, *prevp;
//   uint nunits;
//   if (nbytes % 4096 != 0) return -1;
//   nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header);
//   if((prevp = freep) == 0){
//     base.s.ptr = freep = prevp = &base;
//     base.s.size = 0;
//   }
//   for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
//     if(p->s.size >= nunits){
//       if(p->s.size == nunits)
//         prevp->s.ptr = p->s.ptr;
//       else {
//         p->s.size -= nunits;
//         p += p->s.size;
//         p->s.size = nunits;
//       }
//       freep = prevp;
//       // return (void*)(p + 1);
//       return (p+1);
//     }
//     if(p == freep) {
//       printf(1, "called!\n");
//       if((p = morecoressu(nunits)) == 0) 
//         return -1;
      
//     }
//   }
// }

// // int ssualloc2(uint nbytes) {

// // }