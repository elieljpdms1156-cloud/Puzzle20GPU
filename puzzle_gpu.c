/*
 * puzzle_gpu.c – Solver Bitcoin Puzzle com GPU (OpenCL)
 * SHA256 na GPU, RIPEMD160 na CPU. Prefiltro de 4 bytes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <CL/cl.h>
#include <secp256k1.h>
#include "ripemd160.h"

typedef uint64_t u64;

/* Alvos */
static const uint8_t T20[20] = {0xb9,0x07,0xc3,0xa2,0xa3,0xb2,0x77,0x89,0xdf,0xb5,0x09,0xb7,0x30,0xdd,0x47,0x70,0x3c,0x27,0x28,0x68};
static const uint8_t T25[20] = {0x2f,0x39,0x6b,0x29,0xb2,0x73,0x24,0x30,0x0d,0x0c,0x59,0xb1,0x7c,0x3a,0xbc,0x18,0x35,0xbd,0x3d,0xbb};

/* ========== Kernel OpenCL ========== */
static const char* kernelSrc = R"KERNEL(
typedef uint uint32_t;
typedef ulong uint64_t;

__constant uint K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

inline uint rightRotate(uint x, uint n) { return (x >> n) | (x << (32-n)); }

__kernel void sha256_batch(__global const uchar* pubkeys, __global uint* hashes) {
    int gid = get_global_id(0);
    __global const uchar* pub = pubkeys + gid * 33;
    uint w[64];
    for(int i=0; i<8; i++)
        w[i] = ((uint)pub[4*i]<<24) | (pub[4*i+1]<<16) | (pub[4*i+2]<<8) | pub[4*i+3];
    w[8] = ((uint)pub[32]<<24) | 0x800000;
    for(int i=9; i<15; i++) w[i] = 0;
    w[15] = 264;
    uint a = 0x6a09e667, b = 0xbb67ae85, c = 0x3c6ef372, d = 0xa54ff53a,
         e = 0x510e527f, f = 0x9b05688c, g = 0x1f83d9ab, h = 0x5be0cd19;
    for(int i=0; i<64; i++) {
        uint t1 = h + (rightRotate(e,6) ^ rightRotate(e,11) ^ rightRotate(e,25)) + ((e & f) ^ (~e & g)) + K[i] + w[i];
        uint t2 = (rightRotate(a,2) ^ rightRotate(a,13) ^ rightRotate(a,22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    uint s0 = a + 0x6a09e667, s1 = b + 0xbb67ae85, s2 = c + 0x3c6ef372, s3 = d + 0xa54ff53a,
         s4 = e + 0x510e527f, s5 = f + 0x9b05688c, s6 = g + 0x1f83d9ab, s7 = h + 0x5be0cd19;
    uint w2[16] = {0};
    w2[0] = 0x80000000; w2[15] = 256;
    a = s0; b = s1; c = s2; d = s3; e = s4; f = s5; g = s6; h = s7;
    for(int i=0; i<64; i++) {
        uint t1 = h + (rightRotate(e,6) ^ rightRotate(e,11) ^ rightRotate(e,25)) + ((e & f) ^ (~e & g)) + K[i] + w2[i];
        uint t2 = (rightRotate(a,2) ^ rightRotate(a,13) ^ rightRotate(a,22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    hashes[gid*8 + 0] = a + s0;
    hashes[gid*8 + 1] = b + s1;
    hashes[gid*8 + 2] = c + s2;
    hashes[gid*8 + 3] = d + s3;
    hashes[gid*8 + 4] = e + s4;
    hashes[gid*8 + 5] = f + s5;
    hashes[gid*8 + 6] = g + s6;
    hashes[gid*8 + 7] = h + s7;
}
)KERNEL";

/* ========== Auxiliares ========== */
static secp256k1_context *g_ctx;
static void make_privkey(uint8_t priv[32], u64 hi, u64 lo) {
    memset(priv,0,32);
    priv[16]=(hi>>56)&0xFF; priv[17]=(hi>>48)&0xFF;
    priv[18]=(hi>>40)&0xFF; priv[19]=(hi>>32)&0xFF;
    priv[20]=(hi>>24)&0xFF; priv[21]=(hi>>16)&0xFF;
    priv[22]=(hi>> 8)&0xFF; priv[23]=hi&0xFF;
    priv[24]=(lo>>56)&0xFF; priv[25]=(lo>>48)&0xFF;
    priv[26]=(lo>>40)&0xFF; priv[27]=(lo>>32)&0xFF;
    priv[28]=(lo>>24)&0xFF; priv[29]=(lo>>16)&0xFF;
    priv[30]=(lo>> 8)&0xFF; priv[31]=lo&0xFF;
}
static void make_step_tweak(uint8_t tw[32], int step) {
    memset(tw,0,32); tw[31]=(uint8_t)(step&0xFF); tw[30]=(uint8_t)((step>>8)&0xFF);
}
static inline void inc128(u64*h,u64*l){if(++(*l)==0)++(*h);}
static inline void add128(u64*h,u64*l,u64 v){u64 old=*l;*l+=v;if(*l<old)++(*h);}
static inline int cmp128(u64 ah,u64 al,u64 bh,u64 bl){
    if(ah!=bh)return ah<bh?-1:1;if(al!=bl)return al<bl?-1:1;return 0;
}

int main(int argc, char* argv[]) {
    int puzzle = argc>1 ? atoi(argv[1]) : 25;
    int nth    = argc>2 ? atoi(argv[2]) : 2;
    const uint8_t *tgt20 = (puzzle==20)? T20 : T25;
    uint32_t tgt32 = *(uint32_t*)tgt20;

    g_ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    if(!g_ctx){ fprintf(stderr,"secp256k1 init fail\n"); return 1; }

    cl_platform_id plat; cl_device_id dev; cl_context ctx; cl_command_queue q;
    cl_program prog; cl_kernel k; cl_int ret;
    clGetPlatformIDs(1,&plat,NULL);
    clGetDeviceIDs(plat,CL_DEVICE_TYPE_GPU,1,&dev,NULL);
    ctx = clCreateContext(NULL,1,&dev,NULL,NULL,&ret);
    q = clCreateCommandQueue(ctx,dev,0,&ret);
    prog = clCreateProgramWithSource(ctx,1,&kernelSrc,NULL,&ret);
    ret = clBuildProgram(prog,1,&dev,NULL,NULL,NULL);
    if(ret!=CL_SUCCESS){
        size_t logSize; clGetProgramBuildInfo(prog,dev,CL_PROGRAM_BUILD_LOG,0,NULL,&logSize);
        char *log=malloc(logSize); clGetProgramBuildInfo(prog,dev,CL_PROGRAM_BUILD_LOG,logSize,log,NULL);
        fprintf(stderr,"Kernel error: %s\n",log); free(log); return 1;
    }
    k = clCreateKernel(prog,"sha256_batch",&ret);

    u64 lhi=0,llo,hhi=0,hlo;
    if(puzzle==20){ llo=0x80000ULL; hlo=0xFFFFFULL; }
    else { llo=0x1000000ULL; hlo=0x1FFFFFFULL; }

    printf("================================\n");
    printf("[+] Puzzle %d | Threads %d | GPU detectada\n",puzzle,nth);
    printf("================================\n");

    #define BATCH 16384
    cl_mem pub_buf = clCreateBuffer(ctx,CL_MEM_READ_ONLY,BATCH*33,NULL,NULL);
    cl_mem hash_buf = clCreateBuffer(ctx,CL_MEM_WRITE_ONLY,BATCH*8*sizeof(uint32_t),NULL,NULL);
    uint8_t *host_pub = malloc(BATCH*33);
    uint32_t *host_hash = malloc(BATCH*8*sizeof(uint32_t));
    uint8_t first4[4];

    u64 chi=lhi, clo=llo;
    uint8_t step_tw[32]; make_step_tweak(step_tw,nth);
    secp256k1_pubkey cur_pk;
    uint8_t priv[32]; make_privkey(priv,chi,clo);
    secp256k1_ec_pubkey_create(g_ctx,&cur_pk,priv);
    time_t t0=time(NULL);
    u64 total=0;

    while(cmp128(chi,clo,hhi,hlo)<=0) {
        int batch_count=0;
        for(int i=0; i<BATCH && cmp128(chi,clo,hhi,hlo)<=0; i++) {
            size_t len=33;
            secp256k1_ec_pubkey_serialize(g_ctx,host_pub+i*33,&len,&cur_pk,SECP256K1_EC_COMPRESSED);
            if(!secp256k1_ec_pubkey_tweak_add(g_ctx,&cur_pk,step_tw)) {
                make_privkey(priv,chi,clo);
                secp256k1_ec_pubkey_create(g_ctx,&cur_pk,priv);
            }
            add128(&chi,&clo,(u64)nth);
            batch_count++;
        }
        clEnqueueWriteBuffer(q,pub_buf,CL_TRUE,0,batch_count*33,host_pub,0,NULL,NULL);
        clSetKernelArg(k,0,sizeof(cl_mem),&pub_buf);
        clSetKernelArg(k,1,sizeof(cl_mem),&hash_buf);
        size_t global=batch_count;
        clEnqueueNDRangeKernel(q,k,1,NULL,&global,NULL,0,NULL,NULL);
        clEnqueueReadBuffer(q,hash_buf,CL_TRUE,0,batch_count*8*sizeof(uint32_t),host_hash,0,NULL,NULL);

        for(int i=0; i<batch_count; i++) {
            uint8_t sha[32];
            for(int j=0; j<8; j++) {
                sha[j*4]   = host_hash[i*8+j]>>24;
                sha[j*4+1] = host_hash[i*8+j]>>16;
                sha[j*4+2] = host_hash[i*8+j]>>8;
                sha[j*4+3] = host_hash[i*8+j];
            }
            ripemd160_first4(sha, first4);
            if(*(uint32_t*)first4 == tgt32) {
                RIPEMD160_CTX rctx;
                ripemd160_init(&rctx);
                ripemd160_update(&rctx, sha, 32);
                uint8_t full[20];
                ripemd160_final(full, &rctx);
                if(memcmp(full, tgt20, 20)==0) {
                    printf("\n[!!!] ENCONTRADO! Puzzle #%d\n", puzzle);
                    printf("Chave (hex): %016llx%016llx\n", (unsigned long long)chi, (unsigned long long)clo);
                    goto done;
                }
            }
        }
        total += batch_count;
        if(total % (BATCH*64)==0) {
            double elapsed = difftime(time(NULL), t0);
            printf("[stats] keys=%-12llu %.3f Mkeys/s\n", (unsigned long long)total, total/elapsed/1e6);
        }
    }
    printf("\n[-] Não encontrado.\n");
done:
    clReleaseMemObject(pub_buf); clReleaseMemObject(hash_buf);
    clReleaseKernel(k); clReleaseProgram(prog);
    clReleaseCommandQueue(q); clReleaseContext(ctx);
    free(host_pub); free(host_hash);
    secp256k1_context_destroy(g_ctx);
    return 0;
}
