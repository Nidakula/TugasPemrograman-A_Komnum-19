#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_DATA 100
#define MAX_LINE 1024
#define MAX_DEGREE 3

typedef struct {
    int year;
    double percentage_internet;
    double population;
} DataPoint;

int read_csv(const char* filename, DataPoint* data) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file\n");
        return 0;
    }

    char line[MAX_LINE];
    int count = 0;
    fgets(line, MAX_LINE, file); // Skip header
    
    while (fgets(line, MAX_LINE, file) && count < MAX_DATA) {
        char* token = strtok(line, ",");
        if (token) {
            data[count].year = atoi(token);
            token = strtok(NULL, ",");
            if (token) {
                data[count].percentage_internet = atof(token);
                token = strtok(NULL, ",");
                if (token) {
                    data[count].population = atof(token);
                    count++;
                }
            }
        }
    }
    fclose(file);
    return count;
}

void polynomial_regression(int n, double* x, double* y, int degree, double* coeffs) {
    int i, j, k;
    double X[2 * degree + 1];
    double B[degree + 1][degree + 2];

    for (i = 0; i <= 2 * degree; i++) {
        X[i] = 0;
        for (j = 0; j < n; j++)
            X[i] += pow(x[j], i);
    }

    for (i = 0; i <= degree; i++) {
        for (j = 0; j <= degree; j++) {
            B[i][j] = X[i + j];
        }
    }

    for (i = 0; i <= degree; i++) {
        B[i][degree + 1] = 0;
        for (j = 0; j < n; j++) {
            B[i][degree + 1] += pow(x[j], i) * y[j];
        }
    }

    for (i = 0; i <= degree; i++) {
        for (k = i + 1; k <= degree; k++) {
            double factor = B[k][i] / B[i][i];
            for (j = i; j <= degree + 1; j++) {
                B[k][j] -= factor * B[i][j];
            }
        }
    }

    for (i = degree; i >= 0; i--) {
        coeffs[i] = B[i][degree + 1];
        for (j = i + 1; j <= degree; j++) {
            coeffs[i] -= B[i][j] * coeffs[j];
        }
        coeffs[i] /= B[i][i];
    }
}

double evaluate_polynomial(double x, double* coeffs, int degree) {
    double result = 0;
    for (int i = 0; i <= degree; i++) {
        result += coeffs[i] * pow(x, i);
    }
    return result;
}

void print_polynomial(double* coeffs, int degree) {
    printf("y = ");
    int first_term = 1;
    for (int i = degree; i >= 0; i--) {
        if (coeffs[i] == 0) continue;

        if (coeffs[i] > 0 && !first_term) printf(" + ");
        else if (coeffs[i] < 0) printf(" - ");

        if (i == 0 || fabs(coeffs[i]) != 1) {
            printf("%.10g", fabs(coeffs[i]));
        }

        if (i > 0) {
            printf("x");
            if (i > 1) {
                printf("^%d", i);
            }
        }

        first_term = 0;
    }
    printf("\n");
}

void normalize_years(int n, double* x, double* years, double* min_year, double* scale) {
    *min_year = years[0];
    double max_year = years[0];

    for (int i = 1; i < n; i++) {
        if (years[i] < *min_year) *min_year = years[i];
        if (years[i] > max_year) max_year = years[i];
    }

    *scale = max_year - *min_year;
    if (*scale == 0) *scale = 1;

    for (int i = 0; i < n; i++) {
        x[i] = (years[i] - *min_year) / *scale;
    }
}

int main() {
    DataPoint data[MAX_DATA];
    int count = read_csv("Data Tugas Pemrograman A.csv", data);
    if (count == 0) {
        printf("No data read\n");
        return 1;
    }

    printf("Membaca %d baris data\n\n", count);

    double years[MAX_DATA], x_pop[MAX_DATA], y_pop[MAX_DATA];
    double x_internet[MAX_DATA], y_internet[MAX_DATA];
    int n_pop = 0, n_internet = 0;

    for (int i = 0; i < count; i++) {
        years[i] = data[i].year;

        x_pop[n_pop] = data[i].year;
        y_pop[n_pop] = data[i].population;
        n_pop++;

        if (data[i].year >= 1994) {
            x_internet[n_internet] = data[i].year;
            y_internet[n_internet] = data[i].percentage_internet;
            n_internet++;
        }
    }

    double min_year_pop, scale_pop;
    double min_year_internet, scale_internet;
    double x_pop_norm[MAX_DATA], x_internet_norm[MAX_DATA];

    normalize_years(n_pop, x_pop_norm, x_pop, &min_year_pop, &scale_pop);
    normalize_years(n_internet, x_internet_norm, x_internet, &min_year_internet, &scale_internet);

    double pop_coeffs[MAX_DEGREE + 1] = {0};
    polynomial_regression(n_pop, x_pop_norm, y_pop, MAX_DEGREE, pop_coeffs);
    printf("Persamaan polinomial untuk pertumbuhan populasi:\n");
    print_polynomial(pop_coeffs, MAX_DEGREE);

    double internet_coeffs[MAX_DEGREE + 1] = {0};
    polynomial_regression(n_internet, x_internet_norm, y_internet, MAX_DEGREE, internet_coeffs);
    printf("Persamaan polinomial untuk persentase pengguna internet:\n");
    print_polynomial(internet_coeffs, MAX_DEGREE);

    int years_to_predict[] = {2030, 2035};
    int num_years = sizeof(years_to_predict) / sizeof(years_to_predict[0]);

    printf("\n=== Hasil Prediksi ===\n");
    for (int i = 0; i < num_years; i++) {
        int year = years_to_predict[i];

        double x_pop_pred = (year - min_year_pop) / scale_pop;
        double x_internet_pred = (year - min_year_internet) / scale_internet;

        double pred_pop = evaluate_polynomial(x_pop_pred, pop_coeffs, MAX_DEGREE);
        double pred_internet = evaluate_polynomial(x_internet_pred, internet_coeffs, MAX_DEGREE);

        if (pred_internet > 100.0) pred_internet = 100.0;
        if (pred_internet < 0.0) pred_internet = 0.0;

        printf("Tahun %d:\n", year);
        printf("Jumlah Penduduk: %.0f\n", pred_pop);
        printf("Persentase Pengguna Internet: %.4f%%\n\n", pred_internet);
    }

    return 0;
}