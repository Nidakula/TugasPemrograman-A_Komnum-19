#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 500

typedef struct {
    int year;
    double percentage;
    long population;
} Data;

int read_csv(const char *filename, Data data[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("File tidak bisa dibuka");
        return -1;
    }

    char line[1024];
    int i = 0;

    // Lewati header
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%lf,%ld", &data[i].year, &data[i].percentage, &data[i].population);
        i++;
    }

    fclose(file);
    return i;
}

double linear_interpolate(int x0, double y0, int x1, double y1, int x) {
    return y0 + (double)(x - x0) * (y1 - y0) / (x1 - x0);
}

long linear_interpolate_long(int x0, long y0, int x1, long y1, int x) {
    return y0 + (long)((double)(x - x0) * (y1 - y0) / (x1 - x0));
}

void estimate(Data data[], int size, int target_years[], int target_size) {
    for (int i = 0; i < target_size; i++) {
        int year = target_years[i];
        for (int j = 0; j < size - 1; j++) {
            if (data[j].year <= year && data[j+1].year >= year) {
                long est_population = linear_interpolate_long(data[j].year, data[j].population,
                                                                data[j+1].year, data[j+1].population, year);
                double est_percentage = linear_interpolate(data[j].year, data[j].percentage,
                                                            data[j+1].year, data[j+1].percentage, year);
                printf("Tahun %d:\n", year);
                printf("  - Estimasi Jumlah Penduduk      : %ld\n", est_population);
                printf("  - Estimasi Persentase Internet  : %.2f%%\n\n", est_percentage);
                break;
            }
        }
    }
}

void write_with_predictions(const char *output_file, Data data[], int size, int target_years[], int target_size) {
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("Gagal membuka file output");
        return;
    }

    fprintf(out, "Year,Percentage,Population,Type\n");

    // Original data
    for (int i = 0; i < size; i++) {
        fprintf(out, "%d,%.2f,%ld,Actual\n", data[i].year, data[i].percentage, data[i].population);
    }

    // Predicted data
    for (int i = 0; i < target_size; i++) {
        int year = target_years[i];
        for (int j = 0; j < size - 1; j++) {
            if (data[j].year <= year && data[j+1].year >= year) {
                long est_population = linear_interpolate_long(data[j].year, data[j].population,
                                                                data[j+1].year, data[j+1].population, year);
                double est_percentage = linear_interpolate(data[j].year, data[j].percentage,
                                                            data[j+1].year, data[j+1].percentage, year);
                fprintf(out, "%d,%.2f,%ld,Predicted\n", year, est_percentage, est_population);
                break;
            }
        }
    }

    fclose(out);
}

void generate_python_plot_script(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Gagal membuat file Python");
        return;
    }

    fprintf(fp,
        "import matplotlib.pyplot as plt\n"
        "import csv\n\n"
        "years_actual, perc_actual, pop_actual = [], [], []\n"
        "years_pred, perc_pred, pop_pred = [], [], []\n\n"
        "with open('output_with_predictions.csv') as f:\n"
        "    next(f)  # Skip header\n"
        "    for row in csv.reader(f):\n"
        "        year, perc, pop, typ = row\n"
        "        year = int(year)\n"
        "        perc = float(perc)\n"
        "        pop = int(pop)\n"
        "        if typ == 'Actual':\n"
        "            years_actual.append(year)\n"
        "            perc_actual.append(perc)\n"
        "            pop_actual.append(pop)\n"
        "        else:\n"
        "            years_pred.append(year)\n"
        "            perc_pred.append(perc)\n"
        "            pop_pred.append(pop)\n\n"
        "# Create figure and twin axes\n"
        "fig, ax1 = plt.subplots(figsize=(10, 6))\n"
        "ax2 = ax1.twinx()\n\n"
        "# Plot Internet percentage on left y-axis\n"
        "ax1.plot(years_actual, perc_actual, 'bo-', label='Internet %% (Actual)')\n"
        "ax1.plot(years_pred, perc_pred, 'ro--', label='Internet %% (Predicted)')\n"
        "ax1.set_xlabel('Tahun')\n"
        "ax1.set_ylabel('Persentase Internet', color='b')\n"
        "ax1.tick_params(axis='y', labelcolor='b')\n\n"
        "# Plot Population on right y-axis\n"
        "ax2.plot(years_actual, pop_actual, 'g^-', label='Populasi (Actual)')\n"
        "ax2.plot(years_pred, pop_pred, 'ms--', label='Populasi (Predicted)')\n"
        "ax2.set_ylabel('Populasi (juta)', color='g')\n"
        "ax2.tick_params(axis='y', labelcolor='g')\n\n"
        "# Combine legends from both axes\n"
        "lines1, labels1 = ax1.get_legend_handles_labels()\n"
        "lines2, labels2 = ax2.get_legend_handles_labels()\n"
        "ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left')\n\n"
        "plt.title('Prediksi dan Data Asli: Internet & Populasi')\n"
        "plt.grid(True)\n"
        "plt.tight_layout()\n"
        "plt.savefig('combined_plot.png')\n"
        "plt.show()\n"
    );

    fclose(fp);
}

int main() {
    Data data[MAX];
    int size = read_csv("Data Tugas Pemrograman A.csv", data);

    if (size == -1) return 1;

    int target_years[] = {2005, 2006, 2015, 2016};
    int target_size = sizeof(target_years) / sizeof(target_years[0]);

    estimate(data, size, target_years, target_size);
    write_with_predictions("output_with_predictions.csv", data, size, target_years, target_size);
    generate_python_plot_script("plot.py");
    return 0;
}