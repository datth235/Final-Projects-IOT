#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
    Bai 1:
    - Neu ten chuong trinh chua "lower" thi doi chu HOA -> thuong
    - Neu ten chuong trinh chua "upper" thi doi chu thuong -> HOA
    - Doc tung ky tu tu ban phim va in ra man hinh
*/

int main(int argc, char *argv[]) {
    int c;   // bien luu tung ky tu doc vao

    // Kiem tra co ten chuong trinh hay khong
    if (argc <= 0) {
        return 1;
    }

    // Neu ten chuong trinh co chua chuoi "lower"
    if (strstr(argv[0], "lower") != NULL) {
        // Doc tung ky tu cho den het du lieu
        while ((c = getchar()) != EOF) {
            // Chuyen thanh chu thuong roi in ra
            putchar(tolower(c));
        }
    }
    // Neu ten chuong trinh co chua chuoi "upper"
    else if (strstr(argv[0], "upper") != NULL) {
        // Doc tung ky tu cho den het du lieu
        while ((c = getchar()) != EOF) {
            // Chuyen thanh chu hoa roi in ra
            putchar(toupper(c));
        }
    }
    else {
        // Bao loi neu ten chuong trinh khong dung yeu cau
        printf("Hay doi ten chuong trinh thanh lower hoac upper\n");
        printf("Vi du: lower.exe hoac upper.exe\n");
    }

    return 0;
}