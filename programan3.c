#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEITURAS_POR_SENSOR 2000
#define MAX_ID_SENSOR 20
#define MAX_TIPO_DADO 10

typedef struct {
    char id[MAX_ID_SENSOR];
    char tipo[MAX_TIPO_DADO]; 
} Sensor;

const char *strings_aleatorias[] = {"OK", "FAIL", "ON", "OFF", "IDLE"};
#define TOTAL_STRINGS (sizeof(strings_aleatorias)/sizeof(strings_aleatorias[0]))

unsigned long long criar_timestamp_completo(int dia, int mes, int ano, int hora, int minuto, int segundo) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = ano - 1900;
    timeinfo.tm_mon  = mes - 1;
    timeinfo.tm_mday = dia;
    timeinfo.tm_hour = hora;
    timeinfo.tm_min  = minuto;
    timeinfo.tm_sec  = segundo;
    return (unsigned long long)mktime(&timeinfo);
}

unsigned long long gerar_timestamp_aleatorio(unsigned long long inicio, unsigned long long fim) {
    unsigned long long intervalo = fim - inicio;
    unsigned long long offset = ((unsigned long long)rand() << 32 | rand()) % (intervalo + 1);
    return inicio + offset;
}

float gerar_valor_float(const char *sensor_id) {
    float min, max;
    if (strcmp(sensor_id, "TEMP") == 0)       { min = 15.0; max = 40.0; }
    else if (strcmp(sensor_id, "PRES") == 0)  { min = 980.0; max = 1030.0; }
    else if (strcmp(sensor_id, "VIBR") == 0)  { min = 0.0; max = 10.0; }
    else if (strcmp(sensor_id, "UMID") == 0)  { min = 30.0; max = 90.0; }
    else if (strcmp(sensor_id, "FLUX") == 0)  { min = 100.0; max = 500.0; }
    else { min = 0.0; max = 100.0; }

    return min + ((float)rand() / RAND_MAX) * (max - min);
}

int gerar_valor_int() {
    return rand() % 1000;
}

int gerar_valor_bool() {
    return rand() % 2; 
}

const char* gerar_valor_string() {
    return strings_aleatorias[rand() % TOTAL_STRINGS];
}

int main(int argc, char *argv[]) {
    if (argc < 15) {
        fprintf(stderr, "Uso: %s <dd_i> <mm_i> <aaaa_i> <hh_i> <min_i> <ss_i> "
                        "<dd_f> <mm_f> <aaaa_f> <hh_f> <min_f> <ss_f> "
                        "<nome_arquivo> <sensor1:tipo1> [sensor2:tipo2] ...\n", argv[0]);
        return 1;
    }

    int dia_i = atoi(argv[1]),  mes_i = atoi(argv[2]),  ano_i = atoi(argv[3]);
    int hora_i = atoi(argv[4]), min_i = atoi(argv[5]), seg_i = atoi(argv[6]);
    int dia_f = atoi(argv[7]),  mes_f = atoi(argv[8]),  ano_f = atoi(argv[9]);
    int hora_f = atoi(argv[10]), min_f = atoi(argv[11]), seg_f = atoi(argv[12]);

    char *nome_arquivo = argv[13];
    int num_sensores = argc - 14;

    Sensor *sensores = malloc(num_sensores * sizeof(Sensor));
    if (!sensores) {
        fprintf(stderr, "Erro de memória.\n");
        return 1;
    }

    
    for (int i = 0; i < num_sensores; i++) {
        char *entrada = argv[14 + i];
        char *token = strtok(entrada, ":");
        if (!token) {
            fprintf(stderr, "Erro ao processar sensor: %s\n", entrada);
            free(sensores);
            return 1;
        }
        strncpy(sensores[i].id, token, MAX_ID_SENSOR);

        token = strtok(NULL, ":");
        if (!token) {
            fprintf(stderr, "Tipo de dado ausente para sensor: %s\n", sensores[i].id);
            free(sensores);
            return 1;
        }
        strncpy(sensores[i].tipo, token, MAX_TIPO_DADO);
    }

    srand((unsigned int)time(NULL));

    unsigned long long inicio = criar_timestamp_completo(dia_i, mes_i, ano_i, hora_i, min_i, seg_i);
    unsigned long long fim    = criar_timestamp_completo(dia_f, mes_f, ano_f, hora_f, min_f, seg_f);

    if (fim <= inicio) {
        fprintf(stderr, "Erro: intervalo de tempo inválido.\n");
        free(sensores);
        return 1;
    }

    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        perror("Erro ao abrir arquivo");
        free(sensores);
        return 1;
    }

    printf("Gerando %d leituras por sensor (%d sensores)...\n", LEITURAS_POR_SENSOR, num_sensores);

    for (int i = 0; i < num_sensores; i++) {
        printf("Processando sensor: %s (%s)\n", sensores[i].id, sensores[i].tipo);

        for (int j = 0; j < LEITURAS_POR_SENSOR; j++) {
            unsigned long long timestamp = gerar_timestamp_aleatorio(inicio, fim);

            if (strcmp(sensores[i].tipo, "float") == 0) {
                float valor = gerar_valor_float(sensores[i].id);
                fprintf(arquivo, "%llu %s %.2f\n", timestamp, sensores[i].id, valor);
            } else if (strcmp(sensores[i].tipo, "int") == 0) {
                int valor = gerar_valor_int();
                fprintf(arquivo, "%llu %s %d\n", timestamp, sensores[i].id, valor);
            } else if (strcmp(sensores[i].tipo, "bool") == 0) {
                int valor = gerar_valor_bool();
                fprintf(arquivo, "%llu %s %d\n", timestamp, sensores[i].id, valor);
            } else if (strcmp(sensores[i].tipo, "string") == 0) {
                const char *valor = gerar_valor_string();
                fprintf(arquivo, "%llu %s %s\n", timestamp, sensores[i].id, valor);
            } else {
                fprintf(stderr, "Tipo de dado '%s' não suportado para sensor %s.\n", sensores[i].tipo, sensores[i].id);
                fclose(arquivo);
                free(sensores);
                return 1;
            }
        }
    }

    fclose(arquivo);
    free(sensores);

    printf("Arquivo '%s' gerado com sucesso.\n", nome_arquivo);
    return 0;
}
