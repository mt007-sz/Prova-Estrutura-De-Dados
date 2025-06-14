#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_SENSORES 5 
#define LEITURAS_POR_SENSOR 2000 

const char *SENSORES[NUM_SENSORES] = {"TEMP", "PRES", "VIBR", "UMID", "FLUX"}; 



unsigned long long criar_timestamp(int dia, int mes, int ano, int fim_do_dia) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = ano - 1900;
    timeinfo.tm_mon = mes - 1;
    timeinfo.tm_mday = dia;

    if (fim_do_dia) {
        timeinfo.tm_hour = 23;
        timeinfo.tm_min = 59;
        timeinfo.tm_sec = 59;
    }

    return (unsigned long long)mktime(&timeinfo);
}


float gerar_valor_aleatorio(const char *sensor) {
    float min, max;


    if (strcmp(sensor, "TEMP") == 0) {
        min = 15.0; max = 40.0;
    } else if (strcmp(sensor, "PRES") == 0) {
        min = 980.0; max = 1030.0;
    } else if (strcmp(sensor, "VIBR") == 0) {
        min = 0.0; max = 10.0;
    } else if (strcmp(sensor, "UMID") == 0) {
        min = 30.0; max = 90.0;
    } else if (strcmp(sensor, "FLUX") == 0) {
        min = 100.0; max = 500.0;
    } else {
        min = 0.0; max = 100.0;
    }

    return min + ((float)rand() / RAND_MAX) * (max - min);
}


unsigned long long gerar_timestamp_aleatorio(unsigned long long inicio, unsigned long long fim) {
    unsigned long long intervalo = fim - inicio;
     
    unsigned long long offset = ((unsigned long long)rand() << 32 | rand()) % (intervalo + 1);
    return inicio + offset;
}

int main() {
    int dia, mes, ano;
    char nome_arquivo[100];

    srand((unsigned int)time(NULL));

    // Entrada do usuário
    printf("Informe a data (dd mm aaaa): ");
    scanf("%d %d %d", &dia, &mes, &ano);

    printf("Qual sera o nome do arquivo onde os dados serao salvos? ");
    scanf("%s", nome_arquivo);

    // Gerar timestamps
    unsigned long long inicio = criar_timestamp(dia, mes, ano, 0);
    unsigned long long fim = criar_timestamp(dia, mes, ano, 1);

    if (fim <= inicio) {
        fprintf(stderr, "Erro: data invalida ou intervalo de tempo negativo.\n");
        return 1;
    }

    // Abrir arquivo
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        perror("Erro ao criar o arquivo");
        return 1;
    }

    printf("Iniciando a geracao de %d leituras para cada um dos %d sensores...", LEITURAS_POR_SENSOR, NUM_SENSORES);

    // Geração de leituras
    for (int i = 0; i < NUM_SENSORES; i++) {
        const char *sensor = SENSORES[i];
        printf("Processando sensor: %s...\n", sensor);

        for (int j = 0; j < LEITURAS_POR_SENSOR; j++) {
            unsigned long long timestamp = gerar_timestamp_aleatorio(inicio, fim);
            float valor = gerar_valor_aleatorio(sensor);

            fprintf(arquivo, "%llu %s %.2f\n", timestamp, sensor, valor);
        }
    }

    fclose(arquivo);

    printf("Os dados foram salvos no arquivo '%s' com exito.\n", nome_arquivo);
    printf("Leituras concluidas! Total: %d registros.\n", NUM_SENSORES * LEITURAS_POR_SENSOR);

    return 0;
}
