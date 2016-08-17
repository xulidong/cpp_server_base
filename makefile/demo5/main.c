#include <stdio.h>
#include "add.h"
#include "sub.h"

int main() {
    int a = 2;
    int b = 1;
    
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d - %d = %d\n", a, b, sub(a, b));

    return 0;
}
