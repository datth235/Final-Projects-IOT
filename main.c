#include <stdio.h>

int main() {
    int a, b,hieu, tong;
  printf("HELLO");
    printf("Nhap so thu nhat: ");
    scanf("%d", &a);

    printf("Nhap so thu hai: ");
    scanf("%d", &b);

    tong = a + b;
	hieu=a-b;
    printf("Tong = %d\n", tong);
	printf("Hieu= %d\n", hieu);
    return 0;
}
