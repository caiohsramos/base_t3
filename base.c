#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INT 1
#define DOUBLE 2
#define CHAR 3

typedef struct {
	char *nome;
	char *nomeTipo;
	int tipo;
	int order;
	int tamanho;
} CAMPO;

typedef struct {
	void **vetor;
} REGISTRO;

typedef struct {
	REGISTRO *registro;
	int n_registros;
	CAMPO *campo;
	int n_campos;
	char *nomeData;
	int tamanhoRegistro;
} LISTA;

char *lerNomeSchema(void) {
	char *nome = NULL;
	nome = (char*) malloc (30*sizeof(char));
	scanf("%s", nome);
	getchar();
	return nome;
}

LISTA *criarLista (void) {
	LISTA *lista = (LISTA*) malloc(sizeof(LISTA));
	lista->n_registros = 0;
	lista->n_campos = 0;
	lista->tamanhoRegistro = 0;
	lista->registro = NULL;
	lista->nomeData = NULL;
	lista->campo = NULL;
	return lista;
}

void criaCampo(LISTA *lista) {
	lista->campo = (CAMPO*) realloc (lista->campo, sizeof(CAMPO)*(lista->n_campos+1));
}

void atribuiTipo (LISTA *lista, char *tipo) {
	if (!strcmp(tipo, "int")) {
		lista->campo[lista->n_campos].tipo = INT;
	} else if (!strcmp(tipo, "double")) {
		lista->campo[lista->n_campos].tipo = DOUBLE;
	} else {
		lista->campo[lista->n_campos].tipo = CHAR;
		sscanf(&tipo[5], "%d", &lista->campo[lista->n_campos].tamanho);
	}
	lista->campo[lista->n_campos].nomeTipo = tipo;
}

void processarSchema (char *nomeSchema, LISTA *lista) {
	FILE *fp = NULL;
	char *token = NULL;
	token = (char*) malloc(30*sizeof(char));
	fp = fopen(nomeSchema, "r");
	while (fgetc(fp) != ' ');
	fscanf(fp, "%s", token);
	lista->nomeData = token;
	token = (char*) malloc(30*sizeof(char));
	fscanf(fp, "%s", token);
	do {
		if (!feof(fp)) {
			criaCampo(lista);
			//nome do campo
			lista->campo[lista->n_campos].nome = token;
			//tipo do campo
			token = (char*) malloc(30*sizeof(char));
			fscanf(fp, "%s", token);
			atribuiTipo(lista, token);
			//verifica order
			token = (char*) malloc(30*sizeof(char));
			fscanf(fp, "%s", token);
			if (!feof(fp)) {
				if (!strcmp(token, "order")) {
					lista->campo[lista->n_campos].order = 1;
					free(token);
					token = (char*) malloc(30*sizeof(char));
					fscanf(fp, "%s", token);
				} else lista->campo[lista->n_campos].order = 0;
			} else lista->campo[lista->n_campos].order = 0;
			lista->n_campos++;
		}
	} while (!feof(fp));
	free(token);
	fclose(fp);
}

void calculaTamanhos(LISTA *lista) {
	int i, n;
	n = lista->n_campos;
	for (i = 0; i < n; i++) {
		if(lista->campo[i].tipo == INT) {
			lista->campo[i].tamanho = 4;
			lista->tamanhoRegistro += lista->campo[i].tamanho;
		}
		if(lista->campo[i].tipo == DOUBLE) {
			lista->campo[i].tamanho = 8;
			lista->tamanhoRegistro += lista->campo[i].tamanho;
		}
		if(lista->campo[i].tipo == CHAR) {
			lista->tamanhoRegistro += lista->campo[i].tamanho;
		}
	}
}

void dump_schema (LISTA *lista) {
	int i, n;
	n = lista->n_campos;
	printf("table %s(%d bytes)\n", lista->nomeData, lista->tamanhoRegistro);
	for (i = 0; i < n; i++) {
		if (lista->campo[i].order == 0) {
			printf("%s %s(%d bytes)\n", lista->campo[i].nome, lista->campo[i].nomeTipo, lista->campo[i].tamanho);
		} else {
			printf("%s %s order(%d bytes)\n", lista->campo[i].nome, lista->campo[i].nomeTipo, lista->campo[i].tamanho);
		}
	}
}

void liberaCampos(LISTA *lista) {
	int i, n;
	n = lista->n_campos;
	for (i = 0; i < n; i++) {
		free(lista->campo[i].nome);
		free(lista->campo[i].nomeTipo);
	}
	free(lista->campo);
}

int main (int argc, char *arg[]) {
	char *nomeSchema = NULL;
	LISTA *lista = NULL;
	nomeSchema = lerNomeSchema();
	lista = criarLista();
	processarSchema(nomeSchema, lista);
	calculaTamanhos(lista);
	
	dump_schema(lista);
	
	liberaCampos(lista);
	free(lista->nomeData);
	free(nomeSchema);
	free(lista);
	return 0;
}
