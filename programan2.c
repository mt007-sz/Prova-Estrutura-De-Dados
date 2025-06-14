#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_SENSOR_ID 10
#define BUFFER_DATA_HORA 30

typedef struct {
    unsigned long long timestamp;
    char sensor_id[MAX_SENSOR_ID];
    float valor;
} Leitura;

unsigned long long converter_para_timestamp(int dia, int mes, int ano, int hora, int minuto, int segundo) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = ano - 1900;
    timeinfo.tm_mon = mes - 1;
    timeinfo.tm_mday = dia;
    timeinfo.tm_hour = hora;
    timeinfo.tm_min = minuto;
    timeinfo.tm_sec = segundo;
    return (unsigned long long)mktime(&timeinfo);
}

int busca_binaria(Leitura *leituras, int inicio, int fim, unsigned long long timestamp_alvo) {
    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;
        if (leituras[meio].timestamp == timestamp_alvo) return meio;
        if (leituras[meio].timestamp > timestamp_alvo) fim = meio - 1;
        else inicio = meio + 1;
    }
    return -1;
}

int encontrar_leitura_mais_proxima(Leitura *leituras, int num_leituras, unsigned long long timestamp_alvo) {
    int indice = busca_binaria(leituras, 0, num_leituras - 1, timestamp_alvo);
    if (indice != -1) return indice;

    int mais_proximo = 0;
    unsigned long long menor_diferenca = ULLONG_MAX;

    for (int i = 0; i < num_leituras; i++) {
        unsigned long long diferenca = (leituras[i].timestamp > timestamp_alvo)
            ? leituras[i].timestamp - timestamp_alvo
            : timestamp_alvo - leituras[i].timestamp;

        if (diferenca < menor_diferenca) {
            menor_diferenca = diferenca;
            mais_proximo = i;
        }
    }

    return mais_proximo;
}

void converter_para_data_hora(unsigned long long timestamp, char *buffer) {
    time_t tempo = (time_t)timestamp;
    struct tm *timeinfo = localtime(&tempo);
    strftime(buffer, BUFFER_DATA_HORA, "%d/%m/%Y %H:%M:%S", timeinfo);
}

int main() {
    char nome_sensor[MAX_SENSOR_ID];
    int dia, mes, ano, hora, minuto, segundo;

    printf("Digite qual sensor voce quer verificar (ex: TEMP):");   //
    scanf("%s", nome_sensor);

    printf("Informe o dia, mes, ano e horario (no formato dd mm aaaa hh mm ss):");
    scanf("%d %d %d %d %d %d", &dia, &mes, &ano, &hora, &minuto, &segundo);

    unsigned long long timestamp_alvo = converter_para_timestamp(dia, mes, ano, hora, minuto, segundo);

    char nome_arquivo[20];
    snprintf(nome_arquivo, sizeof(nome_arquivo), "%s.txt", nome_sensor);

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro: Arquivo %s nao encontrado!\n", nome_arquivo);
        return 1;
    }

    Leitura *leituras = NULL;
    int num_leituras = 0, capacidade = 100;
    leituras = malloc(capacidade * sizeof(Leitura));
    if (!leituras) {
        fprintf(stderr, "Erro: Mem\u00f3ria insuficiente!\n");
        fclose(arquivo);
        return 1;
    }

    printf("Abrindo arquivo de dados: %s\n", nome_arquivo);

    while (fscanf(arquivo, "%llu %s %f", &leituras[num_leituras].timestamp,
                  leituras[num_leituras].sensor_id, &leituras[num_leituras].valor) == 3) {
        num_leituras++;
        if (num_leituras >= capacidade) {
            capacidade += 100;
            Leitura *temp = realloc(leituras, capacidade * sizeof(Leitura));
            if (!temp) {
                fprintf(stderr, "Erro: Mem\u00f3ria insuficiente na realoca\u00e7\u00e3o!\n");
                free(leituras);
                fclose(arquivo);
                return 1;
            }
            leituras = temp;
        }
    }
    fclose(arquivo);

    printf("Dados carregados com sucesso. Total de leituras encontradas: %d\n", num_leituras);

    int indice = encontrar_leitura_mais_proxima(leituras, num_leituras, timestamp_alvo);

    char data_hora_alvo[BUFFER_DATA_HORA];
    char data_hora_encontrada[BUFFER_DATA_HORA];
    converter_para_data_hora(timestamp_alvo, data_hora_alvo);
    converter_para_data_hora(leituras[indice].timestamp, data_hora_encontrada);

    printf("\nResultado da consulta:\n");
    printf("Data/hora solicitada: %s\n", data_hora_alvo);
    printf("Leitura mais proxima:\n");
    printf("  Sensor: %s\n", leituras[indice].sensor_id);
    printf("  Data/hora: %s\n", data_hora_encontrada);
    printf("  Valor: %.2f\n", leituras[indice].valor);
    printf("  Diferenca: %llu segundos\n",
           (leituras[indice].timestamp > timestamp_alvo)
               ? leituras[indice].timestamp - timestamp_alvo
               : timestamp_alvo - leituras[indice].timestamp);

    free(leituras);
    return 0;
}
