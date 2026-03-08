#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

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


//копирование, перепроверить на утечку памяти
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
    
    unsigned int final_fdigit;
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


//Разность в первом слагаемом
BigInt* sub_num(BigInt* a, BigInt* b){
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
             for(unsigned int i = old_count + 2; i <= new_count; i++) a->data[i] = 0;
        }
        a->data[0] = new_count;
    }

    unsigned int borrow = 0;
    for(unsigned int i = 1; i <= a->data[0]; i++){
        unsigned int value_b = 0;
        if (i <= b->data[0]) value_b = b->data[i];
        else if (i == b->data[0] + 1) value_b = (unsigned int)b->first_digit & ~(1U << (bits_in_digit - 1));

        unsigned int next_borrow = 0;
        if (a->data[i] < value_b || (a->data[i] == value_b && borrow > 0)) {
            next_borrow = 1;
        }
        a->data[i] = a->data[i] - value_b - borrow;
        borrow = next_borrow;
    }

    a->first_digit = (int)((unsigned int)a->first_digit & (1U << (bits_in_digit - 1)));
    
    while(a->data[0] > 0 && a->data[a->data[0]] == 0){
        a->data[0]--;
    }

    unsigned int* tmp = (unsigned int*)realloc(a->data, (a->data[0] + 1) * sizeof(unsigned int));
    if (tmp) {
        a->data = tmp;
    }else {
        fprintf(stderr, "Realloc failed");
        return NULL;
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

//умножение частей блоко
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

//печать десятичного представления числа
void print_decimal(BigInt* n) {
    if (!n) { printf("(null)"); return; }
    
    int bits_in_digit = sizeof(int) * 8;
    unsigned int sign_mask = (1U << (bits_in_digit - 1));
    int is_zero = 1;
    if (((unsigned int)n->first_digit & ~sign_mask) != 0) is_zero = 0;
    for (unsigned int i = 1; i <= n->data[0]; i++) {
        if (n->data[i] != 0) {
            is_zero = 0;
            break;
        }
    }
    
    if (is_zero) { printf("0"); return; }
    if ((unsigned int)n->first_digit & sign_mask) printf("-");

    BigInt* temp = copy_BigInt(n);
    temp->first_digit &= ~sign_mask;

    size_t est_digits = ((size_t)temp->data[0] * 10) + 20;
    char* buffer = (char*)malloc(est_digits);
    if (!buffer) {
        fprintf(stderr, "Malloc failed\n");
        free_BigInt(temp);
        return;
    }
    
    int pos = 0;
    while (1) {
        int temp_zero = 1;
        if (temp->first_digit != 0) temp_zero = 0;
        for (int i = temp->data[0]; i >= 1; i--) {
            if (temp->data[i] != 0) {
                temp_zero = 0;
                break;
            }
        }
        if (temp_zero) break;

        unsigned long long rem = 0;
        unsigned long long val = (unsigned int)temp->first_digit;
        temp->first_digit = (int)(val / 10);
        rem = val % 10;

        for (int i = temp->data[0]; i >= 1; i--) {
            unsigned long long cur = (unsigned long long)temp->data[i] + (rem << 32);
            temp->data[i] = (unsigned int)(cur / 10);
            rem = cur % 10;
        }
        buffer[pos++] = (char)('0' + rem);
    }

    for (int i = pos - 1; i >= 0; i--) putchar(buffer[i]);
    free(buffer);
    free_BigInt(temp);
}

int main() {
    printf("BigInt Calculator\n");
    printf("Enter expressions. Type 'q' to exit.\n");

    while (1) {
        printf("> ");
        BigInt* current_val = input_BigInt();
        if (!current_val) break;

        char c;
        while ((c = getchar()) != '\n' && c != EOF) {
            if (isspace(c)) continue;

            if (c == '+' || c == '-' || c == '*') {
                BigInt* next_val = input_BigInt();
                if (!next_val) {
                    printf("Error: expected number after '%c'\n", c);
                    
                    while ((c = getchar()) != '\n' && c != EOF);
                    break;
                }
                
                BigInt* res = calculate(current_val, next_val, c);
                free_BigInt(current_val);
                free_BigInt(next_val);
                current_val = res;
            } else {
                printf("Unknown operator '%c'\n", c);
                while ((c = getchar()) != '\n' && c != EOF);
                break;
            }
        }

        printf("Result: ");
        print_decimal(current_val);
        printf("\n");
        free_BigInt(current_val);
    }
    
    return 0;
}
