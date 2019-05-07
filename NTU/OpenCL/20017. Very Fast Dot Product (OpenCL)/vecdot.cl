#define uint32_t unsigned int
inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return  (x << n) | (x >> (32-n));
}
inline uint32_t encrypt(uint32_t m, uint32_t key) {
    return (rotate_left(m, key&31) + key)^key;
}

__kernel void DotProduct(uint32_t key1, uint32_t key2, __global int* C,int local_work_size,int N) {
    __local int buffer[256];

    int globalId = get_global_id(0);
    int groupId = get_group_id(0);
    int localId = get_local_id(0);
    int localSz = get_local_size(0);

    //先把自己那個item內的1024個數據算完
    unsigned tmp = 0;
    int base = globalId * local_work_size;
    for (int i = base; i < N && (i-base) < local_work_size; i++) {
        tmp += encrypt(i, key1) * encrypt(i, key2);
    }
    buffer[localId] = tmp;


    barrier(CLK_LOCAL_MEM_FENCE);
    //然後第二階段1024個已經一組了，這裡是又把它加起來，變成是1024*256,也就是一個256個item,也就是一個group的數字加起來
    //這樣等等回cpu就只剩下group個數字要加而已
    //一半一半一直去切 兩個兩個相加最後答案就在第一個,所以每次只有一半的人做這個動作,再來1/4 ......
    #pragma unroll 10
    for (int i = (localSz/2);i>0; i /=2) {
        if (localId < i)
            buffer[localId] += buffer[localId+i];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (localId == 0)
        C[groupId] = buffer[0];
        
}