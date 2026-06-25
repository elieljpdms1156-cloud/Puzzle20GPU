#include "ripemd160.h"
#include <string.h>
#define ROL(x,n)  (((x)<<(n))|((x)>>(32-(n))))
#define f1(x,y,z) ((x)^(y)^(z))
#define f2(x,y,z) (((x)&(y))|(~(x)&(z)))
#define f3(x,y,z) (((x)|(~(y)))^(z))
#define f4(x,y,z) (((x)&(z))|((y)&(~(z))))
#define f5(x,y,z) ((x)^((y)|(~(z))))
#define STEP(a,b,c,d,e,f,x,k,s) \
    (a)+=f((b),(c),(d))+(x)+(k); (a)=ROL((a),(s))+(e); (c)=ROL((c),10);
static void compress(uint32_t state[5],const uint8_t block[64]){
    uint32_t X[16];
    for(int i=0;i<16;i++) X[i]=(uint32_t)block[4*i]|((uint32_t)block[4*i+1]<<8)|((uint32_t)block[4*i+2]<<16)|((uint32_t)block[4*i+3]<<24);
    uint32_t al=state[0],bl=state[1],cl=state[2],dl=state[3],el=state[4];
    uint32_t ar=state[0],br=state[1],cr=state[2],dr=state[3],er=state[4];
#define f f1
    STEP(al,bl,cl,dl,el,f,X[ 0],0x00000000UL,11) STEP(el,al,bl,cl,dl,f,X[ 1],0x00000000UL,14)
    STEP(dl,el,al,bl,cl,f,X[ 2],0x00000000UL,15) STEP(cl,dl,el,al,bl,f,X[ 3],0x00000000UL,12)
    STEP(bl,cl,dl,el,al,f,X[ 4],0x00000000UL, 5) STEP(al,bl,cl,dl,el,f,X[ 5],0x00000000UL, 8)
    STEP(el,al,bl,cl,dl,f,X[ 6],0x00000000UL, 7) STEP(dl,el,al,bl,cl,f,X[ 7],0x00000000UL, 9)
    STEP(cl,dl,el,al,bl,f,X[ 8],0x00000000UL,11) STEP(bl,cl,dl,el,al,f,X[ 9],0x00000000UL,13)
    STEP(al,bl,cl,dl,el,f,X[10],0x00000000UL,14) STEP(el,al,bl,cl,dl,f,X[11],0x00000000UL,15)
    STEP(dl,el,al,bl,cl,f,X[12],0x00000000UL, 6) STEP(cl,dl,el,al,bl,f,X[13],0x00000000UL, 7)
    STEP(bl,cl,dl,el,al,f,X[14],0x00000000UL, 9) STEP(al,bl,cl,dl,el,f,X[15],0x00000000UL, 8)
#undef f
#define f f2
    STEP(el,al,bl,cl,dl,f,X[ 7],0x5A827999UL, 7) STEP(dl,el,al,bl,cl,f,X[ 4],0x5A827999UL, 6)
    STEP(cl,dl,el,al,bl,f,X[13],0x5A827999UL, 8) STEP(bl,cl,dl,el,al,f,X[ 1],0x5A827999UL,13)
    STEP(al,bl,cl,dl,el,f,X[10],0x5A827999UL,11) STEP(el,al,bl,cl,dl,f,X[ 6],0x5A827999UL, 9)
    STEP(dl,el,al,bl,cl,f,X[15],0x5A827999UL, 7) STEP(cl,dl,el,al,bl,f,X[ 3],0x5A827999UL,15)
    STEP(bl,cl,dl,el,al,f,X[12],0x5A827999UL, 7) STEP(al,bl,cl,dl,el,f,X[ 0],0x5A827999UL,12)
    STEP(el,al,bl,cl,dl,f,X[ 9],0x5A827999UL,15) STEP(dl,el,al,bl,cl,f,X[ 5],0x5A827999UL, 9)
    STEP(cl,dl,el,al,bl,f,X[ 2],0x5A827999UL,11) STEP(bl,cl,dl,el,al,f,X[14],0x5A827999UL, 7)
    STEP(al,bl,cl,dl,el,f,X[11],0x5A827999UL,13) STEP(el,al,bl,cl,dl,f,X[ 8],0x5A827999UL,12)
#undef f
#define f f3
    STEP(dl,el,al,bl,cl,f,X[ 3],0x6ED9EBA1UL,11) STEP(cl,dl,el,al,bl,f,X[10],0x6ED9EBA1UL,13)
    STEP(bl,cl,dl,el,al,f,X[14],0x6ED9EBA1UL, 6) STEP(al,bl,cl,dl,el,f,X[ 4],0x6ED9EBA1UL, 7)
    STEP(el,al,bl,cl,dl,f,X[ 9],0x6ED9EBA1UL,14) STEP(dl,el,al,bl,cl,f,X[15],0x6ED9EBA1UL, 9)
    STEP(cl,dl,el,al,bl,f,X[ 8],0x6ED9EBA1UL,13) STEP(bl,cl,dl,el,al,f,X[ 1],0x6ED9EBA1UL,15)
    STEP(al,bl,cl,dl,el,f,X[ 2],0x6ED9EBA1UL,14) STEP(el,al,bl,cl,dl,f,X[ 7],0x6ED9EBA1UL, 8)
    STEP(dl,el,al,bl,cl,f,X[ 0],0x6ED9EBA1UL,13) STEP(cl,dl,el,al,bl,f,X[ 6],0x6ED9EBA1UL, 6)
    STEP(bl,cl,dl,el,al,f,X[13],0x6ED9EBA1UL, 5) STEP(al,bl,cl,dl,el,f,X[11],0x6ED9EBA1UL,12)
    STEP(el,al,bl,cl,dl,f,X[ 5],0x6ED9EBA1UL, 7) STEP(dl,el,al,bl,cl,f,X[12],0x6ED9EBA1UL, 5)
#undef f
#define f f4
    STEP(cl,dl,el,al,bl,f,X[ 1],0x8F1BBCDCUL,11) STEP(bl,cl,dl,el,al,f,X[ 9],0x8F1BBCDCUL,12)
    STEP(al,bl,cl,dl,el,f,X[11],0x8F1BBCDCUL,14) STEP(el,al,bl,cl,dl,f,X[10],0x8F1BBCDCUL,15)
    STEP(dl,el,al,bl,cl,f,X[ 0],0x8F1BBCDCUL,14) STEP(cl,dl,el,al,bl,f,X[ 8],0x8F1BBCDCUL,15)
    STEP(bl,cl,dl,el,al,f,X[12],0x8F1BBCDCUL, 9) STEP(al,bl,cl,dl,el,f,X[ 4],0x8F1BBCDCUL, 8)
    STEP(el,al,bl,cl,dl,f,X[13],0x8F1BBCDCUL, 9) STEP(dl,el,al,bl,cl,f,X[ 3],0x8F1BBCDCUL,14)
    STEP(cl,dl,el,al,bl,f,X[ 7],0x8F1BBCDCUL, 5) STEP(bl,cl,dl,el,al,f,X[15],0x8F1BBCDCUL, 6)
    STEP(al,bl,cl,dl,el,f,X[14],0x8F1BBCDCUL, 8) STEP(el,al,bl,cl,dl,f,X[ 5],0x8F1BBCDCUL, 6)
    STEP(dl,el,al,bl,cl,f,X[ 6],0x8F1BBCDCUL, 5) STEP(cl,dl,el,al,bl,f,X[ 2],0x8F1BBCDCUL,12)
#undef f
#define f f5
    STEP(bl,cl,dl,el,al,f,X[ 4],0xA953FD4EUL, 9) STEP(al,bl,cl,dl,el,f,X[ 0],0xA953FD4EUL,15)
    STEP(el,al,bl,cl,dl,f,X[ 5],0xA953FD4EUL, 5) STEP(dl,el,al,bl,cl,f,X[ 9],0xA953FD4EUL,11)
    STEP(cl,dl,el,al,bl,f,X[ 7],0xA953FD4EUL, 6) STEP(bl,cl,dl,el,al,f,X[12],0xA953FD4EUL, 8)
    STEP(al,bl,cl,dl,el,f,X[ 2],0xA953FD4EUL,13) STEP(el,al,bl,cl,dl,f,X[10],0xA953FD4EUL,12)
    STEP(dl,el,al,bl,cl,f,X[14],0xA953FD4EUL, 5) STEP(cl,dl,el,al,bl,f,X[ 1],0xA953FD4EUL,12)
    STEP(bl,cl,dl,el,al,f,X[ 3],0xA953FD4EUL,13) STEP(al,bl,cl,dl,el,f,X[ 8],0xA953FD4EUL,14)
    STEP(el,al,bl,cl,dl,f,X[11],0xA953FD4EUL,11) STEP(dl,el,al,bl,cl,f,X[ 6],0xA953FD4EUL, 8)
    STEP(cl,dl,el,al,bl,f,X[15],0xA953FD4EUL, 5) STEP(bl,cl,dl,el,al,f,X[13],0xA953FD4EUL, 6)
#undef f
#define f f5
    STEP(ar,br,cr,dr,er,f,X[ 5],0x50A28BE6UL, 8) STEP(er,ar,br,cr,dr,f,X[14],0x50A28BE6UL, 9)
    STEP(dr,er,ar,br,cr,f,X[ 7],0x50A28BE6UL, 9) STEP(cr,dr,er,ar,br,f,X[ 0],0x50A28BE6UL,11)
    STEP(br,cr,dr,er,ar,f,X[ 9],0x50A28BE6UL,13) STEP(ar,br,cr,dr,er,f,X[ 2],0x50A28BE6UL,15)
    STEP(er,ar,br,cr,dr,f,X[11],0x50A28BE6UL,15) STEP(dr,er,ar,br,cr,f,X[ 4],0x50A28BE6UL, 5)
    STEP(cr,dr,er,ar,br,f,X[13],0x50A28BE6UL, 7) STEP(br,cr,dr,er,ar,f,X[ 6],0x50A28BE6UL, 7)
    STEP(ar,br,cr,dr,er,f,X[15],0x50A28BE6UL, 8) STEP(er,ar,br,cr,dr,f,X[ 8],0x50A28BE6UL,11)
    STEP(dr,er,ar,br,cr,f,X[ 1],0x50A28BE6UL,14) STEP(cr,dr,er,ar,br,f,X[10],0x50A28BE6UL,14)
    STEP(br,cr,dr,er,ar,f,X[ 3],0x50A28BE6UL,12) STEP(ar,br,cr,dr,er,f,X[12],0x50A28BE6UL, 6)
#undef f
#define f f4
    STEP(er,ar,br,cr,dr,f,X[ 6],0x5C4DD124UL, 9) STEP(dr,er,ar,br,cr,f,X[11],0x5C4DD124UL,13)
    STEP(cr,dr,er,ar,br,f,X[ 3],0x5C4DD124UL,15) STEP(br,cr,dr,er,ar,f,X[ 7],0x5C4DD124UL, 7)
    STEP(ar,br,cr,dr,er,f,X[ 0],0x5C4DD124UL,12) STEP(er,ar,br,cr,dr,f,X[13],0x5C4DD124UL, 8)
    STEP(dr,er,ar,br,cr,f,X[ 5],0x5C4DD124UL, 9) STEP(cr,dr,er,ar,br,f,X[10],0x5C4DD124UL,11)
    STEP(br,cr,dr,er,ar,f,X[14],0x5C4DD124UL, 7) STEP(ar,br,cr,dr,er,f,X[15],0x5C4DD124UL, 7)
    STEP(er,ar,br,cr,dr,f,X[ 8],0x5C4DD124UL,12) STEP(dr,er,ar,br,cr,f,X[12],0x5C4DD124UL, 7)
    STEP(cr,dr,er,ar,br,f,X[ 4],0x5C4DD124UL, 6) STEP(br,cr,dr,er,ar,f,X[ 9],0x5C4DD124UL,15)
    STEP(ar,br,cr,dr,er,f,X[ 1],0x5C4DD124UL,13) STEP(er,ar,br,cr,dr,f,X[ 2],0x5C4DD124UL,11)
#undef f
#define f f3
    STEP(dr,er,ar,br,cr,f,X[15],0x6D703EF3UL, 9) STEP(cr,dr,er,ar,br,f,X[ 5],0x6D703EF3UL, 7)
    STEP(br,cr,dr,er,ar,f,X[ 1],0x6D703EF3UL,15) STEP(ar,br,cr,dr,er,f,X[ 3],0x6D703EF3UL,11)
    STEP(er,ar,br,cr,dr,f,X[ 7],0x6D703EF3UL, 8) STEP(dr,er,ar,br,cr,f,X[14],0x6D703EF3UL, 6)
    STEP(cr,dr,er,ar,br,f,X[ 6],0x6D703EF3UL, 6) STEP(br,cr,dr,er,ar,f,X[ 9],0x6D703EF3UL,14)
    STEP(ar,br,cr,dr,er,f,X[11],0x6D703EF3UL,12) STEP(er,ar,br,cr,dr,f,X[ 8],0x6D703EF3UL,13)
    STEP(dr,er,ar,br,cr,f,X[12],0x6D703EF3UL, 5) STEP(cr,dr,er,ar,br,f,X[ 2],0x6D703EF3UL,14)
    STEP(br,cr,dr,er,ar,f,X[10],0x6D703EF3UL,13) STEP(ar,br,cr,dr,er,f,X[ 0],0x6D703EF3UL,13)
    STEP(er,ar,br,cr,dr,f,X[ 4],0x6D703EF3UL, 7) STEP(dr,er,ar,br,cr,f,X[13],0x6D703EF3UL, 5)
#undef f
#define f f2
    STEP(cr,dr,er,ar,br,f,X[ 8],0x7A6D76E9UL,15) STEP(br,cr,dr,er,ar,f,X[ 6],0x7A6D76E9UL, 5)
    STEP(ar,br,cr,dr,er,f,X[ 4],0x7A6D76E9UL, 8) STEP(er,ar,br,cr,dr,f,X[ 1],0x7A6D76E9UL,11)
    STEP(dr,er,ar,br,cr,f,X[ 3],0x7A6D76E9UL,14) STEP(cr,dr,er,ar,br,f,X[11],0x7A6D76E9UL,14)
    STEP(br,cr,dr,er,ar,f,X[15],0x7A6D76E9UL, 6) STEP(ar,br,cr,dr,er,f,X[ 0],0x7A6D76E9UL,14)
    STEP(er,ar,br,cr,dr,f,X[ 5],0x7A6D76E9UL, 6) STEP(dr,er,ar,br,cr,f,X[12],0x7A6D76E9UL, 9)
    STEP(cr,dr,er,ar,br,f,X[ 2],0x7A6D76E9UL,12) STEP(br,cr,dr,er,ar,f,X[13],0x7A6D76E9UL, 9)
    STEP(ar,br,cr,dr,er,f,X[ 9],0x7A6D76E9UL,12) STEP(er,ar,br,cr,dr,f,X[ 7],0x7A6D76E9UL, 5)
    STEP(dr,er,ar,br,cr,f,X[10],0x7A6D76E9UL,15) STEP(cr,dr,er,ar,br,f,X[14],0x7A6D76E9UL, 8)
#undef f
#define f f1
    STEP(br,cr,dr,er,ar,f,X[12],0x00000000UL, 8) STEP(ar,br,cr,dr,er,f,X[15],0x00000000UL, 5)
    STEP(er,ar,br,cr,dr,f,X[10],0x00000000UL,12) STEP(dr,er,ar,br,cr,f,X[ 4],0x00000000UL, 9)
    STEP(cr,dr,er,ar,br,f,X[ 1],0x00000000UL,12) STEP(br,cr,dr,er,ar,f,X[ 5],0x00000000UL, 5)
    STEP(ar,br,cr,dr,er,f,X[ 8],0x00000000UL,14) STEP(er,ar,br,cr,dr,f,X[ 7],0x00000000UL, 6)
    STEP(dr,er,ar,br,cr,f,X[ 6],0x00000000UL, 8) STEP(cr,dr,er,ar,br,f,X[ 2],0x00000000UL,13)
    STEP(br,cr,dr,er,ar,f,X[13],0x00000000UL, 6) STEP(ar,br,cr,dr,er,f,X[14],0x00000000UL, 5)
    STEP(er,ar,br,cr,dr,f,X[ 0],0x00000000UL,15) STEP(dr,er,ar,br,cr,f,X[ 3],0x00000000UL,13)
    STEP(cr,dr,er,ar,br,f,X[ 9],0x00000000UL,11) STEP(br,cr,dr,er,ar,f,X[11],0x00000000UL,11)
#undef f
    uint32_t t=state[1]+cl+dr; state[1]=state[2]+dl+er; state[2]=state[3]+el+ar;
    state[3]=state[4]+al+br;   state[4]=state[0]+bl+cr; state[0]=t;
}
void ripemd160_init(RIPEMD160_CTX *ctx){
    ctx->h[0]=0x67452301UL;ctx->h[1]=0xEFCDAB89UL;ctx->h[2]=0x98BADCFEUL;
    ctx->h[3]=0x10325476UL;ctx->h[4]=0xC3D2E1F0UL;ctx->buf_len=0;ctx->total_bytes=0;
}
void ripemd160_update(RIPEMD160_CTX *ctx,const void *data,size_t len){
    const uint8_t *d=(const uint8_t*)data; ctx->total_bytes+=len;
    while(len>0){uint32_t sp=64-ctx->buf_len,cp=(uint32_t)len<sp?(uint32_t)len:sp;
        memcpy(ctx->buf+ctx->buf_len,d,cp);ctx->buf_len+=cp;d+=cp;len-=cp;
        if(ctx->buf_len==64){compress(ctx->h,ctx->buf);ctx->buf_len=0;}}
}
void ripemd160_final(uint8_t out[20],RIPEMD160_CTX *ctx){
    uint64_t bits=ctx->total_bytes*8;
    ctx->buf[ctx->buf_len++]=0x80;
    if(ctx->buf_len>56){memset(ctx->buf+ctx->buf_len,0,64-ctx->buf_len);compress(ctx->h,ctx->buf);ctx->buf_len=0;}
    memset(ctx->buf+ctx->buf_len,0,56-ctx->buf_len);
    for(int i=0;i<8;i++) ctx->buf[56+i]=(uint8_t)(bits>>(8*i));
    compress(ctx->h,ctx->buf);
    for(int i=0;i<5;i++){out[4*i]=(uint8_t)ctx->h[i];out[4*i+1]=(uint8_t)(ctx->h[i]>>8);
        out[4*i+2]=(uint8_t)(ctx->h[i]>>16);out[4*i+3]=(uint8_t)(ctx->h[i]>>24);}
}
void ripemd160_first4(const uint8_t data[32], uint8_t out[4]){
    RIPEMD160_CTX ctx;
    ctx.h[0]=0x67452301UL;ctx.h[1]=0xEFCDAB89UL;ctx.h[2]=0x98BADCFEUL;
    ctx.h[3]=0x10325476UL;ctx.h[4]=0xC3D2E1F0UL;
    ctx.buf_len=0;ctx.total_bytes=0;
    ripemd160_update(&ctx,data,32);
    uint64_t bits=32*8;
    ctx.buf[ctx.buf_len++]=0x80;
    if(ctx.buf_len>56){memset(ctx.buf+ctx.buf_len,0,64-ctx.buf_len);compress(ctx.h,ctx.buf);ctx.buf_len=0;}
    memset(ctx.buf+ctx.buf_len,0,56-ctx.buf_len);
    for(int i=0;i<8;i++) ctx.buf[56+i]=(uint8_t)(bits>>(8*i));
    compress(ctx.h,ctx.buf);
    out[0]=(uint8_t)ctx.h[0];
    out[1]=(uint8_t)(ctx.h[0]>>8);
    out[2]=(uint8_t)(ctx.h[0]>>16);
    out[3]=(uint8_t)(ctx.h[0]>>24);
}
