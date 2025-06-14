#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAM_SENSOR 10
#define TAM_STRING 17
#define INCREMENTO 1000

typedef enum { TIPO_INT, TIPO_BOOL, TIPO_FLOAT, TIPO_STRING } TipoDado;

typedef struct {
    unsigned long long timestamp;
    char sensor_id[TAM_SENSOR];
    TipoDado tipo;
    union {
        int inteiro;
        float racional;
        char booleano[6]; // "true" ou "false"
        char string[TAM_STRING];
    } valor;
} Leitura;

typedef struct {
    char sensor_id[TAM_SENSOR];
    TipoDado tipo;
} SensorInfo;

int comparar_leituras(const void *a, const void *b) {
    unsigned long long ts_a = ((Leitura *)a)->timestamp;
    unsigned long long ts_b = ((Leitura *)b)->timestamp;
    return (ts_a > ts_b) - (ts_a < ts_b);
}

TipoDado identificar_tipo(const char *str) {
    if (strcmp(str, "true") == 0 || strcmp(str, "false") == 0) return TIPO_BOOL;

    int ponto = 0, digito = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '.') ponto++;
        else if (isdigit(str[i])) digito++;
        else return TIPO_STRING;
    }

    if (ponto == 1) return TIPO_FLOAT;
    if (ponto == 0 && digito > 0) return TIPO_INT;

    return TIPO_STRING;
}

int sensor_ja_existe(SensorInfo *infos, int total, const char *sensor_id) {
    for (int i = 0; i < total; i++) {
        if (strcmp(infos[i].sensor_id, sensor_id) == 0)
            return i;
    }
    return -1;
}

void liberar_leituras(Leitura *leituras) {
    free(leituras);
}

void salvar_leituras_por_sensor(SensorInfo *infos, int num_sensores, Leitura *leituras, int total) {
    for (int i = 0; i < num_sensores; i++) {
        char nome_saida[50];
        sprintf(nome_saida, "%s.txt", infos[i].sensor_id);

        int count = 0;
        for (int j = 0; j < total; j++) {
            if (strcmp(leituras[j].sensor_id, infos[i].sensor_id) == 0) count++;
        }

        Leitura *buffer = malloc(count * sizeof(Leitura));
        if (!buffer) {
            fprintf(stderr, "Erro de memória para %s\n", infos[i].sensor_id);
            continue;
        }

        int idx = 0;
        for (int j = 0; j < total; j++) {
            if (strcmp(leituras[j].sensor_id, infos[i].sensor_id) == 0)
                buffer[idx++] = leituras[j];
        }

        qsort(buffer, count, sizeof(Leitura), comparar_leituras);

        FILE *saida = fopen(nome_saida, "w");
        if (!saida) {
            fprintf(stderr, "Erro ao criar %s\n", nome_saida);
            free(buffer);
            continue;
        }

        for (int j = 0; j < count; j++) {
            fprintf(saida, "%llu %s ", buffer[j].timestamp, buffer[j].sensor_id);
            switch (buffer[j].tipo) {
                case TIPO_INT:
                    fprintf(saida, "%d\n", buffer[j].valor.inteiro);
                    break;
                case TIPO_FLOAT:
                    fprintf(saida, "%.2f\n", buffer[j].valor.racional);
                    break;
                case TIPO_BOOL:
                    fprintf(saida, "%s\n", buffer[j].valor.booleano);
                    break;
                case TIPO_STRING:
                    fprintf(saida, "%s\n", buffer[j].valor.string);
                    break;
            }
        }

        fclose(saida);
        free(buffer);
        printf("Arquivo %s criado com sucesso.\n", nome_saida);
    }
}

int main() {
    FILE *arquivo;
    char nome_arquivo[100];
    Leitura *leituras = NULL;
    int total_leituras = 0, capacidade = 0;
    SensorInfo *infos = NULL;
    int num_sensores = 0;

    printf("Digite o nome do arquivo de entrada: ");
    scanf("%s", nome_arquivo);

    arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    unsigned long long timestamp;
    char sensor_id[TAM_SENSOR];
    char valor_str[TAM_STRING];

    while (fscanf(arquivo, "%llu %s %s", &timestamp, sensor_id, valor_str) == 3) {
        if (total_leituras >= capacidade) {
            capacidade += INCREMENTO;
            Leitura *temp = realloc(leituras, capacidade * sizeof(Leitura));
            if (!temp) {
                perror("Erro de memória");
                fclose(arquivo);
                free(leituras);
                free(infos);
                return 1;
            }
            leituras = temp;
        }

        int idx = sensor_ja_existe(infos, num_sensores, sensor_id);
        TipoDado tipo;
        if (idx == -1) {
            tipo = identificar_tipo(valor_str);
            infos = realloc(infos, (num_sensores + 1) * sizeof(SensorInfo));
            if (!infos) {
                perror("Erro de memória para infos");
                fclose(arquivo);
                free(leituras);
                return 1;
            }
            strcpy(infos[num_sensores].sensor_id, sensor_id);
            infos[num_sensores].tipo = tipo;
            idx = num_sensores++;
        } else {
            tipo = infos[idx].tipo;
        }

        leituras[total_leituras].timestamp = timestamp;
        strcpy(leituras[total_leituras].sensor_id, sensor_id);
        leituras[total_leituras].tipo = tipo;

        switch (tipo) {
            case TIPO_INT:
                leituras[total_leituras].valor.inteiro = atoi(valor_str);
                break;
            case TIPO_FLOAT:
                leituras[total_leituras].valor.racional = atof(valor_str);
                break;
            case TIPO_BOOL:
                strcpy(leituras[total_leituras].valor.booleano, valor_str);
                break;
            case TIPO_STRING:
                strncpy(leituras[total_leituras].valor.string, valor_str, TAM_STRING - 1);
                leituras[total_leituras].valor.string[TAM_STRING - 1] = '\0';
                break;
        }

        total_leituras++;
    }

    fclose(arquivo);

    salvar_leituras_por_sensor(infos, num_sensores, leituras, total_leituras);

    free(infos);
    liberar_leituras(leituras);

    printf("Processamento concluido com sucesso.\n");
    return 0;
}
