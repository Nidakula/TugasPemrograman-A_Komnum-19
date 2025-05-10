#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_DATA 1000     // Maksimum baris data
#define DEGREE   3        // Derajat polinomial

// Fungsi untuk membaca CSV
int read_csv(const char *fname, double x[], double y1[], double y2[]) {
    FILE *fp = fopen(fname, "r");
    if (!fp) {
        perror("Tidak bisa membuka file");
        return -1;
    }
    char line[256];
    int n = 0;
    // Lewati header
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return 0;
    }
    // Baca setiap baris
    while (fgets(line, sizeof(line), fp)) {
        int year;
        double pct;
        long pop;
        if (sscanf(line, "%d,%lf,%ld", &year, &pct, &pop) == 3) {
            x[n]  = year;
            y1[n] = pct;
            y2[n] = (double)pop;
            n++;
            if (n >= MAX_DATA) break;
        }
    }
    fclose(fp);
    return n;
}

// Eliminasi Gauss untuk menyelesaikan A * c = b
void gaussian_elim(int m, double A[][DEGREE+2], double c[]) {
    int i, j, k;
    for (i = 0; i < m; i++) {
        // Pivoting sederhana: cari baris terbesar
        int piv = i;
        for (j = i+1; j < m; j++) {
            if (fabs(A[j][i]) > fabs(A[piv][i])) piv = j;
        }
        // Tukar baris
        if (piv != i) {
            for (k = i; k <= m; k++)
                { double tmp = A[i][k]; A[i][k] = A[piv][k]; A[piv][k] = tmp; }
        }
        // Normalisasi dan eliminasi
        double div = A[i][i];
        for (k = i; k <= m; k++) A[i][k] /= div;
        for (j = 0; j < m; j++) {
            if (j == i) continue;
            double factor = A[j][i];
            for (k = i; k <= m; k++)
                A[j][k] -= factor * A[i][k];
        }
    }
    // Ambil solusi
    for (i = 0; i < m; i++) c[i] = A[i][m];
}

// Fitting polinomial degree = DEGREE
void polyfit(int n, const double x[], const double y[], double coeffs[]) {
    int m = DEGREE + 1;
    // Matriks normal berukuran m x (m+1)
    double N[DEGREE+1][DEGREE+2];
    memset(N, 0, sizeof(N));

    // Hitung elemen matriks
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            double sum = 0;
            for (int k = 0; k < n; k++)
                sum += pow(x[k], i+j);
            N[i][j] = sum;
        }
        // vektor sebelah kanan
        double sumy = 0;
        for (int k = 0; k < n; k++)
            sumy += y[k] * pow(x[k], i);
        N[i][m] = sumy;
    }

    // Selesaikan dengan eliminasi Gauss
    gaussian_elim(m, N, coeffs);
}

int main() {
    double x[MAX_DATA], pct[MAX_DATA], pop[MAX_DATA];
    int n = read_csv("Data Tugas Pemrograman A.csv", x, pct, pop);
    if (n <= 0) {
        fprintf(stderr, "Gagal membaca data\n");
        return 1;
    }

    double c_pct[DEGREE+1], c_pop[DEGREE+1];

    // Fitting
    polyfit(n, x, pct, c_pct);
    polyfit(n, x, pop, c_pop);

    // Cetak hasil
    printf("a) Persentase Pengguna Internet:\n");
    printf("   y = ");
    for (int i = DEGREE; i >= 0; i--) {
        printf("%+.6e", c_pct[i]);
        if (i > 0) printf(" x^%d ", i);
    }
    printf("\n\n");

    printf("b) Pertumbuhan Populasi:\n");
    printf("   y = ");
    for (int i = DEGREE; i >= 0; i--) {
        printf("%+.6e", c_pop[i]);
        if (i > 0) printf(" x^%d ", i);
    }
    printf("\n");

    return 0;
}
