#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <CL/cl.h>
#include <secp256k1.h>
#include "ripemd160.h"

#define MAX_TARGETS 256
#define BATCH 16384

typedef uint64_t u64;

static uint8_t targets[MAX_TARGETS][20];
static int num_targets = 0;
static uint32_t target_first4[MAX_TARGETS];

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

static char* read_file(const char* filename, size_t* out_len) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    rewind(f);
    char* buf = (char*)malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    if (out_len) *out_len = len;
    fclose(f);
    return buf;
}

static void menu() {
    printf("\n========= COLECIONADOR DE CARTEIRAS =========\n");
    printf("1. Adicionar hash160 (hex, 40 caracteres)\n");
    printf("2. Carregar hash160s de arquivo (alvos.txt)\n");
    printf("3. Listar alvos (%d cadastrados)\n", num_targets);
    printf("4. Iniciar busca com GPU\n");
    printf("5. Sair\n");
    printf("Opcao: ");
}
static int add_target(const char* hex) {
    if (strlen(hex) != 40) return 0;
    if (num_targets >= MAX_TARGETS) return 0;
    uint8_t *t = targets[num_targets];
    for (int i=0; i<20; i++) {
        unsigned int byte;
        sscanf(hex + 2*i, "%2x", &byte);
        t[i] = (uint8_t)byte;
    }
    target_first4[num_targets] = *(uint32_t*)t;
    num_targets++;
    return 1;
}
static void load_from_file() {
    FILE *f = fopen("alvos.txt", "r");
    if (!f) { printf("Arquivo alvos.txt nao encontrado.\n"); return; }
    char line[128];
    int added = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = strchr(line, '\n'); if(p) *p=0;
        p = strchr(line, '\r'); if(p) *p=0;
        if (strlen(line) == 40) {
            if (add_target(line)) added++;
        }
    }
    fclose(f);
    printf("%d hash160s carregados.\n", added);
}
static void list_targets() {
    printf("\n==== %d ALVOS CADASTRADOS ====\n", num_targets);
    for (int i=0; i<num_targets; i++) {
        printf("%2d: ", i+1);
        for (int j=0; j<20; j++) printf("%02x", targets[i][j]);
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    g_ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    if(!g_ctx){ fprintf(stderr,"secp256k1 init fail\n"); return 1; }

    printf("=== Colecionador de Carteiras (GPU) ===\n");
    while (1) {
        menu();
        int opcao;
        scanf("%d", &opcao);
        getchar();
        if (opcao == 1) {
            char hex[128];
            printf("Cole o hash160 (40 hex): ");
            fgets(hex, sizeof(hex), stdin);
            hex[40] = 0;
            if (add_target(hex)) printf("Adicionado!\n");
            else printf("Formato invalido ou limite atingido.\n");
        } else if (opcao == 2) {
            load_from_file();
        } else if (opcao == 3) {
            list_targets();
        } else if (opcao == 4) {
            if (num_targets == 0) { printf("Nenhum alvo.\n"); continue; }
            printf("Intervalo (inicio fim em hex): ");
            u64 llo, hlo;
            scanf("%llx %llx", &llo, &hlo); getchar();
            u64 lhi=0, hhi=0;
            int nth;
            printf("Threads CPU: ");
            scanf("%d", &nth); getchar();

            // Carrega kernel do arquivo kernel.cl
            size_t kernel_len = 0;
            char* kernelSrc = read_file("kernel.cl", &kernel_len);
            if (!kernelSrc) {
                fprintf(stderr, "Erro ao ler kernel.cl\n");
                continue;
            }

            cl_platform_id plat; cl_device_id dev; cl_context ctx; cl_command_queue q;
            cl_program prog; cl_kernel k; cl_int ret;
            clGetPlatformIDs(1,&plat,NULL);
            clGetDeviceIDs(plat,CL_DEVICE_TYPE_GPU,1,&dev,NULL);
            ctx = clCreateContext(NULL,1,&dev,NULL,NULL,&ret);
            q = clCreateCommandQueue(ctx,dev,0,&ret);
            prog = clCreateProgramWithSource(ctx,1,(const char**)&kernelSrc,&kernel_len,&ret);
            ret = clBuildProgram(prog,1,&dev,NULL,NULL,NULL);
            if(ret!=CL_SUCCESS){
                size_t logSize; clGetProgramBuildInfo(prog,dev,CL_PROGRAM_BUILD_LOG,0,NULL,&logSize);
                char *log=malloc(logSize); clGetProgramBuildInfo(prog,dev,CL_PROGRAM_BUILD_LOG,logSize,log,NULL);
                fprintf(stderr,"Kernel error: %s\n",log); free(log); free(kernelSrc); return 1;
            }
            k = clCreateKernel(prog,"sha256_batch",&ret);
            free(kernelSrc);

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
                    uint32_t f4 = *(uint32_t*)first4;
                    for (int t=0; t<num_targets; t++) {
                        if (f4 == target_first4[t]) {
                            RIPEMD160_CTX rctx;
                            ripemd160_init(&rctx);
                            ripemd160_update(&rctx, sha, 32);
                            uint8_t full[20];
                            ripemd160_final(full, &rctx);
                            if (memcmp(full, targets[t], 20) == 0) {
                                printf("\n[!!!] ENCONTRADO ALVO %d!\n", t+1);
                                printf("Chave privada: %016llx%016llx\n", (unsigned long long)chi, (unsigned long long)clo);
                                printf("Hash160: ");
                                for (int j=0; j<20; j++) printf("%02x", full[j]);
                                printf("\n");
                            }
                        }
                    }
                }
                total += batch_count;
                if(total % (BATCH*64)==0) {
                    double elapsed = difftime(time(NULL), t0);
                    printf("[stats] keys=%-12llu %.3f Mkeys/s\n", (unsigned long long)total, total/elapsed/1e6);
                }
            }
            printf("\nBusca concluida.\n");
            clReleaseMemObject(pub_buf); clReleaseMemObject(hash_buf);
            clReleaseKernel(k); clReleaseProgram(prog);
            clReleaseCommandQueue(q); clReleaseContext(ctx);
            free(host_pub); free(host_hash);
        } else if (opcao == 5) {
            break;
        }
    }
    secp256k1_context_destroy(g_ctx);
    return 0;
}
