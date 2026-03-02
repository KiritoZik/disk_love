#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef struct BigInt{
    int first_digit;
    unsigned int * data;
} BigInt;


//инициализация
BigInt* init_BigInt(int count){
    BigInt* num = (BigInt*)malloc(sizeof(BigInt));
    if (!num){fprintf(stderr,"Malloc error"); exit(1);}

    num->data = (unsigned int*)malloc((count+1)*sizeof(unsigned int));
    if(!num->data){
        fprintf(stderr, "Malloc error");
        free(num);
        exit(1);
    }

    num->data[0] = count;
    for(int i=1; i <= count; i++){
        num->data[i] = 0;
    }
    return num;
}


void free_BigInt(BigInt* num){
    free(num->data);
    free(num);
}


//Подумать, стоит ли такое писать
// BigInt* convert_to_bigint(long long value){
//     unsigned long long uvalue = value > 0 ? (unsigned long long) value : -(unsigned long long)value;
//     int bit_in_int = sizeof(int) * 8;
//     int capacity = (sizeof(long long) + sizeof(int) - 1) / sizeof(int);
//     BigInt* res = init_BigInt(capacity-1);
//     for (int i=1; i < capacity; i++){
//         res->data[i] = (unsigned int)(uvalue);
//         uvalue >>= bit_in_int;
//     }
//     res->first_digit = (int)(uvalue);
//     if(value<0){
//         res->first_digit |= 1U << (bit_in_int - 1);
//     }
//     return res;
// }


BigInt* copy_BigInt(BigInt* num){
    BigInt* new_num = init_BigInt(num->data[0]);
    new_num->first_digit = num->first_digit;
    for(int i=1; i<=new_num->data[0]; i++){
        new_num->data[i] = num->data[i];
    }
    return new_num;
}


//сумма в первом слагаемом
BigInt* add_num(BigInt* a, BigInt* b){
    int bits_in_digit = sizeof(int) * 8;
    if(a->data[0] < b->data[0]){
        unsigned int* tmp = (unsigned int*)realloc(a->data, (b->data[0]+1)*sizeof(unsigned int));
        if(!tmp){
            fprintf(stderr, "Realloc failed");
            exit(1);
        }
        a->data = tmp;

        for(int i=a->data[0]+1; i<=(b->data[0]);i++){
            a->data[i] = 0;
        }
        a->data[0] = b->data[0];
    }
    unsigned int carry = 0;
    for(int i = 1; i <= a->data[0]; i++){
        unsigned int value_b = (i <= b->data[0]) ? b->data[i] : 0;
        unsigned int next_value = a->data[i] + value_b + carry;
        unsigned int next_carry = (next_value < a->data[i] || (next_value == a->data[i] && carry > 0)) ? 1 : 0;
        a->data[i] = next_value;
        carry = next_carry;
    }

    unsigned int fdigit_a = (unsigned int)((a->first_digit) & ~(1U << (bits_in_digit -1)));
    unsigned int fdigit_b = (unsigned int)((b->first_digit) & ~(1U << (bits_in_digit -1)));
    unsigned int final_fdigit = fdigit_a + fdigit_b + carry;
    a->first_digit = (int)((final_fdigit & ~(1U << (bits_in_digit -1))) | ((unsigned int)a->first_digit & (1U << (bits_in_digit -1))));
    return a;
}

//сделать функцию для сложения и вычитания, которая будет вызывать их смотря на знак выражения
BigInt* add(BigInt* a, BigInt* b){
    BigInt* res = copy_BigInt(a);
    add_num(res, b);
    return res;
}


//a > b == 1
//a < b == -1
//a == b == 0
int compare_BigInt(BigInt* a, BigInt* b){
    if(a->data[0]>b->data[0]){
        return 1;
    }else if(a->data[0] < b->data[0]){
        return -1;
    }

    int bits_in_int = sizeof(int) * 8;
    unsigned int fdigit_a = (unsigned int)a->first_digit & ~(1U << (bits_in_int-1));
    unsigned int fdigit_b = (unsigned int)b->first_digit & ~(1U << (bits_in_int-1));

    if(fdigit_a > fdigit_b){
        return 1;
    }else if(fdigit_a < fdigit_b){
        return -1;
    }

    for(int i = a->data[0]; i > 0; i--){
        if(a->data[i] > b->data[i]){return 1;}
        if(a->data[i] < b->data[i]){return -1;}
    }
    return 0;
}

BigInt* sub_num(BigInt* a, BigInt* b){
    
}





int loword(unsigned int value){return value & ((1<<(sizeof(value)<<2))-1);} //подумать можеть все-таки unigned int


int hiword(unsigned int value){return (value >> (sizeof(value)<<2))&((1 << (sizeof(value) << 2))-1);}
