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
	int n_registros;
	int n_index;
	CAMPO *campo;
	INDEX *index; //USEI
	int n_campos;
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
	lista->n_index = 0;
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
	lista->n_index = lista->n_registros;
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
			for(j = 0; j < lista->n_index; j++) {
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
	}
	fclose(fp);
}

int buscaBinaria(LISTA *lista, void *chave, int i, int *passo) {
	int inf = 0, sup, meio, tamanho, offset;
	char *nomeIndex = NULL;
	FILE *fp = NULL;
	void *dado = NULL;
	nomeIndex = (char*) malloc(sizeof(char)*30);
	(*passo) = 0;
	strcpy(nomeIndex, lista->nomeArquivo);
	nomeIndex = strcat(nomeIndex, "-");
	nomeIndex = strcat(nomeIndex, lista->campo[i].nome);
	nomeIndex = strcat(nomeIndex, ".idx");
	fp = fopen(nomeIndex, "r");
	tamanho = (lista->campo[i].tamanho);
	dado = malloc(tamanho);
	sup = (lista->n_index-1);
	while (inf <= sup) {
		(*passo) += 1;
		meio = (inf + sup)/2;
		fseek(fp, (meio*(tamanho+4)), SEEK_SET);
		fread(dado, tamanho, 1, fp);
		if(lista->campo[i].tipo == CHAR) {
			if (strcmp((char*)chave, (char*)dado) == 0) {
				fread(&offset, sizeof(int), 1, fp);
				free(nomeIndex);
				fclose(fp);
				free(dado);
				return offset;
			}
			else if (strcmp((char*)chave, (char*)dado) < 0)	sup = meio-1;
			else inf = meio+1;
		} else {
			if (memcmp(chave, dado, tamanho) == 0) {
				fread(&offset, sizeof(int), 1, fp);
				free(nomeIndex);
				fclose(fp);
				free(dado);
				return offset;
			}
			else if (memcmp(chave,  dado, tamanho) < 0)	sup = meio-1;
			else inf = meio+1;
		}
	}
	free(dado);
	free(nomeIndex);
	fclose(fp);
	return -1;   // não encontrado
}

void imprimeConteudo(LISTA *lista, int offset, char *campo_impresso) {
	int soma = 0, i;
	FILE *fp = NULL;
	void *dado = NULL;
	i = 0;
	while(strcmp(campo_impresso, lista->campo[i].nome)) {
		soma += lista->campo[i].tamanho;
		i++;
	}
	fp = fopen(lista->nomeData, "r");
	fseek(fp, offset, SEEK_SET);
	fseek(fp, soma, SEEK_CUR);
	switch(lista->campo[i].tipo) {
		case INT:
			dado = malloc(sizeof(int));
			fread(dado, sizeof(int), 1, fp);
			printf("%d\n", *(int*)dado);
			break;
		case DOUBLE:
			dado = malloc(sizeof(double));
			fread(dado, sizeof(double), 1, fp);
			printf("%.2lf\n", *(double*)dado);
			break;
		case CHAR:
			dado = malloc(lista->campo[i].tamanho);
			fread(dado, lista->campo[i].tamanho, 1, fp);
			printf("%s\n", (char*)dado);
			break;
	}
	fclose(fp);
	free(dado);
}

int buscaSeq(LISTA *lista, void *chave, int *passo, int i) {
	int j, soma = 0, n, tamanho;
	void *dado = NULL;
	FILE *fp = NULL;
	j = 0;
	while(strcmp(lista->campo[j].nome, lista->campo[i].nome)) {
		soma += lista->campo[j].tamanho;
		j++;
	}
	//printf("Soma: %d\n", soma);
	fp = fopen(lista->nomeData, "r");
	fseek(fp, 0, SEEK_END);
	lista->n_registros = ((ftell(fp))/(double)(lista->tamanhoRegistro));
	n = lista->n_index;
	//printf("Numero de index: %d\n", n);
	//printf("Numero de reg: %d\n", lista->n_registros);
	tamanho = (lista->campo[i].tamanho);
	//printf("tamanho: %d\n", tamanho);
	//printf("TIPO: %d\n", lista->campo[i].tipo);
	//fseek(fp, (n*lista->tamanhoRegistro), SEEK_SET);
	for(j = 0; j < (lista->n_registros - n); j++) {
		(*passo) += 1;
		dado = malloc(tamanho);
		fseek(fp, ((j*lista->tamanhoRegistro)+(n*lista->tamanhoRegistro)), SEEK_SET);
		fseek(fp, soma, SEEK_CUR);
		fread(dado, tamanho, 1, fp);
		//printf("dado(int) %d\n", *(int*)dado);
		if(lista->campo[i].tipo == CHAR) {
			if(!strcmp((char*)dado, (char*)chave)) {
				fclose(fp);
				free(dado);
				return ((j+n) * (lista->tamanhoRegistro));
			}
		} else if(!memcmp(chave, dado, tamanho)) {
			//printf("Encontrado na sequecial\n");
			fclose(fp);
			free(dado);
			return ((j+n) * (lista->tamanhoRegistro));
		}
		free(dado);
	}
	fclose(fp);
	return -1;
} 

void procura(LISTA *lista) {
	int i, offset, passo, encontrado = 0;
	char *campo = NULL;
	char *campo_impresso = NULL;
	void *termo = NULL;
	campo = (char*)malloc(30*sizeof(char));
	campo_impresso = (char*)malloc(30*sizeof(char));
	scanf("%s", campo);
	for(i = 0; i < lista->n_campos; i++) {
		if(!strcmp(campo, lista->campo[i].nome)) {
			if(lista->campo[i].order) {
				encontrado = 1;
				break;
			}
		}
	}
	if(encontrado) {
		switch(lista->campo[i].tipo) {
			case INT:
				termo = malloc(sizeof(int));
				scanf("%d", (int*)termo);
				break;
			case DOUBLE:
				termo = malloc(sizeof(double));
				scanf("%lf", (double*)termo);
				break;
			case CHAR:
				termo = malloc(lista->campo[i].tamanho);
				scanf("%s", (char*)termo);
				break;
		}
		scanf("%s", campo_impresso);
		offset = buscaBinaria(lista, termo, i, &passo);
		if(offset == -1) {
			//busca sequencial no .data
			//printf("Entrei na sequencial...\n");
			offset = buscaSeq(lista, termo, &passo, i);
		}
		printf("%d\n", passo);
		if(offset == -1) printf("value not found\n");
		else imprimeConteudo(lista, offset, campo_impresso);
	} else printf("index not found\n");
	free(campo);
	free(campo_impresso);
	free(termo);
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
		if(!strcmp(opt, "update_index")) criaArquivoIndex(lista);
		if(!strcmp(opt, "select")) procura(lista);
	} while(strcmp(opt, "exit"));
	
	
	free(opt);
	liberaCampos(lista);
	free(lista->nomeData);
	free(lista->nomeArquivo);
	free(nomeSchema);
	free(lista);
	return 0;
}
