#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

/*
Code: Least Slack Time first
Version: v0.4 // modelo PROUD.SHAME
Author: J. Victor T. P
Date: 10/09/2026
Definicao: LST da mais prioridade para quem tem menos slack time (folga), ou seja, 
computacao_restante = computacao - tempo_atual + tempo_chegada 
Tslack = deadline - tempo_atual - computacao_restante

*/

/*
TO DO:
Decidir o que fazer quando receber valores invalidos de n, t e das tarefas
Verificar se alguma tarefa perdeu deadline
*/

#define DEBUG 0
#define MAX_TAREFAS 26
#define MAX_VALOR 2048
#define MAX_INSTANCIAS 64

struct Tarefa_t {
    char name;
    int c; // tempo de computacao
    int p; // periodo
    int d; // deadline
};

struct Instancia_t {
    char name;
    int slack; // slack time
    int c_falt; // tempo de computacao que ainda falta executar
    int t_cheg;
    int deadline;
    int pronta;
};

struct Escalonamento_t{
    int tr; // numero de trocas
    int pr; // numero de preempcoes
    char gantt[MAX_VALOR + 1]; // diagrama de gannt simplificado daquele escalonamento
};

int ler_entradas(int *n, int *t, struct Tarefa_t tarefas[]) {
    while (1) {
        if (scanf("%d %d", n, t) != 2) {
            if(DEBUG) printf("Quantidade de parametros invalida. Precisa passar numero de tarefas e tempo de simulacao\n");
            return 0;
        }

        if ((*n == 0) || (*t == 0)) {
            if(DEBUG) printf("valor de n ou t fornecidos estao zerados. Terminando looping de entradas\n");
            return 0;
        }

        if ((*n < 1) || (*n > MAX_TAREFAS)) {
            if(DEBUG) printf("Numero invalido de tarefas. Valor precisa estar entre 1 e %d. N fornecido: %d\n", MAX_TAREFAS, *n);
            continue;
        }

        if ((*t < 1) || (*t > MAX_VALOR)) {
            if(DEBUG) printf("Tempo de simulacao invalido. Valor precisa estar entre 1 e %d. T fornecido: %d\n", MAX_VALOR, *t);
            continue;
        }

        for (int i = 0; i < *n; i++) {
            tarefas[i].name = 0x41 + i; // associa caracter ao nome da tarefa
            if (scanf("%d %d %d", &tarefas[i].c, &tarefas[i].p, &tarefas[i].d) != 3) {
                if(DEBUG) printf("Quantidade de parametros invalida. Precisa passar tempo de computacao, periodo e deadline\n");
                return 0;
            }

            if ((tarefas[i].c < 1) || (tarefas[i].c > MAX_VALOR)) {
                if(DEBUG) printf("Tempo de computacao invalido para tarefa %d. Valor precisa estar entre 1 e %d. C fornecido: %d\n", i + 1, MAX_VALOR, tarefas[i].c);
                continue;
            }

            if ((tarefas[i].p < 1) || (tarefas[i].p > MAX_VALOR)) {
                if(DEBUG) printf("Periodo invalido para tarefa %d. Valor precisa estar entre 1 e %d. P fornecido: %d\n", i + 1, MAX_VALOR, tarefas[i].p);
                continue;
            }

            if ((tarefas[i].d < 1) || (tarefas[i].d > MAX_VALOR)) {
                if(DEBUG) printf("Deadline invalido para tarefa %d. Valor precisa estar entre 1 e %d. D fornecido: %d\n", i + 1, MAX_VALOR, tarefas[i].d);
                continue;
            }
        }

        return 1;
    }
}

