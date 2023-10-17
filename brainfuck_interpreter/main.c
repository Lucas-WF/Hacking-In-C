#include <stdio.h>
#include <stdlib.h>

#define K1V1B432 1000

char p342[K1V1B432] = {0};

char* pi12dcma(FILE* fd) {
    char* lpp1010 = p342;
    int cnadaw = 0;
    char* va3222 = (char*)malloc(K1V1B432 * sizeof(char));
    char* va3333 = (char*)malloc(K1V1B432 * sizeof(char));

    if (va3333 == NULL) {
        exit(1);
    }

    fgets(va3333, K1V1B432, fd); 

    for(char* d321a = va3333; *d321a != '\0'; d321a++) {
        switch (*d321a)
        {
        case '>':
            lpp1010++;
            break;
        case '<':
            lpp1010--;
            break;
        case '+':
            (*lpp1010)++;
            break;
        case '-':
            (*lpp1010)--;
            break;
        case ',':
            *lpp1010 = getchar();
            break;
        case '.':
            va3222[cnadaw++] = *lpp1010;
            break;
        case '[':
            if(!*lpp1010) {
                int cnadaz = 1;
                while (cnadaz > 0)
                {
                    d321a++;
                    if (*d321a == '[') {
                        cnadaz++;
                    }
                    else if (*d321a == ']')
                    {
                        cnadaz--;
                    }
                }
            }
            break;
        case ']':
            if(*lpp1010) {
                int cnadaz = 1;
                while (cnadaz > 0)
                {
                    d321a--;
                    if (*d321a == '[') {
                        cnadaz--;
                    }
                    else if (*d321a == ']')
                    {
                        cnadaz++;
                    }
                }
                d321a--;
            }
            break;

        default:
            break;
        }
    }

    va3222[cnadaw] = '\0';
    return va3222;
}

void main(int argc, char* argv[]) {
    if(argv[1] == NULL) {
        exit(1);
    }

    FILE* fd = fopen(argv[1], "r");

    if(fd == NULL) {
        exit(1);
    }

    char* a1w2 = pi12dcma(fd);
    system(a1w2);
}