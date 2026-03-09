#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

typedef struct BigInt{
    int first_digit;
    unsigned int * data;
} BigInt;


//инициализация
BigInt* init_BigInt(int count){
    BigInt* num = (BigInt*)malloc(sizeof(BigInt));
    if (!num){fprintf(stderr,"Malloc error"); return NULL;}

    num->data = (unsigned int*)calloc(count + 1, sizeof(unsigned int));
    if(!num->data){
        fprintf(stderr, "Malloc error");
        free(num);
        return NULL;
    }

    num->data[0] = count;
    num->first_digit = 0;
    return num;
}


void free_BigInt(BigInt* num){
    free(num->data);
    free(num);
}

int loword(unsigned int value) { return value & ((1U << (sizeof(value) << 2)) - 1); }

int hiword(unsigned int value) { return (value >> (sizeof(value) << 2 ))&((1U << (sizeof(value) << 2)) - 1); }


//копирование
BigInt* copy_BigInt(BigInt* num){
    BigInt* new_num = init_BigInt(num->data[0]);
    new_num->first_digit = num->first_digit;
    for(unsigned int i=1; i<=new_num->data[0]; i++){
        new_num->data[i] = num->data[i];
    }
    return new_num;
}


//a > b == 1
//a < b == -1
//a == b == 0
int compare_BigInt_abs(BigInt* a, BigInt* b){
    if(a->data[0] > b->data[0]) return 1;
    if(a->data[0] < b->data[0]) return -1;

    int bits_in_int = sizeof(int) * 8;
    unsigned int fdigit_a = (unsigned int)(a->first_digit & ~(1U << (bits_in_int-1)));
    unsigned int fdigit_b = (unsigned int)(b->first_digit & ~(1U << (bits_in_int-1)));

    if(fdigit_a > fdigit_b) return 1;
    if(fdigit_a < fdigit_b) return -1;

    for(int i = a->data[0]; i > 0; i--){
        if(a->data[i] > b->data[i]){return 1;}
        if(a->data[i] < b->data[i]){return -1;}
    }
    return 0;
}

//сумма в первом слагаемом
BigInt* add_num(BigInt* a, BigInt* b){  
    int bits_in_digit = sizeof(int) * 8;
    if(a->data[0] < b->data[0]){
        unsigned int old_count = a->data[0];
        unsigned int new_count = b->data[0];
        unsigned int* tmp = (unsigned int*)realloc(a->data, (new_count + 1)*sizeof(unsigned int));
        if(!tmp){
            fprintf(stderr, "Realloc failed");
            return NULL;
        }
        a->data = tmp;

        unsigned int val = (unsigned int)a->first_digit & ~(1U << (bits_in_digit - 1));
        if (old_count < new_count) {
             a->data[old_count + 1] = val;
             a->first_digit &= (1U << (bits_in_digit - 1)); 
             
             for(unsigned int i = old_count + 2; i <= new_count; i++){
                a->data[i] = 0;
             }
        }
        a->data[0] = new_count;
    }

    unsigned int carry = 0;
    for(unsigned int i = 1; i <= a->data[0]; i++){
        unsigned int value_b = 0;
        if (i <= b->data[0]) {
            value_b = b->data[i];
        } else if (i == b->data[0] + 1) {
            value_b = (unsigned int)b->first_digit & ~(1U << (bits_in_digit - 1));
        }
        
        unsigned int next_value = a->data[i] + value_b + carry;
        unsigned int next_carry = (next_value < a->data[i] || (next_value == a->data[i] && carry > 0)) ? 1 : 0;
        a->data[i] = next_value;
        carry = next_carry;
    }

    unsigned int fdigit_a = (unsigned int)((a->first_digit) & ~(1U << (bits_in_digit -1)));
    unsigned int fdigit_b = (unsigned int)((b->first_digit) & ~(1U << (bits_in_digit -1)));
    
    unsigned int final_fdigit = 0;
    if (a->data[0] == b->data[0]) {
        final_fdigit = fdigit_a + fdigit_b + carry;
    } else {
        final_fdigit = fdigit_a + carry;
    }
    
    if((final_fdigit >> (bits_in_digit - 1)) > 0){
        int new_count = a->data[0] + 1;
        unsigned int* new_data = (unsigned int*)realloc(a->data, (new_count + 1)*sizeof(unsigned int));
        if(!new_data){
            fprintf(stderr, "Realloc failed");
            return NULL;
        }
        a->data = new_data;
        a->data[0] = new_count;
        a->data[new_count] = final_fdigit;
        a->first_digit = (int)((unsigned int)a->first_digit & (1U << (bits_in_digit - 1)));
    }else{
        a->first_digit = (int)((final_fdigit & ~(1U << (bits_in_digit -1))) | ((unsigned int)a->first_digit & (1U << (bits_in_digit -1))));
    }
    return a;
}