int calcula_slack_time(struct Tarefa_t tarefas[], struct Instancia_t instancias[], int *n, int *t_atual){
    // computacao_restante = computacao - tempo_atual + tempo_chegada 
    // Tslack = deadline - tempo_atual - computacao_restante

    int tarefa_mais_prioritaria = -1; // posicao da tarefa mais prioritaria
    int menor_slack = INT_MAX; // posicao da tarefa com o menor slack

    for(int i = 0; i < *n; i++){
        if(instancias[i].pronta && instancias[i].c_falt > 0) {
            instancias[i].slack = instancias[i].deadline - *t_atual - instancias[i].c_falt;

            if(instancias[i].slack < menor_slack) {
                menor_slack = instancias[i].slack;
                tarefa_mais_prioritaria = i;
            }

            if(DEBUG) printf("SLACK TIME DA TAREFA %c: %d\n", tarefas[i].name, instancias[i].slack);
        }
    }

    return tarefa_mais_prioritaria;
}

int main(void) {
    // declaracao de variaveis

    int n = 0, t = 0;                                                   // numero de tarefas e tempo de simulacao
    int exec = 0;                                                       // numero de execucoes do algoritmo em paralelo

    struct Tarefa_t tarefas[MAX_TAREFAS];                               // inicializa um vetor de tarefas
    struct Escalonamento_t escalonamentos[MAX_INSTANCIAS];              // resultado dos conjuntos de tarefas executados

    // fim da declaracao de variaveis

    // inicio chamada para loop de leitura das entradas

    while(ler_entradas(&n, &t, tarefas)) {
        
        // inicializacao de variaveis
        
        escalonamentos[exec].tr = 0;
        escalonamentos[exec].pr = 0;
        escalonamentos[exec].gantt[t] = '\0';

        struct Instancia_t ready[MAX_TAREFAS];

        for(int i = 0; i < n; i++) {
            ready[i].name = tarefas[i].name;
            ready[i].slack = 0;
            ready[i].c_falt = 0;
            ready[i].t_cheg = 0;
            ready[i].deadline = 0;
            ready[i].pronta = 0;
        }

        int t_atual = 0;
        int tarefa_mais_prioritaria = -1;                               // posicao da tarefa mais prioritaria
        int tarefa_atual = -1;                                          // posicao da tarefa atual executando
        int tarefa_anterior = -1;

        // Passos do escalonador

        for(t_atual = 0; t_atual < t; t_atual++){

            // Verificar quais tarefas chegaram

            for(int i = 0; i < n; i++){
                if((t_atual % tarefas[i].p) == 0) {
                    ready[i].name = tarefas[i].name;
                    ready[i].c_falt = tarefas[i].c;
                    ready[i].t_cheg = t_atual;
                    ready[i].deadline = t_atual + tarefas[i].d;
                    ready[i].pronta = 1;
                }
            }

            // Verificar se alguma tarefa perdeu deadline

            // Calcular slack das tarefas prontas
            tarefa_mais_prioritaria = calcula_slack_time(tarefas, ready, &n, &t_atual);

            // Escolher tarefa de menor slack
            tarefa_atual = tarefa_mais_prioritaria;

            // Verificar se houve troca/preempção
            if(tarefa_atual != -1) {
                if(tarefa_anterior != tarefa_atual) {
                    escalonamentos[exec].tr++;

                    if(tarefa_anterior != -1 && ready[tarefa_anterior].c_falt > 0) {
                        escalonamentos[exec].pr++;
                    }
                }
            }

            // Executar 1 unidade de tempo
            if(tarefa_atual != -1) {

                // Decrementar computacao que resta
                ready[tarefa_atual].c_falt--;

                // Remover da fila de prontas
                if(ready[tarefa_atual].c_falt == 0) {
                    ready[tarefa_atual].pronta = 0;
                }

                // Registrar tarefa no Gantt
                escalonamentos[exec].gantt[t_atual] = ready[tarefa_atual].name;

                tarefa_anterior = tarefa_atual;
            }
            else {
                if(tarefa_anterior != -1) {
                    escalonamentos[exec].tr++;
                }

                escalonamentos[exec].gantt[t_atual] = '.';
                tarefa_anterior = -1;
            }
        }

        // Imprimir resultado desta simulacao
        printf("\n%s\n", escalonamentos[exec].gantt);
        printf("%d %d \n\n", escalonamentos[exec].tr, escalonamentos[exec].pr);

        // Proxima execucao
        exec++; 
    }


    return 0;
}