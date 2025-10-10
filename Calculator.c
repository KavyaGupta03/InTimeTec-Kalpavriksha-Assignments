#include <stdio.h>
#include <ctype.h>
#define MAX_SIZE 1000

int val_Stack[MAX_SIZE];
int vSt_top = -1;
char op_Stack[MAX_SIZE]; 
int oSt_top = -1;

void pushVal(int x) {
    if (vSt_top >= MAX_SIZE - 1) {
        printf("Error: Value Stack Overflow !!!\n");
        return;
    }
    val_Stack[++vSt_top] = x;
}

int popVal() {
    if (vSt_top < 0) {
        printf("Error: Value Stack Underflow !!!\n");
        return 0;
    }
    return val_Stack[vSt_top--];
}

void pushOp(char c) {
    if (oSt_top >= MAX_SIZE - 1) {
        printf("Error: Operator Stack Overflow !!!\n");
        return;
    }
    op_Stack[++oSt_top] = c;
}

char popOp() {
    if (oSt_top < 0) {
        printf("Error: Operator Stack Underflow !!!\n");
        return 0;
    }
    return op_Stack[oSt_top--];
}

int prec(char c) {
    if (c == '*' || c == '/') {
        return 2;
    }
    if (c == '+' || c == '-') {
        return 1;
    }
    return 0;
}

int apply(int a, int b, char o) {
    if (o == '+') {
        return a + b;
    }
    if (o == '-') {
        return a - b;
    }
    if (o == '*') {
        return a * b;
    }
    if (o == '/') {
        if (b == 0) {
            printf("Division By Zero is NOT POSSIBLE !!! Sorry \n");
            return 0;
        }
        return a / b;
    }
    printf("This is an Invalid expression !!! Sorry\n");
    return 0;
}

int main() {
    char exp[1000];
    if (!fgets(exp, sizeof(exp), stdin)) {
        return 0;
    }

    int i = 0;
    while (exp[i]) {
        if (isspace(exp[i])) {
            i++;
            continue;
        }

        if (isdigit(exp[i])) {
            int num = 0;
            while (isdigit(exp[i])) {
                num=num * 10 + (exp[i] - '0');
                i++;
            }
            pushVal(num);
            continue;
        }

        if (exp[i] == '+' || exp[i] == '-' || exp[i] == '*' || exp[i] == '/') {
            while (oSt_top >= 0 && prec(op_Stack[oSt_top]) >= prec(exp[i])) {
                if (vSt_top < 1) {
                    printf("This is an Invalid expression !!! Sorry \n");
                    return 0;
                }
                int b = popVal();
                int a = popVal();
                char o = popOp();
                int r = apply(a, b, o);

                pushVal(r);
            }
            pushOp(exp[i]);
            i++;
            continue;
        }

        printf(" This is an Invalid expression !!! Sorry \n");
        return 0;
    }

    while (oSt_top >= 0) {
        if (vSt_top < 1) {
            printf("This is an Invalid expression !!! Sorry \n");
            return 0;
        }
        int b = popVal();
        int a = popVal();
        char o = popOp();
        int r = apply(a, b, o);
        
        pushVal(r);
    }

    if (vSt_top == 0) {
        printf("%d\n", val_Stack[vSt_top]);
    } else {
        printf(" This is an Invalid expression !!! Sorry  \n");
    }

    return 0;
}
