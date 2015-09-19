/* Programa que le diversos arquivos de dados e realiza as operacoes
 * 	desejadas de acordo com a escolha do usuario.
 * 
 * T3 - Caio Ramos NUSP 9292991 
 */

//bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//atalhos para os diferentes tipos de dados
#define INT 1
#define DOUBLE 2
#define CHAR 3

//struct que guarda as informacoes sobre um determinado campo do ".schema"
typedef struct {
	char *nome; //nome do campo
	char *nomeTipo; //nome do tipo do campo
	int tipo; //tipo do campo (de acordo com os defines)
	int order; //1 - order, 0 - nao order
	int tamanho; //tamanho em bytes do dado do campo
} CAMPO;

//struct que guardara os dados de cada index quando for realizada uma operacao
//	com o arquivo ".idx".
typedef struct {
	void *chave; //guarda um endereco de uma chave
	int offset; //guarda o offset de uma chave
} INDEX;

//struct que junta todas as informacoes necessarias para manipulas os dados
//	dos arquivos.
typedef struct {
	int n_registros; //numero de registros no ".data"
	int n_index; //numero de index's salvos nos ".idx"
	CAMPO *campo; //vetor para os campos do ".schema"
	INDEX *index; //vetor para guardar os indices (em algumas operacoes)
	int n_campos; //numero de campos
	int tipoIndex; //tipo do index que esta sendo manipulado (de acordo com os defines)
	char *nomeArquivo; //nome inicial do arquivo que sera usado
	char *nomeData; //nome do arquivo ".data"
	int tamanhoRegistro; //tamanho de cada registro do ".data"
} LISTA;

/*
 * Funcao que le o nome do arquivo ".schema" e retorna o endereco alocado da string
 * 
 * name: char *lerNomeSchema(void)
 * @param: nenhum
 * @return: ponteiro para uma string alocada com o nome do arquivo
 * 
 */

char *lerNomeSchema(void) {
	char *nome = NULL;
	nome = (char*) malloc (30*sizeof(char));
	scanf("%s", nome);
	getchar();
	return nome;
}

/*
 * Funcao que inicializa uma struct LISTA, que guarda os dados necessarios
 * 	para manipular todos os arquivos
 * 
 * name: LISTA *criarLista(void)
 * @param: nenhum
 * @return: ponteiro para uma LISTA, com as variaveis iniciadas
 * 
 */

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

/*
 * Funcao que cria um novo espaco para um campo que sera lido do ".schema"
 * 
 * name: void criaCampo(LISTA*)
 * @param: lista - ponteiro para struct lista que contem os dados a serem manipulados
 * @return: nenhum
 * 
 */

void criaCampo(LISTA *lista) {
	lista->campo = (CAMPO*) realloc (lista->campo, sizeof(CAMPO)*(lista->n_campos+1));
}

/*
 * Funcao que compara uma string com o nome de um tipo de dado e atribui
 * um tipo a variavel "campo.tipo" do ultimo campo lido do ".schema".
 * 
 * name: void atribuiTipo(LISTA*,char*)
 * @param: lista - ponteiro para a struct LISTA, tipo - string com o 
 * 			tipo de dado a ser atribuido.
 * @return: nenhum
 * 
 */

void atribuiTipo (LISTA *lista, char *tipo) {
	if (!strcmp(tipo, "int")) {
		lista->campo[lista->n_campos].tipo = INT;
	} else if (!strcmp(tipo, "double")) {
		lista->campo[lista->n_campos].tipo = DOUBLE;
	} else {
		//no caso de umc char[], o tamanho e calculado aqui...
		lista->campo[lista->n_campos].tipo = CHAR;
		sscanf(&tipo[5], "%d", &lista->campo[lista->n_campos].tamanho);
	}
	lista->campo[lista->n_campos].nomeTipo = tipo;
}

/*
 * Funcao que abre o ".schema", le e organiza as informacoes neles contidas.
 * 		A leitura gravara os dados nas variaveis da struct LISTA.
 * 
 * name: void processarSchema(char*, LISTA*)
 * @param: nomeSchema - string com o nome do arquivo ".schema", 
 * 		   lista - ponteiro para struct LISTA que guardara as informacoes sobre os arquivos
 * @return: nenhum
 * 
 */

