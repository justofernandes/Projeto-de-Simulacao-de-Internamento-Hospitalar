#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include "../include/paciente.h"
#include "../include/fila.h"
#include "../include/leito.h"
#include "../include/historico.h"

int main() {
    setlocale(LC_ALL, ""); //n ta funcionando, tentar corrigir dps

    Deque fila;
    Leitos leitos;
    PilhaHistorico historico;

    // ===== Inicializa tudo =====
    inicializar_fila(&fila);
    inicializar_leitos(&leitos);
    inicializar_pilha(&historico);
    inicializar_tabela();
    carregar_pacientes_csv("dados/pacientes.csv");

    int ciclo = 1;
    registrar_log("===== INÍCIO DA SIMULAÇÃO =====");

    while (1) {
        int houve_acao = 0;  // controle de ações a cada ciclo

        char buffer[60];
        sprintf(buffer, "\n[CICLO %02d]", ciclo++);
        registrar_log(buffer);

        // ===== ALTAS HOSPITALARES ALEATORIAS =====
        for (int i = 0; i < MAX_LEITOS; i++) {
            if (leitos.ocupados[i] && (rand() % 3 == 0)) {
                Paciente alta = *(leitos.leitos[i]);
                liberar_leito(&leitos, i);
                empilhar_historico(&historico, alta);

                char msg[150];
                sprintf(msg, "ALTA - %s (%s)", alta.id, alta.nome);
                registrar_log(msg);
                houve_acao = 1;
            }
        }

        // ===== INTERNAMENTO DOS PACIENTES =====
        if (!fila_vazia(&fila)) {
            Paciente proximo = remover_fila(&fila);
            int leito_index = internar_paciente(&leitos, proximo);
            if (leito_index != -1) {
                char msg[200];
                sprintf(msg, "INTERNADO - %s (%s, prioridade %d)", proximo.id, proximo.nome, proximo.prioridade);
                registrar_log(msg);
                houve_acao = 1;
            } 
            else {
                char msg[200];
                sprintf(msg, "! Todos os leitos estão ocupados. %s (%s) não pôde ser internado.", proximo.id, proximo.nome);
                registrar_log(msg);
                inserir_fila(&fila, proximo);  // devolve para a fila
            }
        }

        // ===== SORTEIO PARA INSERIR NA FILA =====
        if (fila.tamanho < TAM_DEQUE) {
            Paciente *sorteado = sortear_paciente();
            if (sorteado) {
                int inserido = inserir_fila(&fila, *sorteado);
                if (inserido) {
                    char msg[250];
                    const char *pos = (sorteado->prioridade >= 4) ? "início" : "fim";
                    sprintf(msg, "ESPERA - %s (%s, prioridade %d) -> inserido no %s do deque", sorteado->id, sorteado->nome, sorteado->prioridade, pos);
                    registrar_log(msg);
                    houve_acao = 1;
                }
            }
        } 
        else {
            registrar_log("! Deque está cheio. Paciente não pôde ser adicionado.");
        }

        // Verifica se algo aconteceu
        if (!houve_acao) {
            registrar_log("! Nenhuma ação realizada neste ciclo.");
        }

        // Verifica para encerrar o programa
        if (!paciente_disponivel() && fila_vazia(&fila)) {
            int leitos_ocupados = 0;
            for (int i = 0; i < MAX_LEITOS; i++)
                if (leitos.ocupados[i]) {
                    leitos_ocupados++;
                }    

            if (leitos_ocupados == 0) {
                break;
            }
                
        }

        sleep(2); //Espera 2s
    }

    registrar_log("===== FIM DA SIMULAÇÃO =====");
    return 0;
}
