#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAM_BUF 1024
#define INTERVALO_ATUALIZACAO 5 

void mostra_process() {
    FILE *arquivo;
    char buf[TAM_BUF];


    printf("Nova atualizacao de processos:\n");
    printf(" PID  | Nome Process      | CPU limite | CPU Utilizacao | Utilizacao IO | Utilizacao Memoria | Tipo Processo \n");
    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("PROCESSOS DE USUÁRIO:\n");
    arquivo = popen("ps -eo pid,comm,%cpu,%mem,c --sort=-%cpu |head -n 11", "r");
    if (arquivo == NULL) {
        exit(1);
    }

    fgets(buf, TAM_BUF, arquivo); 
    double total_cpu_util = 0, total_io_util = 0, total_mem_util = 0;
    while (fgets(buf, TAM_BUF, arquivo) != NULL) {
        char *pid_str = strtok(buf, " ");
        char *nome_processo = strtok(NULL, " ");
        char *comando = strtok(NULL, " ");
        char *cpu_str = strtok(NULL, " ");
        char *mem_str = strtok(NULL, " ");
        int pid = atoi(pid_str);
        double cpu = strtod(cpu_str, NULL);
        double mem = strtod(mem_str, NULL);

        char caminho_arquivo_io[TAM_BUF];
        sprintf(caminho_arquivo_io, "/proc/%d/io", pid);
        FILE *arquivo_io = fopen(caminho_arquivo_io, "r");
        if (arquivo_io == NULL) {
            continue;
        }
        char *linha = NULL;
        size_t len = 0;
        double bytes_lidos = 0, bytes_escritos = 0;
        while (getline(&linha, &len, arquivo_io) != -1) {
            if (strncmp(linha, "read_bytes:", 11) == 0) {
                bytes_lidos = strtod(linha + 11, NULL);
            } else if (strncmp(linha, "write_bytes:", 12) == 0) {
                bytes_escritos = strtod(linha + 12, NULL);
            }
        }
        free(linha);
        fclose(arquivo_io);

        double utilizacao_io = (bytes_lidos + bytes_escritos) / (cpu * 1000000) * 100;
        char *tipo_str = (comando[0] == '/') ? "system" : "user";
        char *cpu_bound = (utilizacao_io > 10) ? "I/O" : "CPU";
       
        printf(" %4d | %-18s | %-9s | %15.3f | %14.3f | %18.3lf | %-12s\n", pid, nome_processo, cpu_bound, cpu, utilizacao_io, mem, tipo_str);

        total_cpu_util += cpu;
        total_io_util += utilizacao_io;
        total_mem_util += mem;
    }

    pclose(arquivo);
    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("PROCESSOS DO SISTEMA\n");
    arquivo = popen("ps -eo pid,comm,%cpu,%mem,c --sort=-%cpu ", "r");
    if (arquivo == NULL) {
        exit(1);
    }

    fgets(buf, TAM_BUF, arquivo); 
    while (fgets(buf, TAM_BUF, arquivo) != NULL) {
        char *pid_str = strtok(buf, " ");
        char *nome_processo = strtok(NULL, " ");
        char *comando = strtok(NULL, " ");
        char *cpu_str = strtok(NULL, " ");
        char *mem_str = strtok(NULL, " ");
        int pid = atoi(pid_str);
        double cpu = strtod(cpu_str, NULL);
        double mem = strtod(mem_str, NULL);

        char caminho_arquivo_io[TAM_BUF];
        sprintf(caminho_arquivo_io, "/proc/%d/io", pid);
        FILE *arquivo_io = fopen(caminho_arquivo_io, "r");
        if (arquivo_io == NULL) {
            continue;
        }
        char *linha = NULL;
        size_t len = 0;
        double bytes_lidos = 0, bytes_escritos = 0;
        while (getline(&linha, &len, arquivo_io) != -1) {
            if (strncmp(linha, "read_bytes:", 11) == 0) {
                bytes_lidos = strtod(linha + 11, NULL);
            } else if (strncmp(linha, "write_bytes:", 12) == 0) {
                bytes_escritos = strtod(linha + 12, NULL);
            }
        }
        free(linha);
        fclose(arquivo_io);

        double utilizacao_io = (bytes_lidos + bytes_escritos) / (cpu * 1000000) * 100;
        char *tipo_str = (comando[0] == '/') ? "user" : "system";
        char *cpu_bound = (utilizacao_io > 10) ? "I/O" : "CPU";
        printf(" %4d | %-18s | %-9s | %15.3f | %14.3f | %18.3lf | %-12s\n", pid, nome_processo, cpu_bound, cpu, utilizacao_io, mem, tipo_str);

        total_cpu_util += cpu;
        total_io_util += utilizacao_io;
        total_mem_util += mem;
    }

    pclose(arquivo);

    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf(" Total Utilizacao | %15.3f | %14.3f | %18.3lf |\n", total_cpu_util, total_io_util, total_mem_util);
    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("\n");
    printf("Vai atualizar em %d segundos\n", INTERVALO_ATUALIZACAO);
    printf("\n");
}

int main(void) {
    while (1) {
        mostra_process();
        sleep(INTERVALO_ATUALIZACAO);
    }
    return 0;
}