void processarSchema (char *nomeSchema, LISTA *lista) {
	FILE *fp = NULL;
	char *token = NULL;
	token = (char*) malloc(30*sizeof(char)); //variavel que guardara palavra por palavra do arquivo
	fp = fopen(nomeSchema, "r");
	while (fgetc(fp) != ' '); //move o ponteiro do arquivo para o primerio token a ser lido
	fscanf(fp, "%s", token);
	lista->nomeArquivo = token; //guarda o nome inicial do arquivo
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
				//se o campo lido tiver o atributo 'order'...
				if (!strcmp(token, "order")) {
					lista->campo[lista->n_campos].order = 1;
					free(token);
					token = (char*) malloc(30*sizeof(char));
					fscanf(fp, "%s", token);
				//senao...
				} else lista->campo[lista->n_campos].order = 0;
			} else lista->campo[lista->n_campos].order = 0;
			//incrementa o numero de campos lidos
			lista->n_campos++;
		}
	} while (!feof(fp)); //realiza a rotina ate o fim de arquivo
	//libera o espaco alocado para o token se ele nao for nulo
	if(token != NULL) free(token);
	fclose(fp); //fecha o arquivo
	
	//cria o nome para o arquivo ".data"
	lista->nomeData = (char*)malloc(sizeof(char)*30);
	strcpy(lista->nomeData, lista->nomeArquivo);
	strcat(lista->nomeData, ".data");
}

/*
 * Funcao que calcula os tamanhos dos campos lidos de acordo com os tipos guardados,
 * 		alem do tamanho total de cada registro.
 * 
 * name: void calculaTamanhos(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

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

/*
 * Funcao que faz a impressao do ".schema", sem carrega-lo para a memoria.
 * 
 * name: void dump_schema(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

void dump_schema (LISTA *lista) {
	int i, n;
	n = lista->n_campos;
	printf("table %s(%d bytes)\n", lista->nomeArquivo, lista->tamanhoRegistro);
	for (i = 0; i < n; i++) {
		//verifica se o campo tem o atributo order...
		if (lista->campo[i].order == 0) {
			printf("%s %s(%d bytes)\n", lista->campo[i].nome, lista->campo[i].nomeTipo, lista->campo[i].tamanho);
		//senao...
		} else {
			printf("%s %s order(%d bytes)\n", lista->campo[i].nome, lista->campo[i].nomeTipo, lista->campo[i].tamanho);
		}
	}
}

/*
 * Funcao que faz a impressao do ".data" sem carrega-lo completamente para a memoria
 * 
 * name: void dump_data(LISTA*)
 * @param: ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

void dump_data(LISTA *lista) {
	int i, n, j;
	FILE *fp = NULL;
	void *p = NULL;
	fp = fopen(lista->nomeData, "r");
	fseek(fp, 0, SEEK_END);
	
	//calcula o numero de registros do .data
	lista->n_registros = ((ftell(fp))/(double)(lista->tamanhoRegistro));
	
	fseek(fp, 0, SEEK_SET);
	n = lista->n_campos;
	for(i = 0; i < lista->n_registros; i++) { //percorre os 'n' registos
		for(j = 0; j < n; j++) { //percorre os 'n' campos
			switch (lista->campo[j].tipo) {
				//para cada caso e feito uma alocacao do tamanho do dado a ser impresso
				//	em um ponteiro void, e um casting e feito na hora da impressao.
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

/*
 * Funcao que aloca um novo index e le do ".data" somente o necessario.
 * Os index's serao liberados apos a criacao do ".idx"
 * 
 * name: void novoIndex(LISTA *lista, int tipo, int offset, int tamanho, int offset_registro)
 * @param: lista - ponteiro para struct LISTA, tipo - tipo a ser guardado, 
 * 			offset - offset a ser lido, tamanho - bytes do 'tipo', 
 * 			offset_registro - offset no registro que esta sendo lido.
 * @return: nenhum
 * 
 */

void novoIndex(LISTA *lista, int tipo, int offset, int tamanho, int offset_registro) {
	FILE *fp = NULL;
	fp = fopen(lista->nomeData, "r");
	fseek(fp, offset, SEEK_SET);
	//adiciona um index ao vetor de index
	lista->index = (INDEX*) realloc(lista->index, sizeof(INDEX)*(lista->n_index+1));
	lista->index[lista->n_index].chave = malloc(tamanho);
	//le a chave do ".data"
	fread(lista->index[lista->n_index].chave, tamanho, 1, fp);
	lista->index[lista->n_index].offset = offset_registro;
	lista->tipoIndex = tipo;
	lista->n_index++;
	fclose(fp);
}

/* As funcoes a seguir sao para serem passadas como ponteiros para funcao,
 * 	para a funcao de ordenacao das chaves dos index's. Todas sao analogas a primeira.
 * 
 * name: int comp(((Tipo)))(INDEX*, INDEX*)
 * @param: a - primero index a ser comparado, b - segundo index a ser comparado
 * @return: 1, se a > b; 0 se a <=b
 * 
 */
 
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

