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
	void *chave;
	int offset;
} INDEX; //USEI

typedef struct {
	void **registro; //AINDA NAO USEI
	int n_registros;
	CAMPO *campo;
	INDEX *index; //USEI
	int n_campos;
	int n_index;
	int tipoIndex;
	char *nomeArquivo;
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
	lista->n_index = 0;
	lista->tamanhoRegistro = 0;
	lista->tipoIndex = 0;
	lista->registro = NULL;
	lista->index = NULL;
	lista->nomeData = NULL;
	lista->nomeArquivo = NULL;
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
	lista->nomeArquivo = token;
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
	if(token != NULL) free(token);
	fclose(fp);
	lista->nomeData = (char*)malloc(sizeof(char)*30);
	strcpy(lista->nomeData, lista->nomeArquivo);
	strcat(lista->nomeData, ".data");
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
	printf("table %s(%d bytes)\n", lista->nomeArquivo, lista->tamanhoRegistro);
	for (i = 0; i < n; i++) {
		if (lista->campo[i].order == 0) {
			printf("%s %s(%d bytes)\n", lista->campo[i].nome, lista->campo[i].nomeTipo, lista->campo[i].tamanho);
		} else {
			printf("%s %s order(%d bytes)\n", lista->campo[i].nome, lista->campo[i].nomeTipo, lista->campo[i].tamanho);
		}
	}
}

void dump_data(LISTA *lista) {
	int i, n, j;
	FILE *fp = NULL;
	void *p = NULL;
	fp = fopen(lista->nomeData, "r");
	fseek(fp, 0, SEEK_END);
	lista->n_registros = ((ftell(fp))/(double)(lista->tamanhoRegistro));
	fseek(fp, 0, SEEK_SET);
	n = lista->n_campos;
	for(i = 0; i < lista->n_registros; i++) {
		for(j = 0; j < n; j++) {
			switch (lista->campo[j].tipo) {
				case INT:
					p = malloc(sizeof(int));
					fread(p, sizeof(int), 1, fp);
					printf("%s = %d\n", lista->campo[j].nome, *((int*)p));
					free(p);
					break;
				case DOUBLE:
					p = malloc(sizeof(double));
					fread(p, sizeof(double), 1, fp);
					printf("%s = %.2lf\n", lista->campo[j].nome, *((double*)p));
					free(p);
					break;
				case CHAR:
					p = malloc(sizeof(char)*(lista->campo[j].tamanho));
					fread(p, sizeof(char), lista->campo[j].tamanho, fp);
					printf("%s = %s\n", lista->campo[j].nome, (char*)p);
					free(p);
					break;
			}
		}
	}
	fclose(fp);
}

void novoIndex(LISTA *lista, int tipo, int offset, int tamanho, int offset_registro) {
	FILE *fp = NULL;
	fp = fopen(lista->nomeData, "r");
	fseek(fp, offset, SEEK_SET);
	lista->index = (INDEX*) realloc(lista->index, sizeof(INDEX)*(lista->n_index+1));
	lista->index[lista->n_index].chave = malloc(tamanho);
	fread(lista->index[lista->n_index].chave, tamanho, 1, fp);
	lista->index[lista->n_index].offset = offset_registro;
	lista->tipoIndex = tipo;
	lista->n_index++;
	fclose(fp);
}

int compInt(INDEX *a, INDEX *b) {
	int A, B;
	A = *((int*)a->chave);
	B = *((int*)b->chave);
	if(A > B) return 1;
	else return 0;
}
int compDouble(INDEX *a, INDEX *b) {
	double A, B;
	A = *((double*)a->chave);
	B = *((double*)b->chave);
	if(A > B) return 1;
	else return 0;
}
int compChar(INDEX *a, INDEX *b) {
	char *A, *B;
	A = (char*)a->chave;
	B = (char*)b->chave;
	if (strcmp(A, B) > 0) return 1;
	else return 0;
} 

void ordenaIndex(LISTA *lista, int(*f)(INDEX*, INDEX*)) {
	int i, j;
	INDEX eleito;
	for (i = 0; i < lista->n_index; i++) {
		eleito = lista->index[i];
		j = i;
		while ((j > 0) && f(&lista->index[j-1], &eleito)) {
			lista->index[j] = lista->index[j-1];
			j--;
		}
		lista->index[j] = eleito;
	}
}

/*
void imprimeIndex(LISTA  *lista, int tipo) {
	int i;
	for(i = 0; i < lista->n_index; i++) {
		printf("%d = %d\n", *(int*)lista->index[i].chave, lista->index[i].offset);
	}
}
*/


void liberaIndex(LISTA *lista) {
	int i;
	for(i = 0; i < lista->n_index; i++) {
		free(lista->index[i].chave);
	}
	free(lista->index);
	lista->index = NULL;
}