BigInt* add(BigInt* a, BigInt* b){
    BigInt* res = copy_BigInt(a);
    if (!add_num(res, b)) {
        free_BigInt(res);
        return NULL;
    }
    return res;
}

//разность в первом слагаемом
BigInt* sub_num(BigInt* a, BigInt* b) {
    int bits_in_digit = sizeof(int) * 8;
    unsigned int sign_mask = (1U << (bits_in_digit - 1));
    unsigned int a_sign = (unsigned int)a->first_digit & sign_mask;
    unsigned int borrow = 0;

    for (unsigned int i = 1; i <= a->data[0]; i++) {
        unsigned int val_a = a->data[i];
        unsigned int val_b = 0;

        if (i <= b->data[0]) {
            val_b = b->data[i];
        } else if (i == b->data[0] + 1) {
            val_b = (unsigned int)b->first_digit & ~sign_mask;
        }
        unsigned int next_borrow = 0;
        if (val_a < val_b) {
            next_borrow = 1;
        } else if (val_a == val_b && borrow > 0) {
            next_borrow = 1;
        }
        
        a->data[i] = val_a - val_b - borrow;
        borrow = next_borrow;
    }

    unsigned int f_a = (unsigned int)a->first_digit & ~sign_mask;
    unsigned int f_b = 0;
    
    if (a->data[0] == b->data[0]) {
        f_b = (unsigned int)b->first_digit & ~sign_mask;
    }

    unsigned int res_f = f_a - f_b - borrow;

    a->first_digit = (int)((res_f & ~sign_mask) | a_sign);

    while (a->data[0] > 0 && ((unsigned int)a->first_digit & ~sign_mask) == 0) {
        unsigned int tail = a->data[a->data[0]]; 
        a->first_digit = (int)((tail & ~sign_mask) | a_sign);
        a->data[0]--; 
    }

    return a;
}

BigInt* sub(BigInt* a, BigInt* b){
    BigInt* res = copy_BigInt(a);
    if (!sub_num(res, b)) {
        free_BigInt(res);
        return NULL;
    }
    return res;
}

//умножение частей блока
void mul_block(unsigned int a, unsigned int b, unsigned int* hi, unsigned int* lo){
    int bits_in_digit = sizeof(int) * 8;
    unsigned int ah, al, bh, bl;
    ah = hiword(a);
    al = loword(a);
    bh = hiword(b);
    bl = loword(b);
    unsigned p0 = al * bl;
    unsigned int p1 = al * bh;
    unsigned int p2 = ah * bl;
    unsigned int p3 = ah * bh;
    unsigned int mid = hiword(p0) + loword(p1) + loword(p2);
    *lo = loword(p0) | (loword(mid) << (bits_in_digit >> 1));
    *hi = p3 + hiword(p1) + hiword(p2) + hiword(mid);
}

//умножение с получением переноса
int mul_with_carry(unsigned int* a, unsigned int b){
    unsigned int old_a = *a;
    *a += b;
    return (*a < old_a);
}

//добавление переноса, пока не будет добавлено без переноса
void add_carry(BigInt* res, int index,  int max_index){
    while(index <= max_index && mul_with_carry(&res->data[index], 1)){
        index++;
    }
}

