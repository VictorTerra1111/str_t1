/*
    Title: Least Slack Time first Simulator
    Version: v2.5
    Author: Joao Victor T. P,
    Date: 01/10/2026
    Definicao: LST da mais prioridade para quem tem menos slack time (folga), ou seja, 
    Tslack = deadline - tempo_atual - computacao_restante

    Simula um escalonador de tempo real que utilize politica LST
*/

#include <stdio.h>
#include <ctype.h>

#define MAX_TAREFAS 26
#define MAX_VALOR 2048
#ifndef DEBUG
#define DEBUG 0
#endif

// INICIO: structs necessarias
typedef struct Tarefa_t {
    unsigned c;         // computacao
    unsigned p;         // periodo
    unsigned d;         // deadline
} Tarefa_t;

typedef struct Instancia_t{
    int comp_restante;  // computacao restante
    int id;             // id da tarefa
    int deadline;       // deadline
} Instancia_t;
// FIM: structs necessarias

// INICIO: funcoes auxiliares
int ler_entradas(int *n, int *t, Tarefa_t tarefas[]) {
    while (1) {
        
        // leitura inicial dos parametros: quantidade de tarefas e tempo de simulacao
        // INICIO: verificacoes de entrada invalida
        if (DEBUG) printf("[DEBUG] Informe a quantidade de tarefas (n) e o tempo de simulacao (t), ou 0 0 para encerrar: ");
        if (scanf("%d %d", n, t) != 2) {
            if (DEBUG) printf("ERROR #01: parametros invalidos\n");
            return 0;
        }

        if ((*n == 0) || (*t == 0)) {
            if (DEBUG) printf("NOT ERROR #02: valores fornecidos iguais a zero. Terminando...\n");
            return 0;
        }

        if ((*n < 1) || (*n > MAX_TAREFAS)) {
            if (DEBUG) printf("ERROR #03: Quantidade invalida de tarefas. Valor precisa estar entre 1 e %d. N fornecido: %d\n", MAX_TAREFAS, *n);
            continue;
        }

        if ((*t < 1) || (*t > MAX_VALOR)) {
            if (DEBUG) printf("ERROR #04: Tempo de simulacao invalido. Valor precisa estar entre 1 e %d. T fornecido: %d\n", MAX_VALOR, *t);
            continue;
        }
        // FIM: verificacoes de entrada invalida
        // INICIO: entrada de especificacao de tarefas
        for (int iterador = 0; iterador < *n; iterador++) {
            // INICIO: verificacoes de entrada invalida
            if (DEBUG) printf("[DEBUG] Informe c, p e d da tarefa %d: ", iterador + 1);
            if (scanf("%u %u %u", &tarefas[iterador].c, &tarefas[iterador].p, &tarefas[iterador].d) != 3) {
                if (DEBUG) printf("Quantidade de parametros invalida. Precisa passar tempo de computacao, periodo e deadline\n");
                return 0;
            }
            if ((tarefas[iterador].c < 1) || (tarefas[iterador].c > MAX_VALOR)) {
                if (DEBUG) printf("Tempo de computacao invalido para tarefa %d. Valor precisa estar entre 1 e %d. C fornecido: %u\n", iterador + 1, MAX_VALOR, tarefas[iterador].c);
                return 0;
            }

            if ((tarefas[iterador].p < 1) || (tarefas[iterador].p > MAX_VALOR)) {
                if (DEBUG) printf("Periodo invalido para tarefa %d. Valor precisa estar entre 1 e %d. P fornecido: %u\n", iterador + 1, MAX_VALOR, tarefas[iterador].p);
                return 0;
            }

            if ((tarefas[iterador].d < 1) || (tarefas[iterador].d > MAX_VALOR)) {
                if (DEBUG) printf("Deadline invalido para tarefa %d. Valor precisa estar entre 1 e %d. D fornecido: %u\n", iterador + 1, MAX_VALOR, tarefas[iterador].d);
                return 0;
            }
            // FIM: verificacoes de entrada invalida

        }
        // FIM: entrada de especificacao de tarefas

        return 1;
    }
}

int calcula_slack_time(Instancia_t *instancia, int t_atual) {
    return (instancia->deadline - t_atual) - instancia->comp_restante;   
}

void monta_gantt(char gantt[], unsigned *gantt_pos, int t_prio, int deadline_atrasado) {
    // utiliza posicao da tabela ASCII para montar o diagrama
    char nome_tarefa;

    if (t_prio == -1) {
        gantt[(*gantt_pos)++] = '.';
    }
    else {
        nome_tarefa = t_prio + 65;                          // 65: posicao do A

        if (deadline_atrasado) nome_tarefa = nome_tarefa + 32;           // 32: deslocamento do A -> a

        gantt[(*gantt_pos)++] = nome_tarefa;
    }
}

// FIM: funcoes auxiliares 