/*
 * Funcao que ordena os index carregados na memoria
 * 
 * name: void ordenaIndex(LISTA*, int(*f)(INDEX*, INDEX*))
 * @param: lista - ponteiro para struct LISTA, f - ponteiro para a funcao de ordenacao desejada
 * @return: nenhum
 * 
 */

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
 * Funcao que da 'free' nos ponteiro dos index's e finalmente, no proprio vetor
 * 		de index's.
 * 
 * name: void liberaIndex(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

void liberaIndex(LISTA *lista) {
	int i;
	for(i = 0; i < lista->n_index; i++) {
		free(lista->index[i].chave);
	}
	free(lista->index);
	lista->index = NULL;
}

/*
 * Funcao que ordena os index carregados na memoria
 * 
 * name: void gravaIndex(LISTA*,int,int)
 * @param: lista - ponteiro para struct LISTA, tamanho - tamanho do dado,
 * 		pos - posicao do campo a ser criado um arquivo ".idx"
 * @return: nenhum
 * 
 */

void gravaIndex(LISTA *lista, int tamanho, int pos) {
	FILE *fp = NULL;
	int i;
	char *nomeIndex = (char*)malloc(sizeof(char)*30);
	strcpy(nomeIndex, lista->nomeArquivo);
	nomeIndex = strcat(nomeIndex, "-");
	nomeIndex = strcat(nomeIndex, lista->campo[pos].nome);
	nomeIndex = strcat(nomeIndex, ".idx");
	fp = fopen(nomeIndex, "w");
	for(i = 0; i < lista->n_index; i++) { //percorre os 'n' index
		fwrite(lista->index[i].chave, tamanho, 1, fp); //grava a chave
		fwrite(&lista->index[i].offset, sizeof(int), 1, fp); //grava o offset
	}
	fclose(fp);
	free(nomeIndex);
}

/*
 * Funcao que cria os arquivos ".idx", se o atributo order estiver presente
 * 
 * name: void criaArquivoIndex(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

void criaArquivoIndex(LISTA *lista) {
	FILE *fp = NULL;
	int i, j, soma = 0;
	fp = fopen(lista->nomeData, "r");
	fseek(fp, 0, SEEK_END);
	//conta o numero de registros do ".data"
	lista->n_registros = ((ftell(fp))/(double)(lista->tamanhoRegistro));
	fclose(fp);
	for (i = 0; i < lista->n_campos; i++) { //percorre os 'n' campos
		if(lista->campo[i].order) { //se o campo tiver o atributo order...
			lista->n_index = 0;
			for(j = 0; j < lista->n_registros; j++) { //percorre os 'n' registros...
				//...criando um index para cada um
				novoIndex(lista, lista->campo[i].tipo, (((lista->tamanhoRegistro)*j)+soma), lista->campo[i].tamanho, j*lista->tamanhoRegistro);
			}
			//ordena o vetor de index
			if(lista->campo[i].tipo == INT) {
				ordenaIndex(lista, &compInt);
			}
			if(lista->campo[i].tipo == DOUBLE) {
				ordenaIndex(lista, &compDouble);
			}
			if(lista->campo[i].tipo == CHAR) {
				ordenaIndex(lista, &compChar);
			}
			
			//gravar os dados na memoria para o arquivo correto
			gravaIndex(lista, lista->campo[i].tamanho, i);
			//libera os index salvos na memoria
			liberaIndex(lista);
		}
		//soma para saber qual o campo de cada registro deve ser gravado
		soma += lista->campo[i].tamanho;
	}
	//atualiza o numero de indexs presentes nos arquivos
	lista->n_index = lista->n_registros;
}

/*
 * Funcao que imprime os index's dos arquivos ".idx"
 * 
 * name: void dump_index(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

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
		
		//tenta abrir um arquivo ".idx"
		fp = fopen(nomeIndex, "r");
		
		//se o arquivo foi aberto...(existe)
		if(fp != NULL) {
			for(j = 0; j < lista->n_index; j++) { //percorre os 'n' index
				p = malloc(lista->campo[i].tamanho); //aloca um ponteiro void
				fread(p, lista->campo[i].tamanho, 1, fp); //le a chave
				fread(&offset, sizeof(int), 1, fp); //le o offset
				switch(lista->campo[i].tipo) { //imprime com o casting correto
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

/*
 * Funcao que insere um novo registro no ".data"
 * 
 * name: void insert(LISTA*)
 * @param: lista - ponteiro
 * @return: nenhum
 * 
 */

void insert(LISTA *lista) {
	void *dado = NULL;
	int i;
	FILE *fp = NULL;
	
	//abre o arquivo no modo "append"
	fp = fopen(lista->nomeData, "a");
	
	for(i = 0; i < lista->n_campos; i++) {
		dado = malloc(lista->campo[i].tamanho);
		//faz a leitura adequada para o campo...
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
		//escreve o dado no arquivo
		fwrite(dado, lista->campo[i].tamanho, 1, fp);
		free(dado);
	}
	fclose(fp);
}

/*
 * Funcao que faz a busca binaria no arquivo ".idx"
 * 
 * name: int buscaBinaria(LISTA*,void*,int,int*)
 * @param: lista - ponteiro para struct LISTA, chave - chave a ser buscada,
 * 		i - posicao do campo no vetor de campos, passo - contador de passos
 * @return: offset da chave, se for encontrada; -1, se nao for encontrada
 * 
 */

int buscaBinaria(LISTA *lista, void *chave, int i, int *passo) {
	int inf = 0, sup, meio, tamanho, offset;
	char *nomeIndex = NULL;
	FILE *fp = NULL;
	void *dado = NULL;
	nomeIndex = (char*) malloc(sizeof(char)*30);
	(*passo) = 0;
	
	//abre o arquivo de dados
	strcpy(nomeIndex, lista->nomeArquivo);
	nomeIndex = strcat(nomeIndex, "-");
	nomeIndex = strcat(nomeIndex, lista->campo[i].nome);
	nomeIndex = strcat(nomeIndex, ".idx");
	fp = fopen(nomeIndex, "r");
	tamanho = (lista->campo[i].tamanho);
	dado = malloc(tamanho);
	sup = (lista->n_index-1);
	
	//realiza a busca binaria
	while (inf <= sup) {
		(*passo) += 1;
		meio = (inf + sup)/2;
		fseek(fp, (meio*(tamanho+4)), SEEK_SET);
		fread(dado, tamanho, 1, fp);
		if(lista->campo[i].tipo == CHAR) { //procedimento para se o campo for um char[]
			if (strcmp((char*)chave, (char*)dado) == 0) {
				fread(&offset, sizeof(int), 1, fp);
				free(nomeIndex);
				fclose(fp);
				free(dado);
				return offset;
			}
			else if (strcmp((char*)chave, (char*)dado) < 0)	sup = meio-1;
			else inf = meio+1;
		} else { //procedimento para os outros tipos
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
	return -1;   // chave nao encontrada
}

/*
 * Funcao que imprime o conteudo de um campo a partir do nome do campo e
 * 		do offset de um registro.
 * 
 * name: void imprimeConteudo(LISTA*, int,char*)
 * @param: lista - ponteiro para struct LISTA, offset - offset do registro 
 * 		que contem o que deve ser impresso, campo_impresso - nome do campo 
 * 		a ser impresso.
 * @return: nenhum
 * 
 */

void imprimeConteudo(LISTA *lista, int offset, char *campo_impresso) {
	int soma = 0, i;
	FILE *fp = NULL;
	void *dado = NULL;
	i = 0;
	
	//calcula quantos bytes apos o inicio do registro o campo esta
	while(strcmp(campo_impresso, lista->campo[i].nome)) {
		soma += lista->campo[i].tamanho;
		i++;
	}
	fp = fopen(lista->nomeData, "r");
	//posiciona o ponteiro do arquivo no byte certo
	fseek(fp, offset, SEEK_SET);
	fseek(fp, soma, SEEK_CUR);
	
	//realiza a impressao do dado com o casting certo
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

/*
 * Funcao que realiza uma busca sequencial no ".data" apos a busca binaria
 * 		nao obter sucesso.
 * 
 * name: int buscaSeq(LISTA*,void*,int*,int)
 * @param: lista - ponteiro para struct LISTA, chave - chave a ser buscada,
 * 		passo - contador de passos, i - posicao do campo no vetor de CAMPO.
 * @return: offset do registro, se a chave for encontrada; -1, caso nao encontre
 * 
 */

int buscaSeq(LISTA *lista, void *chave, int *passo, int i) {
	int j, soma = 0, n, tamanho;
	void *dado = NULL;
	FILE *fp = NULL;
	j = 0;
	//calcula quantos bytes apos o inicio do registro o campo esta
	while(strcmp(lista->campo[j].nome, lista->campo[i].nome)) {
		soma += lista->campo[j].tamanho;
		j++;
	}
	fp = fopen(lista->nomeData, "r");
	fseek(fp, 0, SEEK_END);
	lista->n_registros = ((ftell(fp))/(double)(lista->tamanhoRegistro));
	n = lista->n_index;
	tamanho = (lista->campo[i].tamanho);
	for(j = 0; j < (lista->n_registros - n); j++) {
		(*passo) += 1;
		dado = malloc(tamanho);
		//posiciona o ponteiro do arquivo...
		fseek(fp, ((j*lista->tamanhoRegistro)+(n*lista->tamanhoRegistro)), SEEK_SET);
		fseek(fp, soma, SEEK_CUR);
		//le o dado do arquivo
		fread(dado, tamanho, 1, fp);
		if(lista->campo[i].tipo == CHAR) { //se o tipo do campo for char[]
			if(!strcmp((char*)dado, (char*)chave)) {
				fclose(fp);
				free(dado);
				return ((j+n) * (lista->tamanhoRegistro));
			}
		} else if(!memcmp(chave, dado, tamanho)) { //se o campo possuir outro tipo
			fclose(fp);
			free(dado);
			return ((j+n) * (lista->tamanhoRegistro));
		}
		free(dado);
	}
	fclose(fp);
	return -1;
} 

/*
 * Funcao que faz a procura de um item no ".data" com a busca binaria no ".idx"
 * 	 e sequencial no ".data"
 * name: void procura(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

void procura(LISTA *lista) {
	int i, offset, passo, encontrado = 0;
	char *campo = NULL;
	char *campo_impresso = NULL;
	void *termo = NULL;
	campo = (char*)malloc(30*sizeof(char));
	campo_impresso = (char*)malloc(30*sizeof(char));
	
	//leitura dos dados para a pesquisa
	scanf("%s", campo);
	for(i = 0; i < lista->n_campos; i++) {
		if(!strcmp(campo, lista->campo[i].nome)) {
			if(lista->campo[i].order) {
				encontrado = 1;
				break;
			}
		}
	}
	//caso o campo informado possua o atributo 'order'
	if(encontrado) {
		//leitura do valor de acordo com o campo
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
		//leitura do nome do campo a ser impresso, caso a chave seja encontrada
		scanf("%s", campo_impresso);
		//realiza a busca binaria
		offset = buscaBinaria(lista, termo, i, &passo);
		if(offset == -1) {
			//realiza a busca sequncial (binaria falhou)
			offset = buscaSeq(lista, termo, &passo, i);
		}
		//imprime a quantidade de passos
		printf("%d\n", passo);
		//caso o valor nao seja encontrado
		if(offset == -1) printf("value not found\n");
		//caso o valor seja encontrado
		else imprimeConteudo(lista, offset, campo_impresso);
	} else printf("index not found\n"); //caso o campo informado nao possua 'order'
	free(campo);
	free(campo_impresso);
	free(termo);
}

/*
 * Funcao que libera as alocacoes de cada campo de LISTA
 * name: void liberaCampos(LISTA*)
 * @param: lista - ponteiro para struct LISTA
 * @return: nenhum
 * 
 */

void liberaCampos(LISTA *lista) {
	int i, n;
	n = lista->n_campos;
	for (i = 0; i < n; i++) {
		free(lista->campo[i].nome);
		free(lista->campo[i].nomeTipo);
	}
	free(lista->campo);
}


//inicio do programa
int main (int argc, char *arg[]) {
	char *nomeSchema = NULL;
	char *opt = NULL;
	LISTA *lista = NULL;
	//processamento dos arquivos
	nomeSchema = lerNomeSchema();
	lista = criarLista();
	processarSchema(nomeSchema, lista);
	calculaTamanhos(lista);
	criaArquivoIndex(lista);
	opt = (char*) malloc(sizeof(char)*20);
	
	//realiza as operacoes
	do {
		scanf("%s", opt);
		if(!strcmp(opt, "dump_schema")) dump_schema(lista);
		if(!strcmp(opt, "dump_data")) dump_data(lista);
		if(!strcmp(opt, "dump_index")) dump_index(lista);
		if(!strcmp(opt, "insert")) insert(lista);
		if(!strcmp(opt, "update_index")) criaArquivoIndex(lista);
		if(!strcmp(opt, "select")) procura(lista);
	} while(strcmp(opt, "exit"));
	
	//liberacoes de memoria
	free(opt);
	liberaCampos(lista);
	free(lista->nomeData);
	free(lista->nomeArquivo);
	free(nomeSchema);
	free(lista);
	//fim do programa
	return 0;
}