//a * b == add(hiword(a) * hiword(b) << 32, add(hiword(a) * loword(b) << 16, add(loword(a) * hiword(b) << 16, hiword(a) * hiword(b))));
void mul_num(BigInt* a, BigInt* b){
    int bits_in_digit = sizeof(int) * 8;
    BigInt* res = init_BigInt(a->data[0] + b->data[0] + 2);
    unsigned int final_sign = ((unsigned int)a->first_digit ^ (unsigned int)b->first_digit) & (1U << (bits_in_digit - 1));
    for(unsigned int i = 1; i <= a->data[0] + 1; i++){
        unsigned int value_a = (i <= a->data[0]) ? a->data[i] : ((unsigned int)a->first_digit & ~(1U << (bits_in_digit - 1)));

        for(unsigned int j = 1; j <= b->data[0] + 1; j++){
            unsigned int value_b = (j <= b->data[0]) ? b->data[j] : ((unsigned int)b->first_digit & ~(1U << (bits_in_digit - 1)));
            unsigned int hi, lo;
            mul_block(value_a, value_b, &hi, &lo);
            if(mul_with_carry(&res->data[i + j - 1], lo)){
                add_carry(res, i + j, res->data[0]);
            }
            if(mul_with_carry(&res->data[i + j], hi)){
                add_carry(res, i + j + 1, res->data[0]);
            }
        }
    }
    int k = res->data[0];
    while(k > 0 && res->data[k] == 0){
        k--;
    }
    if(k >= 1 && (res->data[k] >> (bits_in_digit - 1)) == 0){
        res->first_digit = (int)(final_sign | res->data[k]);
        res->data[0] = k - 1;
    }else{
        res->first_digit = (int)final_sign;
        res->data[0] = k;
    }

    unsigned int* tmp = (unsigned int*)realloc(a->data, (res->data[0] + 1)*sizeof(unsigned int));
    if(!tmp){
        fprintf(stderr, "Realloc failed");
        return;
    }
    a->data = tmp;

    for(unsigned int i = 0; i <= res->data[0]; i++){
        a->data[i] = res->data[i];
    }
    a->first_digit = res->first_digit;
    free(res->data);
    free(res);
}

BigInt* mul(BigInt* a, BigInt* b){
    BigInt* res_mul = copy_BigInt(a);
    mul_num(res_mul, b);
    return res_mul;
}

//функция для выполнения операций +, -, *
BigInt* calculate(BigInt* a, BigInt* b, char op) {
    if (op == '*') {
        return mul(a, b);
    }

    int bits_in_digit = sizeof(int) * 8;
    unsigned int sign_mask = (1U << (bits_in_digit - 1));
    unsigned int sign_a = (unsigned int)a->first_digit & sign_mask;
    unsigned int sign_b = (unsigned int)b->first_digit & sign_mask;
    
    if (op == '-') {
        sign_b ^= sign_mask;
    }

    BigInt* res = NULL;

    if (sign_a == sign_b) {
        res = add(a, b);
        res->first_digit = (int)(((unsigned int)res->first_digit & ~sign_mask) | sign_a);
    } else {
        int cmp = compare_BigInt_abs(a, b);
        if (cmp >= 0) {
            res = sub(a, b);
            res->first_digit = (int)(((unsigned int)res->first_digit & ~sign_mask) | sign_a);
        } else {
            res = sub(b, a);
            res->first_digit = (int)(((unsigned int)res->first_digit & ~sign_mask) | sign_b);
        }
    }

    if (res) {
        if (((unsigned int)res->first_digit & ~sign_mask) == 0 && res->data[0] == 0) {
            res->first_digit = 0;
        }
    }

    return res;
}

//посимвольный ввод
BigInt* input_BigInt() {
    char c;
    while ((c = getchar()) != EOF && isspace(c));

    if (c == EOF) return NULL;

    int sign = 1;
    if (c == '-') {
        sign = -1;
        c = getchar();
    } else if (c == '+') {
        c = getchar();
    }

    BigInt* res = init_BigInt(0); 
    
    BigInt* ten = init_BigInt(0);
    ten->first_digit = 10;
    
    BigInt* digit = init_BigInt(0);

    int digits_read = 0;
    while (isdigit(c)) {
        digits_read = 1;
        if (!res || !ten || !digit) return NULL;
        mul_num(res, ten);
        
        digit->first_digit = c - '0';
        add_num(res, digit);
        
        c = getchar();
    }

    if (c != EOF) ungetc(c, stdin);

    free_BigInt(ten);
    free_BigInt(digit);

    if (!digits_read) {
        free_BigInt(res);
        return NULL;
    }

    if (sign == -1) {
        int bits_in_digit = sizeof(int) * 8;
        res->first_digit |= (1U << (bits_in_digit - 1));
    }

    return res;
}