// INICIO: main
int main() {
    // INICIO: declaracao de variaveis
    int n, t;                                                              // n: numero de tarefas, t: tempo de simulacao

    int inicio[MAX_TAREFAS], fim[MAX_TAREFAS], exec_inst[MAX_TAREFAS];     // indices das filas ready
    int deadline_atrasado[MAX_TAREFAS];
    int slack_tarefa[MAX_TAREFAS];
    
    int id_ready, inst_atual, t_prio, inst_exec, slack, least_slack, t_atual, iterador;

    unsigned gantt_pos, preempcoes, troca_de_contexto;

    char gantt[MAX_VALOR + 1];

    Tarefa_t tarefas[MAX_TAREFAS];

    Instancia_t ready[MAX_TAREFAS][MAX_VALOR + 1];
    Instancia_t inst_nova, *job;

    // FIM: declaracao de variaveis

    if (DEBUG) {
        printf("[DEBUG] Codigo: Least Slack Time first Simulator (LST)\n");
        printf("[DEBUG] Autor: Joao Victor T. P\n");
    }

    // INICIO: laco principal
    while (ler_entradas(&n, &t, tarefas)) {
        // passos do escalonador enumerados

        // 1) inicializa instancias
        gantt_pos = 0;
        preempcoes = 0;
        troca_de_contexto = 0;

        // ALTERAR
        id_ready = -99;                  // id_ready nao pode ser -1, pois seria processo IDLE
        inst_atual = -1;                // instancia atual nenhuma, instancia 0 seria a primeira

        // inicializacao das tarefas
        for (iterador = 0; iterador < MAX_TAREFAS; iterador++) {
            inicio[iterador] = 0;
            fim[iterador] = 0;
            exec_inst[iterador] = 0;
            deadline_atrasado[iterador] = 0;
        }

        // 2) executa uma unidade de tempo
        for (t_atual = 0; t_atual <= t; t_atual++) {

            // 3) verifica quem chegou
            for (iterador = 0; iterador < n; iterador++) {

                // se tempo atual eh multiplo do periodo
                if (t_atual % tarefas[iterador].p == 0) {
                    inst_nova.comp_restante = tarefas[iterador].c;
                    inst_nova.id = exec_inst[iterador]++;
                    inst_nova.deadline = tarefas[iterador].d + t_atual;

                    ready[iterador][fim[iterador]] = inst_nova;
                    fim[iterador]++;
                }
            }

            // 4) verificao se perdeu o deadline
            for (iterador = 0; iterador < n; iterador++) {
                deadline_atrasado[iterador] = inicio[iterador] < fim[iterador] && t_atual >= ready[iterador][inicio[iterador]].deadline;
            }

            // 5) calculo do slack time
            for (iterador = 0; iterador < n; iterador++) {

                if (inicio[iterador] >= fim[iterador]) continue; // verificao para indice invalido

                job = &ready[iterador][inicio[iterador]];

                slack_tarefa[iterador] = calcula_slack_time(job, t_atual);
            }

            // 6) escolhe tarefa mais prioritaria
            least_slack = 0;
            t_prio = -1;

            for (iterador = 0; iterador < n; iterador++) {

                if (inicio[iterador] >= fim[iterador]) continue;

                slack = slack_tarefa[iterador];

                if (t_prio == -1 || slack < least_slack) {
                    t_prio = iterador;
                    least_slack = slack;
                }
            }

            if (t_prio >= 0) inst_exec = ready[t_prio][inicio[t_prio]].id;
            else inst_exec = -1;

            // 7) troca de contexto e preempcoes
            if ((t_prio != id_ready || inst_exec != inst_atual) && id_ready != -99) {

                troca_de_contexto++;

                if (id_ready == -1) {
                    preempcoes++; // preempcao do IDLE
                }
                else if ((ready[id_ready][inicio[id_ready]].id == inst_atual) && inicio[id_ready] < fim[id_ready]) {
                    preempcoes++;
                }
            }

            // 8) executa unidade de tempo e monta o gantt
            if (t_atual < t) {
                monta_gantt(gantt, &gantt_pos, t_prio, t_prio >= 0 && deadline_atrasado[t_prio]);

                if (t_prio != -1) {
                    job = &ready[t_prio][inicio[t_prio]];
                    job->comp_restante--;

                    // 9) se a instancia atual da tarefa conseguiu terminar
                    if (job->comp_restante == 0) inicio[t_prio]++;
                }
            }

            // 10) guarda para a proxima unidade de tempo
            id_ready = t_prio;
            inst_atual = inst_exec;
        }

        // INICIO: impressao do Diagrama de Gantt
        gantt[gantt_pos] = '\0';

        if (DEBUG) {
            printf("[DEBUG] Gantt: %s\n", gantt);
            printf("[DEBUG] Preempcoes: %u\n", preempcoes);
            printf("[DEBUG] Trocas de contexto: %u\n", troca_de_contexto);
        }

        printf("%s\n", gantt);
        printf("%u %u\n\n", troca_de_contexto, preempcoes);
        // FIM: impressao do Diagrama de Gantt

    }
    // FIM: laco principal

    return 0;
}
// FIM: main