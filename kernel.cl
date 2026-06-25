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