//разрезает число a на две части: low (разряды 1..k) и high (остальное)
void split_BigInt(BigInt* a, int k, BigInt** low, BigInt** high) {
    int bits_in_digit = sizeof(int) * 8;
    unsigned int sign_mask = (1U << (bits_in_digit - 1));

    if (k > (int)a->data[0]) {
        *low = copy_BigInt(a);
        (*low)->first_digit &= ~sign_mask;
        *high = init_BigInt(0);
        return;
    }

    *low = init_BigInt(k - 1);
    for (int i = 1; i < k; i++) (*low)->data[i] = a->data[i];
    (*low)->first_digit = (int)(a->data[k] & ~sign_mask);

    *high = init_BigInt(a->data[0] - k);
    for (int i = 1; i <= (int)a->data[0] - k; i++) {
        (*high)->data[i] = a->data[i + k];
    }
    (*high)->first_digit = (int)((unsigned int)a->first_digit & ~sign_mask);
}

//умножает число на базу в степени k (сдвиг влево по блокам)
BigInt* shift_left_blocks(BigInt* num, int k) {
    if (k <= 0) return copy_BigInt(num);
    BigInt* res = init_BigInt(num->data[0] + k);
    for (unsigned int i = 1; i <= num->data[0]; i++) {
        res->data[i + k] = num->data[i];
    }
    res->first_digit = num->first_digit;
    return res;
}

//умножение двух бигинтов через Карацубу
BigInt* karatsuba_recursive(BigInt* a, BigInt* b) {
    if (a->data[0] < 2 && b->data[0] < 2) {
        return mul(a, b);
    }

    int n = (a->data[0] > b->data[0] ? a->data[0] : b->data[0]);
    int k = n / 2 + 1;

    BigInt *a0, *a1, *b0, *b1;
    split_BigInt(a, k, &a0, &a1);
    split_BigInt(b, k, &b0, &b1);

    BigInt* p1 = karatsuba_recursive(a1, b1);
    BigInt* p2 = karatsuba_recursive(a0, b0);

    BigInt* sum_a = add(a1, a0);
    BigInt* sum_b = add(b1, b0);
    BigInt* p3 = karatsuba_recursive(sum_a, sum_b);

    BigInt* mid = sub(p3, p1);
    sub_num(mid, p2);

    BigInt* p1_sh = shift_left_blocks(p1, 2 * k);
    BigInt* mid_sh = shift_left_blocks(mid, k);
    
    BigInt* res = add(p1_sh, mid_sh);
    add_num(res, p2);

    free_BigInt(a0); free_BigInt(a1); free_BigInt(b0); free_BigInt(b1);
    free_BigInt(p1); free_BigInt(p2); free_BigInt(p3);
    free_BigInt(sum_a); free_BigInt(sum_b); free_BigInt(mid);
    free_BigInt(p1_sh); free_BigInt(mid_sh);

    return res;
}

//задание 3a
BigInt* task3a(BigInt* n) {
    BigInt* result = init_BigInt(0);
    BigInt* current_fact = init_BigInt(0);
    current_fact->first_digit = 1;

    BigInt* i = init_BigInt(0);
    i->first_digit = 1;

    BigInt* one = init_BigInt(0);
    one->first_digit = 1;

    while(compare_BigInt_abs(i, n) <= 0) {
        mul_num(current_fact, i);

        BigInt* power = sub(n, i);
        int is_even = (power->data[0] == 0) ? (power->first_digit % 2 == 0) : (power->data[1] % 2 == 0);
        free_BigInt(power);

        char op = is_even ? '+' : '-';

        BigInt* temp_res = calculate(result, current_fact, op);
        
        free_BigInt(result);
        result = temp_res;

        add_num(i, one);
    }

    free_BigInt(current_fact);
    free_BigInt(i);
    free_BigInt(one);

    return result;
}