void gravaIndex(LISTA *lista, int tamanho, int pos) {
	FILE *fp = NULL;
	int i;
	char *nomeIndex = (char*)malloc(sizeof(char)*30);
	strcpy(nomeIndex, lista->nomeArquivo);
	nomeIndex = strcat(nomeIndex, "-");
	nomeIndex = strcat(nomeIndex, lista->campo[pos].nome);
	nomeIndex = strcat(nomeIndex, ".idx");
	fp = fopen(nomeIndex, "w");
	for(i = 0; i < lista->n_index; i++) {
		fwrite(lista->index[i].chave, tamanho, 1, fp);
		fwrite(&lista->index[i].offset, sizeof(int), 1, fp);
	}
	fclose(fp);
	free(nomeIndex);
}

void criaArquivoIndex(LISTA *lista) {
	FILE *fp = NULL;
	int i, j, soma = 0;
	fp = fopen(lista->nomeData, "r");
	fseek(fp, 0, SEEK_END);
	lista->n_registros = ((ftell(fp))/(double)(lista->tamanhoRegistro));
	fclose(fp);
	for (i = 0; i < lista->n_campos; i++) {
		if(lista->campo[i].order) {
			lista->n_index = 0;
			for(j = 0; j < lista->n_registros; j++) {
				//criar vetor de indexs
				novoIndex(lista, lista->campo[i].tipo, (((lista->tamanhoRegistro)*j)+soma), lista->campo[i].tamanho, j*lista->tamanhoRegistro);
			}
			//ordenar o vetor de index
			if(lista->campo[i].tipo == INT) {
				ordenaIndex(lista, &compInt);
			}
			if(lista->campo[i].tipo == DOUBLE) {
				ordenaIndex(lista, &compDouble);
			}
			if(lista->campo[i].tipo == CHAR) {
				ordenaIndex(lista, &compChar);
			}
			//imprimeIndex(lista, lista->campo[i].tipo); //temporario
			//gravar no arquivo
			gravaIndex(lista, lista->campo[i].tamanho, i);
			//free no vetor de index, zerar n_index
			liberaIndex(lista);
		}
		soma += lista->campo[i].tamanho;
	}
}

void dump_index(LISTA *lista) {
	int i, j, offset;
	FILE *fp = NULL;
	void *p = NULL;
	char *nomeIndex = NULL;
	for(i = 0; i < lista->n_campos; i++) {
		nomeIndex = (char*)malloc(sizeof(char)*30);
		strcpy(nomeIndex, lista->nomeArquivo);
		nomeIndex = strcat(nomeIndex, "-");
		nomeIndex = strcat(nomeIndex, lista->campo[i].nome);
		nomeIndex = strcat(nomeIndex, ".idx");
		fp = fopen(nomeIndex, "r");
		if(fp != NULL) {
			for(j = 0; j < lista->n_registros; j++) {
				p = malloc(lista->campo[i].tamanho);
				fread(p, lista->campo[i].tamanho, 1, fp);
				fread(&offset, sizeof(int), 1, fp);
				switch(lista->campo[i].tipo) {
					case INT:
						printf("%d = %d\n", *(int*)p, offset);
						break;
					case DOUBLE:
						printf("%lf = %d\n", *(double*)p, offset);
						break;
					case CHAR:
						printf("%s = %d\n", (char*)p, offset);
						break;
				}
				free(p);
			}
			fclose(fp);
		}
		free(nomeIndex);
	}
}

void insert(LISTA *lista) {
	void *dado = NULL;
	int i;
	FILE *fp = NULL;
	fp = fopen(lista->nomeData, "a");
	for(i = 0; i < lista->n_campos; i++) {
		dado = malloc(lista->campo[i].tamanho);
		switch(lista->campo[i].tipo) {
			case INT:
				scanf("%d", (int*)dado);
				break;
			case DOUBLE:
				scanf("%lf", (double*)dado);
				break;
			case CHAR:
				scanf("%s", (char*)dado);
				break;
		}
		fwrite(dado, lista->campo[i].tamanho, 1, fp);
		free(dado);
		lista->n_registros++;
	}
	fclose(fp);
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
	char *opt = NULL;
	LISTA *lista = NULL;
	nomeSchema = lerNomeSchema();
	lista = criarLista();
	processarSchema(nomeSchema, lista);
	calculaTamanhos(lista);
	criaArquivoIndex(lista);
	opt = (char*) malloc(sizeof(char)*20);
	
	do {
		scanf("%s", opt);
		if(!strcmp(opt, "dump_schema")) dump_schema(lista);
		if(!strcmp(opt, "dump_data")) dump_data(lista);
		if(!strcmp(opt, "dump_index")) dump_index(lista);
		if(!strcmp(opt, "insert")) insert(lista);
	} while(strcmp(opt, "exit"));
	
	//falta fazer o insert
	
	free(opt);
	liberaCampos(lista);
	free(lista->nomeData);
	free(lista->nomeArquivo);
	free(nomeSchema);
	free(lista);
	return 0;
}
