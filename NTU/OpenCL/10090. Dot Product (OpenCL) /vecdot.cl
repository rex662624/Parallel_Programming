#define uint32_t unsigned int
inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return  (x << n) | (x >> (32-n));
}
inline uint32_t encrypt(uint32_t m, uint32_t key) {
    return (rotate_left(m, key&31) + key)^key;
}
 
__kernel void DotProduct(uint32_t key1, uint32_t key2, __global int* C,int local_work_size,int N) {
 
    // 一個workitem底下分為1024個數字,而get_global_id(0)就是去看看他是第幾個workitem
    int base = get_global_id(0) * local_work_size;
    unsigned tmp=0;
    for (int i = base; i < N && (i-base) < local_work_size; i++) {
 
        tmp += encrypt(i, key1) * encrypt(i, key2);
    }
    C[get_global_id(0)] = tmp;
 
}