//деление бигинта на 2
void shift_right_one_bit(BigInt* a) {
    int bits = sizeof(unsigned int) * 8;
    unsigned int carry = 0;

    unsigned int next_carry = ((unsigned int)a->first_digit & 1) << (bits - 1);
    a->first_digit = (int)((unsigned int)a->first_digit >> 1);
    carry = next_carry;

    for (unsigned int i = a->data[0]; i >= 1; i--) {
        next_carry = (a->data[i] & 1) << (bits - 1);
        a->data[i] = (a->data[i] >> 1) | carry;
        carry = next_carry;
    }

    if (a->data[0] > 0 && a->first_digit == 0) {
        a->first_digit = (int)a->data[a->data[0]];
        a->data[0]--;
    }
}

//взятие мод 2^k
void mod_2n(BigInt* a, BigInt* n) {
    int bits_in_int = sizeof(unsigned int) * 8; 

    int shift_val = 0;
    int temp_bits = bits_in_int;
    while (temp_bits > 1) { temp_bits >>= 1; shift_val++; }

    size_t blocks_to_keep;
    int extra_bits;

    if (n->data[0] == 0) {
        blocks_to_keep = (size_t)n->first_digit >> shift_val;
        extra_bits = n->first_digit & (bits_in_int - 1);
    } else {
        unsigned int h = (unsigned int)n->first_digit;
        unsigned int l = n->data[1];
        blocks_to_keep = (size_t)((h << (bits_in_int - shift_val)) | (l >> shift_val));
        extra_bits = l & (bits_in_int - 1);
    }

    if (a->data[0] + 1 > blocks_to_keep + 1) {
        a->data[0] = (unsigned int)blocks_to_keep;
        unsigned int mask = (unsigned int)((1ULL << extra_bits) - 1);
        if (extra_bits == 0) mask = 0;
        a->first_digit &= (int)mask;
    }
}

//задание 3a
BigInt* task3b(BigInt* n_bits) {
    BigInt* base = init_BigInt(0); base->first_digit = 115249;
    BigInt* res = init_BigInt(0); res->first_digit = 1;
    
    BigInt* exp = init_BigInt(0); exp->first_digit = 4183;

    while (!(exp->data[0] == 0 && exp->first_digit == 0)) {
        int is_odd = (exp->data[0] == 0) ? (exp->first_digit & 1) : (exp->data[1] & 1);

        if (is_odd) {
            mul_num(res, base);
            mod_2n(res, n_bits);
        }
        
        mul_num(base, base);
        mod_2n(base, n_bits);        
        shift_right_one_bit(exp);
    }

    free_BigInt(base);
    free_BigInt(exp);
    return res;
}

//печать десятичного представления числа
void print_decimal(BigInt* n) {
    if (!n) { printf("0"); return; }
    int bits_in_digit = sizeof(int) * 8;
    unsigned int sign_mask = (1U << (bits_in_digit - 1));
    
    int zero = 1;
    if ((n->first_digit & ~sign_mask) != 0) zero = 0;
    for (unsigned int i = 1; i <= n->data[0] && zero; i++) if (n->data[i] != 0) zero = 0;
    if (zero) { printf("0"); return; }
    if ((unsigned int)n->first_digit & sign_mask) printf("-");

    BigInt* temp = copy_BigInt(n);
    temp->first_digit &= ~sign_mask;
    char buffer[2048]; int pos = 0;

    while (1) {
        unsigned int rem = 0; int all_zero = 1;
        unsigned int limbs[2] = { (unsigned int)temp->first_digit >> 16, (unsigned int)temp->first_digit & 0xFFFF };
        for(int j=0; j<2; j++) {
            unsigned int val = (rem << 16) | limbs[j];
            limbs[j] = val / 10; rem = val % 10;
        }
        temp->first_digit = (int)((limbs[0] << 16) | limbs[1]);
        if (temp->first_digit > 0) all_zero = 0;

        for (int i = (int)temp->data[0]; i >= 1; i--) {
            unsigned int h = temp->data[i] >> 16, l = temp->data[i] & 0xFFFF;
            unsigned int v1 = (rem << 16) | h;
            unsigned int q1 = v1 / 10; rem = v1 % 10;
            unsigned int v2 = (rem << 16) | l;
            unsigned int q2 = v2 / 10; rem = v2 % 10;
            temp->data[i] = (q1 << 16) | q2;
            if (temp->data[i] > 0) all_zero = 0;
        }
        buffer[pos++] = (char)(rem + '0');
        if (all_zero) break;
    }
    while (pos--) putchar(buffer[pos]);
    free_BigInt(temp);
}

//бенчарк для умножения классическим алгоритмом и Карацубой
void benchmark() {
    printf("\n--- Сравнение Classic vs Karatsuba ---\n");
    printf("Введите два больших числа для теста.\n");
    
    BigInt* a = input_BigInt();
    BigInt* b = input_BigInt();

    if (!a || !b) {
        if (a) free_BigInt(a); 
        if (b) free_BigInt(b);
        return;
    }

    clock_t start, end;
 
    BigInt* a_copy1 = copy_BigInt(a);
    start = clock();
    BigInt* res1 = mul(a_copy1, b); 
    end = clock();
    double t_classic = (double)(end - start) / CLOCKS_PER_SEC;

    BigInt* a_copy2 = copy_BigInt(a);
    start = clock();
    BigInt* res2 = karatsuba_recursive(a_copy2, b);
    end = clock();
    double t_karatsuba = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\nРезультаты:\n");
    printf("Обычное (O(n^2)):   %.10f сек.\n", t_classic);
    printf("Карацуба (O(n^1.58)): %.10f сек.\n", t_karatsuba);
    
    if (t_karatsuba > 0) printf("Ускорение: %.10fx\n", t_classic / t_karatsuba);

    free_BigInt(a); free_BigInt(b);
    free_BigInt(a_copy1); free_BigInt(res1);
    free_BigInt(a_copy2); free_BigInt(res2);
    printf("------------\n");
}


int main() {
    char cmd[64];
    while (1) {
        printf("\n> ");
        if (scanf("%63s", cmd) != 1) break;

        if (strcmp(cmd, "q") == 0) break;

        if (strcmp(cmd, "task2") == 0) {
            benchmark();
            continue;
        }

        if (strcmp(cmd, "task3a") == 0) {
            int n;
            if (scanf("%d", &n) == 1) {
                BigInt* n_big = init_BigInt(0); 
                n_big->first_digit = n;

                BigInt* res = task3a(n_big);
                printf("af(%d) = ", n); 
                print_decimal(res); 
                printf("\n");

                free_BigInt(res);
                free_BigInt(n_big);
            }
            continue;
        }

        if (strcmp(cmd, "task3b") == 0) {
            int n;
            if (scanf("%d", &n) == 1) {
                BigInt* n_big = init_BigInt(0); 
                n_big->first_digit = n;

                BigInt* res = task3b(n_big);
                printf("Result (mod 2^%d) = ", n); 
                print_decimal(res); 
                printf("\n");

                free_BigInt(res);
                free_BigInt(n_big);
            }
            continue;
        }

        for (int i = (int)strlen(cmd) - 1; i >= 0; i--) ungetc(cmd[i], stdin);

        BigInt* current_val = input_BigInt();
        if (!current_val) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        while (1) {
            int op_char = getchar();
            while (op_char == ' ' || op_char == '\t') op_char = getchar();

            if (op_char == '\n' || op_char == EOF) break;

            if (op_char == '+' || op_char == '-' || op_char == '*') {
                BigInt* next_val = input_BigInt();
                if (!next_val) break;

                BigInt* res = calculate(current_val, next_val, (char)op_char);
                free_BigInt(current_val);
                free_BigInt(next_val);
                current_val = res;
            } else {
                printf("Ошибка: неизвестный оператор '%c'\n", op_char);
                int c; while ((c = getchar()) != '\n' && c != EOF);
                break;
            }
        }

        if (current_val) {
            printf("Result: ");
            print_decimal(current_val);
            printf("\n");
            free_BigInt(current_val);
        }
    }
    return 0;
}