// Lucas Gabriel Mendes Miranda - 10265892
// André Rennó de Campos        - 10298864
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define TAM_REG_CAB 19  //tamanho fixo do registro de cabeçalho
#define TAM_REG_PAD 85  //tamanho fixo do registro padrão
#define DELIMITADOR '|'  //delimitador de campos de tamanho variável
#define FIM_STRING '\0'  //no final de TODA string em RAM
#define LIXO '#'  //inserido quando sobra espaço no final do registro
#define NULO_TAM_FIXO -1  //indicador de nulo para campos de tamanho fixo
#define NUM_REG_MAX 5000  //numero maximo de registros num arquivo de dados
#define NUM_VER_MAX 10000  //número máximo de vértices no arquivo auxiliar
#define TAM_CHAVE 10  //tamanho das chaves armazenadas no arquivo auxiliar

typedef struct registro_cabecalho { //modelo do registro de cabeçalho (tamanho fixo)
    char status; //1 byte - indica a consistência (0 é inconsistente e 1 é consistente)
    int numeroVertices; //4 bytes - indica o número de cidades diferentes
    int numeroArestas; //4 bytes - indica o número de registros no arquivo de dados
    char dataUltimaCompactacao[11]; //10 bytes + 1 bytes (\0) - indica a data em que ocorreu a última compactação
} REGISTRO_CABECALHO;

typedef struct registro_padrao { //modelo de um registro padrão (tamanho fixo)
    //campos de tamanho fixo:
    char estadoOrigem[3]; //2 bytes + 1 byte (\0) - indica o estado da cidade de origem
    char estadoDestino[3]; //2 bytes + 1 byte (\0) - indica o estado da cidade de destino
    int distancia; //4 bytes - indica a distância entre as duas localidades

    //campos de tamanho variavel (apesar disso, usamos alocação estática):
    char cidadeOrigem[100];
    char cidadeDestino[100];
    char tempoViagem[100]; //indica o tempo de viagem entre as duas localidades (aceita valores nulos)

    //indica o número de registros no arquivo de dados: (só conterá um valor no primeiro registro, mas não será escrito)
    int numReg;
} REGISTRO_PADRAO;

typedef struct vertice { //um modelo de registro para um arquivo de dados auxiliar para a contagem de vertices
    //campos de tamanho fixo:
    char chaveCidade[TAM_CHAVE]; //4 bytes + 1 byte (\0) - indica uma parte do nome de uma cidade (o que couber em 4 bytes, sendo que resto será truncado)
    int numeroArestas; //4 bytes - indica o número de arestas ligadas a cada vértice

    //indica o número de vértices no arquivo de dados: (apenas o primeiro registro conterá um valor)
    int numVert;
} VERTICE;

//---->Funções dadas pelo Matheus:

void binarioNaTela1(char *nomeArquivoBinario) {

    /* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
     *  Ela vai abrir de novo para leitura e depois fechar. */

    unsigned long i, cs;
    unsigned char *mb;
    size_t fl;
    FILE *fs;
    if (nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
        fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela1): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
        return;
    }
    fseek(fs, 0, SEEK_END);
    fl = ftell(fs);
    fseek(fs, 0, SEEK_SET);
    mb = (unsigned char *) malloc(fl);
    fread(mb, 1, fl, fs);

    cs = 0;
    for (i = 0; i < fl; i++) {
        cs += (unsigned long) mb[i];
    }
    printf("%lf\n", (cs / (double) 100));
    free(mb);
    fclose(fs);
}

void trim(char *str) {

    /*
     *	Essa função arruma uma string de entrada "str".
     *	Manda pra ela uma string que tem '\r' e ela retorna sem.
     *	Ela remove do início e do fim da string todo tipo de espaçamento (\r, \n, \t, espaço, ...).
     *	Por exemplo:
     *
     *	char minhaString[] = "    \t TESTE  DE STRING COM BARRA R     \t  \r\n ";
     *	trim(minhaString);
     *	printf("[%s]", minhaString); // vai imprimir "[TESTE  DE STRING COM BARRA R]"
     *
     */

    size_t len;
    char *p;

    for (len = strlen(str); len > 0 && isspace(str[len - 1]); len--); // remove espaçamentos do fim
    str[len] = '\0';
    for (p = str; *p != '\0' && isspace(*p); p++); // remove espaçamentos do começo
    len = strlen(p);
    memmove(str, p, sizeof (char) * (len + 1));
}

void scan_quote_string(char *str) {

    /*
     *	Use essa função para ler um campo string delimitado entre aspas (").
     *	Chame ela na hora que for ler tal campo. Por exemplo:
     *
     *	A entrada está da seguinte forma:
     *		nomeDoCampo "MARIA DA SILVA"
     *
     *	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
     *		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
     *		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
     *
     */

    char R;

    while ((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar(); // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar(); // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

//---->Funções implementadas por mim:

void limparString(char string[], int tamanho) {
    /*
     * Essa função preenche uma string de um dado tamanho com '\0'.
     */

    int i;

    for (i = 0; i < tamanho; i++) {
        string[i] = '\0';
    }

    return;
}

char* geraChaveCidade(char cidade[]) {
    /*
     * Essa função gera uma chave de uma dada cidade. Essa chave é armazenada num vetor
     * de VERTICEs , que é mantido para contabilizar o número de vértices registrado no arquivo
     * de dados.
     */

    int i;
    char* chaveGerada;

    //aloca:
    chaveGerada = (char*) calloc(TAM_CHAVE, sizeof (char));
    if (chaveGerada == NULL) {
        return NULL; //erro de alocação
    }

    //gera a chave e a retorna:
    for (i = 0; i < TAM_CHAVE - 1; i++) {
        chaveGerada[i] = cidade[i];
    }

    return chaveGerada;
}

int buscaBinaria(VERTICE* verticesOrdenados, char *chave, int tam) {
    /*
     * Essa função realiza uma busca binária simples no vetor de VERTICEs.
     */
    int bottom = 0;
    int meio;
    int top = tam - 1;

    while (bottom <= top) {
        meio = (bottom + top) / 2;
        if (strcmp(verticesOrdenados[meio].chaveCidade, chave) == 0) {
            return meio;
        } else if (strcmp(verticesOrdenados[meio].chaveCidade, chave) > 0) {
            top = meio - 1;
        } else if (strcmp(verticesOrdenados[meio].chaveCidade, chave) < 0) {
            bottom = meio + 1;
        }
    }
    return -1;
}

int insereOrdenado(VERTICE* verticesOrdenados, int n, char* chave) {
    /*
     * Essa função insere um vértice de maneira ordenada no vetor de VERTICEs.
     * A inserção precisa ser ordenada para viabilizar a busca binária.
     */

    // Não pode inserir mais um elemento se n for superior ao número de vértices máximo
    if (n >= NUM_VER_MAX) {
        return n;
    }

    int i;
    for (i = n - 1; (i >= 0 && (strcmp(verticesOrdenados[i].chaveCidade, chave) > 0)); i--) {
        strcpy(verticesOrdenados[i + 1].chaveCidade, verticesOrdenados[i].chaveCidade);
        verticesOrdenados[i + 1].numeroArestas = verticesOrdenados[i].numeroArestas;
    }

    strcpy(verticesOrdenados[i + 1].chaveCidade, chave);
    verticesOrdenados[i + 1].numeroArestas = 1;

    return (n + 1);
}

void atualizaRemocaoArqAux(VERTICE* verticesOrdenados, char* verticeRetirado) {
    /*
     * Essa função subtrai o número de vértices do vetor VERTICEs.
     */
    char* chaveGerada;
    int indiceEncontrado;
    int numVertices; //armazena o número de vértices antes de qualquer remoção

    numVertices = verticesOrdenados[0].numVert;

    //busca binária através da chave:
    chaveGerada = geraChaveCidade(verticeRetirado);
    indiceEncontrado = buscaBinaria(verticesOrdenados, chaveGerada, numVertices);

    //remove conexões dos vértices. Se o resultado dessa remoção for zero, remove o próprio vértice
    if (indiceEncontrado != -1) {
        verticesOrdenados[indiceEncontrado].numeroArestas--;
    }

    if ((indiceEncontrado != -1) && (verticesOrdenados[indiceEncontrado].numeroArestas == 0)) {
        verticesOrdenados[0].numVert--;
    }

    free(chaveGerada);
}

int insereVetorArqAux(char* cidade, VERTICE* verticesOrdenados, int numVertices) {
    /*
     * Essa função insere um vértice em um vetor de VERTICEs, que é usado para
     * escrever o cabeçalho.
     */
    char* chaveGerada;
    int indiceEncontrado;

    chaveGerada = geraChaveCidade(cidade);
    indiceEncontrado = buscaBinaria(verticesOrdenados, chaveGerada, numVertices);
    if (indiceEncontrado != -1) { //encontrou
        (verticesOrdenados[indiceEncontrado].numeroArestas)++;
    } else { //não encontrou
        insereOrdenado(verticesOrdenados, numVertices, chaveGerada); //insere ordenado
        numVertices++;
    }

    free(chaveGerada);

    return numVertices; //retorna o tamanho do vetor de VERTICEs após a inserção
}

VERTICE* geraArquivoAuxiliar(REGISTRO_PADRAO* registros) {
    /*
     * Essa função gera um vetor de VERTICEs.
     */

    VERTICE* verticesOrdenados; //contém os vértices ordenados
    int j; //para percorrer os dois vetores
    int numVertices; //armazena o número de vértices

    numVertices = 0;
    j = 0;

    //alocação de um vetor de VERTICES:
    verticesOrdenados = (VERTICE*) calloc(NUM_VER_MAX, sizeof (VERTICE));
    if (verticesOrdenados == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return NULL;
    }

    //faz a busca binária das restantes para decidir se vai inserir ou incrementar:
    while ((j < NUM_REG_MAX) && (strcmp(registros[j].cidadeDestino, "") != 0)) {
        //cada registro possui dois novos vértices em potencial:
        if (registros[j].estadoOrigem[0] != '*') { //só conta se não for um registro excluído
            numVertices = insereVetorArqAux(registros[j].cidadeDestino, verticesOrdenados, numVertices);
            numVertices = insereVetorArqAux(registros[j].cidadeOrigem, verticesOrdenados, numVertices);
        }

        //incrementa para preservar o loop:
        j++;
    }

    verticesOrdenados[0].numVert = numVertices; //armazena o número de vértices

    return verticesOrdenados;
}

REGISTRO_PADRAO* lerCsvPadrao(char nomeArquivo[]) {
    /*
     * Essa função lê o arquivo .csv para um vetor de tipos REGISTRO_PADRAO.
     */

    REGISTRO_PADRAO* registros; //vetor que vai armazenar o conjunto de registros padrão do arquivo
    FILE* descritor; //descritor do arquivo .csv
    char buffer[100];
    int i;

    i = 0;

    //abre o arquivo.csv e verifica se a abertura foi bem sucedida :
    descritor = fopen(nomeArquivo, "r");
    if (descritor == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return NULL;
    }

    //aloca o vetor na heap:
    registros = (REGISTRO_PADRAO*) calloc(NUM_REG_MAX, sizeof (REGISTRO_PADRAO));
    if (registros == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return NULL;
    }

    //Ignora a primeira linha do arquivo:
    fgets(buffer, 100, descritor);

    //lê o restante, linha a linha, do arquivo .csv:
    while ((i < NUM_REG_MAX) && fscanf(descritor, "%2[^,],%2[^,],%d,%100[^,],%100[^,],%100[^\n]", registros[i].estadoOrigem, registros[i].estadoDestino, &registros[i].distancia, registros[i].cidadeOrigem, registros[i].cidadeDestino, registros[i].tempoViagem) != EOF) {
        fgetc(descritor); //consome o \n
        i++; //conta quantos registros foram lidos
    }

    registros[0].numReg = i; //armazena o número total de registros apenas no primeiro

    fclose(descritor);

    return registros;
}

REGISTRO_CABECALHO* geraCabecalho(VERTICE* verticesOrdenados, int numArestas) {
    /*
     * Essa função gera o registro de cabeçalho numa estrutura do tipo REGISTRO_CABECALHO.
     */

    REGISTRO_CABECALHO* cabecalhoGerado;

    //alocação:
    cabecalhoGerado = (REGISTRO_CABECALHO*) calloc(1, sizeof (REGISTRO_CABECALHO));
    if (cabecalhoGerado == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return NULL;
    }

    //preenchendo os campos:
    cabecalhoGerado->status = '1'; //consistente ao criar
    cabecalhoGerado->numeroArestas = numArestas;
    cabecalhoGerado->numeroVertices = verticesOrdenados[0].numVert;
    strcpy(cabecalhoGerado->dataUltimaCompactacao, "00/00/0000"); //o arquivo acabou de ser carregado

    //retorna:
    return cabecalhoGerado;
}

int escreveCabecalho(REGISTRO_CABECALHO* cabecalho, char* nomeDados, char* modo) {
    /*
     * Essa função escreve o cabeçalho no arquivo de dados .bin. Note que o modo de abertura
     * precisa ser determinado por quem chamou a função.
     */

    FILE* descritor;
    int i;
    char lixo = LIXO;

    //abre o arquivo de dados no modo escrita em binário:
    descritor = fopen(nomeDados, modo);
    if (descritor == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return -1;
    }

    //escreve no arquivo:
    fwrite(&(cabecalho->status), sizeof (char), 1, descritor);
    fwrite(&(cabecalho->numeroVertices), sizeof (int), 1, descritor);
    fwrite(&(cabecalho->numeroArestas), sizeof (int), 1, descritor);

    if (strcmp(cabecalho->dataUltimaCompactacao, "00/00/0000") != 0) { //se esse campo não for nulo, escreve ele
        fwrite(&(cabecalho->dataUltimaCompactacao), sizeof (char), 10, descritor);
    } else { //se for nulo, escreve lixo
        for (i = 0; i < 10; i++) {
            fwrite(&lixo, sizeof (char), 1, descritor);
        }
    }

    fclose(descritor);

    return 0;
}

int escreveCampoVariavel(FILE* descritor, char* dados) {
    /*
     * Essa função escreve campos de tamanho variável num arquivo de dados .bin
     * e retorna quantos bytes foram escritos.
     */

    int bytesEscritos;
    char delimitador = DELIMITADOR;

    bytesEscritos = 0;

    if (strcmp(dados, "") != 0) { //se o campo não for nulo, escreve ele
        bytesEscritos += sizeof (char)*fwrite(dados, sizeof (char), strlen(dados), descritor);
    }

    bytesEscritos += sizeof (char)*fwrite(&delimitador, sizeof (char), 1, descritor); //escreve o delimitador

    return bytesEscritos;
}

void escreveRegistroPadrao(REGISTRO_PADRAO registro, FILE* descritor) {
    /*
     * Essa função escreve um registro padrão (ou seja, um registro que não é um registro
     * de cabeçalho) num arquivo a partir da posição corrente que se encontra o cursor.
     */

    int bytesEscritos;
    int j;
    char lixo;

    lixo = LIXO;
    bytesEscritos = 0;

    //escrevendo os campos de tamanho fixo:
    bytesEscritos += sizeof (char)*fwrite(&(registro.estadoOrigem), sizeof (char), 2, descritor);
    bytesEscritos += sizeof (char)*fwrite(&(registro.estadoDestino), sizeof (char), 2, descritor);
    bytesEscritos += sizeof (int)*fwrite(&(registro.distancia), sizeof (int), 1, descritor);

    //escreve os campos de tamanho variável:
    bytesEscritos += escreveCampoVariavel(descritor, registro.cidadeOrigem);
    bytesEscritos += escreveCampoVariavel(descritor, registro.cidadeDestino);
    bytesEscritos += escreveCampoVariavel(descritor, registro.tempoViagem);

    //preenche o resto do registro com lixo:
    for (j = 0; j < TAM_REG_PAD - bytesEscritos; j++) {
        fwrite(&lixo, sizeof (char), 1, descritor);
    }

}

int escreveRegistrosPadrao(REGISTRO_PADRAO* registros, char* nomeDados) {
    /*
     * Essa função utiliza a função acima para escrever um vetor de registros padrão
     * no arquivo de dados.
     */

    FILE* descritor;
    int i;

    i = 0;

    //abre o arquivo de dados no modo escrita em binário:
    descritor = fopen(nomeDados, "ab");
    if (descritor == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        return -1;
    }

    //escreve no arquivo cada registro:
    while ((i < NUM_REG_MAX) && (strcmp(registros[i].cidadeDestino, "") != 0)) { //continua no loop até atingir o máximo de registros que podem existir, ou até os registro acabarem
        escreveRegistroPadrao(registros[i], descritor);
        i++;
    }

    fclose(descritor);

    return 0;
}

char* lerCampoVariavel(FILE* descritor) {
    /*
     * Essa função lê um campo de variável apontado pela posição corrente do
     * cursor associado ao descritor de arquivo passado como parâmetro. Esse
     * campo é retornado pela função como uma string.
     */

    char* campoLido;
    char percorre;
    int i;

    i = 0;

    //alocação:
    campoLido = (char*) calloc(100, sizeof (char));
    if (campoLido == NULL) {
        return NULL;
    }

    //lê o primeiro caractere do campo:
    percorre = fgetc(descritor);

    //preenche o campoLido:
    while ((percorre != '|') && (i < 100)) {
        campoLido[i] = percorre;
        percorre = fgetc(descritor);
        i++;
    }

    return campoLido;
}

REGISTRO_PADRAO* lerRegistro(FILE* descritor) {
    /*
     * Essa função lê o registro apontado pelo cursor do arquivo associado
     * ao descritor passado como parâmetro.
     */

    REGISTRO_PADRAO* registroLido;
    char* campoLido;
    int start_pos = ftell(descritor);

    registroLido = (REGISTRO_PADRAO*) calloc(1, sizeof (REGISTRO_PADRAO));
    if (registroLido == NULL) {
        return NULL;
    }

    //lê os campos de tamanho fixo:
    if (fread(&(registroLido->estadoOrigem), sizeof (char), 2, descritor) != 2) {
        free(registroLido);
        return NULL;
    }
    fread(&(registroLido->estadoDestino), sizeof (char), 2, descritor);
    fread(&(registroLido->distancia), sizeof (int), 1, descritor);

    //lê os campos de tamanho variável:
    campoLido = lerCampoVariavel(descritor);
    strcpy(registroLido->cidadeOrigem, campoLido);
    free(campoLido);

    campoLido = lerCampoVariavel(descritor);
    strcpy(registroLido->cidadeDestino, campoLido);
    free(campoLido);

    campoLido = lerCampoVariavel(descritor);
    strcpy(registroLido->tempoViagem, campoLido);
    free(campoLido);
    fseek(descritor, start_pos + TAM_REG_PAD, SEEK_SET);

    return registroLido;
}

REGISTRO_PADRAO* lerTodosOsRegistros(char* nomeDados) {
    /*
     * Essa função transfere os dados de um arquivo binário para um vetor de
     * REGISTRO_PADRAO
     */

    int i;
    //aloca o vetor que ficará com os dados:
    REGISTRO_PADRAO* registros = (REGISTRO_PADRAO*) calloc(NUM_REG_MAX, sizeof (REGISTRO_PADRAO));
    if (registros == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return NULL;
    }
    REGISTRO_PADRAO* aux;

    //abre o arquivo:
    FILE* descritor = fopen(nomeDados, "rb");
    if (descritor == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return NULL;
    }

    fseek(descritor, TAM_REG_CAB, SEEK_SET); //muda o posição corrente para além do cabeçalho

    aux = lerRegistro(descritor);
    for (i = 0; (i < NUM_REG_MAX) && (aux != NULL); i++) {
        strcpy(registros[i].cidadeDestino, aux->cidadeDestino);
        strcpy(registros[i].cidadeOrigem, aux->cidadeOrigem);
        strcpy(registros[i].estadoDestino, aux->estadoDestino);
        strcpy(registros[i].estadoOrigem, aux->estadoOrigem);
        strcpy(registros[i].tempoViagem, aux->tempoViagem);
        registros[i].distancia = aux->distancia;
        free(aux);
        fseek(descritor, TAM_REG_CAB + TAM_REG_PAD * (i + 1), SEEK_SET);
        aux = lerRegistro(descritor);
    }
    registros[0].numReg = i;
    if(aux != NULL){
        free(aux);
    }

    fclose(descritor);
    return registros;
}

int recuperaRegistro(int RRN, FILE* descritor, int modo) {
    /*
     * Essa função recupera um registro no arquivo de dados e o imprime na tela.
     * modo == 1: se o registro recuperado estiver escolhido não imprime que ele é inexistente.
     * modo == 2: se o registro recuperado estiver escolhido imprime que ele é inexistente.
     */

    REGISTRO_PADRAO* registroRecuperado;

    //posiciona o posição corrente no RRN:
    fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB, SEEK_SET);

    //lê o registro:
    registroRecuperado = lerRegistro(descritor);

    //imprime o registro na tela:
    if (registroRecuperado->estadoOrigem[0] != '*') { //só imprime se o registro não estiver deletado
        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
    } else {
        if (modo == 2) {
            printf("Registro inexistente.\n");
        }
        free(registroRecuperado);
        return -1;
    }

    free(registroRecuperado);
    return 0;
}

void lerCabecalho(REGISTRO_CABECALHO* cabecalho, FILE* descritor) {
    /*
     * Essa função lê o registro de cabeçalho do arquivo de dados e o coloca
     * na posição de memória apontada pelo ponteiro "cabecalho".
     */

    cabecalho->status = fgetc(descritor);
    fread(&(cabecalho->numeroVertices), sizeof (int), 1, descritor);
    fread(&(cabecalho->numeroArestas), sizeof (int), 1, descritor);
    fread(&(cabecalho->dataUltimaCompactacao), sizeof (char), 10, descritor);

    return;
}

int recuperaTodosOsRegistros(char* nomeDados) { //é como a função acima, mas recupera todos os registros no arquivo de dados e os imprime
    /*
     * Essa função é como a "recuperaRegistro", mas itera por todos os registros,
     * de modo a recuperar um por um.
     */

    REGISTRO_CABECALHO* cabecalho; //precisaremos dele para determinar quantos registros existem no arquivo de dados
    FILE* descritor;
    int i;
    int rrn = 0;

    //lê o registro de cabeçalho:
    cabecalho = (REGISTRO_CABECALHO*) calloc(1, sizeof (REGISTRO_CABECALHO));
    if (cabecalho == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    //abre o arquivo:
    descritor = fopen(nomeDados, "rb");
    if (descritor == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    lerCabecalho(cabecalho, descritor);

    if (cabecalho->status == '1') { //consistente
        if (cabecalho->numeroArestas == 0) { //caso o arquivo não possua nenhum registro
            printf("Registro inexistente.\n");
            return 0;
        }
        for (i = 0; i < cabecalho->numeroArestas; i++) {
            if (recuperaRegistro(rrn, descritor, 1) == -1) {
                i--;
            }
            rrn++;
        }
    } else { //inconsistente
        printf("Falha no processamento do arquivo.\n");
    }

    free(cabecalho);
    fclose(descritor);
    return 0;
}

void removeRegistro(int RRN, FILE* descritor) {
    /*
     * Essa função realiza a remoção lógica do registro apontado pela posição
     * corrente do cursor de um arquivo de dados.
     */

    char marcaRemovido = '*';
    char inconsistente = '0';

    //marca o cabeçalho do arquivo como inconsistente
    fseek(descritor, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof (char), 1, descritor);

    //localiza o registro que se deseja remover:
    fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB, SEEK_SET);

    //realiza a remoção lógica:
    fwrite(&marcaRemovido, sizeof (char), 1, descritor);
}

int comparaRegistro(int RRN, FILE* descritor, char* nomeCampo, char* valorCampo, int modo, VERTICE* verticesOrdenados, REGISTRO_CABECALHO* cabecalho) {
    /*
     * Essa função compara um valor de um determinado campo pertencente a um registro com
     * um outro valor dado. Se os dois forem iguais, a ação tomada varia de acordo com o
     * o modo em que a função foi chamada.
     *
     * Possui dois modos:
     *
     * -modo 1: imprime o registro;
     * -modo 2: deleta o registro.
     */

    REGISTRO_PADRAO* registroRecuperado;

    //posiciona o posição corrente no RRN:
    fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB, SEEK_SET);

    //lê o registro:
    registroRecuperado = lerRegistro(descritor);

    //faz a comparação de acordo com o campo que está sendo comparado:
    if (registroRecuperado->estadoOrigem[0] != '*') { //só prossegue se o registro não for deletado
        if (strcmp(nomeCampo, "estadoOrigem") == 0) {
            if (strcmp(registroRecuperado->estadoOrigem, valorCampo) == 0) { //se o valor do campo coincidir, imprime o registro
                switch (modo) {
                    case 1: //imprime o registro
                        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
                        free(registroRecuperado);

                        return 1;
                    case 2: //deleta o registro
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeDestino);
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeOrigem);
                        removeRegistro(RRN, descritor);
                        free(registroRecuperado);

                        return 1;
                }
            }

            free(registroRecuperado);
            return 0;
        }
        if (strcmp(nomeCampo, "estadoDestino") == 0) {
            if (strcmp(registroRecuperado->estadoDestino, valorCampo) == 0) {
                switch (modo) {
                    case 1:
                        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
                        free(registroRecuperado);

                        return 1;
                    case 2:
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeDestino);
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeOrigem);
                        removeRegistro(RRN, descritor);
                        free(registroRecuperado);

                        return 1;
                }

            }

            free(registroRecuperado);
            return 0;
        }
        if (strcmp(nomeCampo, "distancia") == 0) {
            if (registroRecuperado->distancia == atoi(valorCampo)) {
                switch (modo) {
                    case 1:
                        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
                        free(registroRecuperado);

                        return 1;
                    case 2:
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeDestino);
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeOrigem);
                        removeRegistro(RRN, descritor);
                        free(registroRecuperado);

                        return 1;
                }

            }

            free(registroRecuperado);
            return 0;
        }
        if (strcmp(nomeCampo, "cidadeOrigem") == 0) {
            if (strcmp(registroRecuperado->cidadeOrigem, valorCampo) == 0) {
                switch (modo) {
                    case 1:
                        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
                        free(registroRecuperado);

                        return 1;
                    case 2:
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeDestino);
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeOrigem);
                        removeRegistro(RRN, descritor);
                        free(registroRecuperado);

                        return 1;
                }

            }

            free(registroRecuperado);
            return 0;
        }
        if (strcmp(nomeCampo, "cidadeDestino") == 0) {
            if (strcmp(registroRecuperado->cidadeDestino, valorCampo) == 0) {
                switch (modo) {
                    case 1:
                        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
                        free(registroRecuperado);

                        return 1;
                    case 2:
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeDestino);
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeOrigem);
                        removeRegistro(RRN, descritor);
                        free(registroRecuperado);

                        return 1;
                }

            }

            free(registroRecuperado);
            return 0;
        }
        if (strcmp(nomeCampo, "tempoViagem") == 0) {
            if (strcmp(registroRecuperado->tempoViagem, valorCampo) == 0) {
                switch (modo) {
                    case 1:
                        printf("%d %s %s %d %s %s %s\n", RRN, registroRecuperado->estadoOrigem, registroRecuperado->estadoDestino, registroRecuperado->distancia, registroRecuperado->cidadeOrigem, registroRecuperado->cidadeDestino, registroRecuperado->tempoViagem);
                        free(registroRecuperado);

                        return 1;
                    case 2:
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeDestino);
                        atualizaRemocaoArqAux(verticesOrdenados, registroRecuperado->cidadeOrigem);
                        removeRegistro(RRN, descritor);
                        free(registroRecuperado);

                        return 1;
                }

            }

            free(registroRecuperado);
            return 0; //se uma das comparações der certo
        }
    } else {
        cabecalho->numeroArestas++;
    }
    free(registroRecuperado);
    return 0; //se as comparações não derem certo
}

int comparaTodosOsRegistros(char* nomeDados, char* nomeCampo, char* valorCampo, int modo) {
    /*
     * Essa função itera por todos os registros do arquivo de dados, comparando o valor
     * de um determinado campo com um outro valor dado. Se os dois forem iguais, o registro é
     * impresso na tela.
     *
     * modos:
     * 1 - imprime os registros cuja comparação foi positiva
     * 2 - deleta do arquivo de dados os registros cuja comparação foi positiva
     */

    REGISTRO_PADRAO* registrosPadrao;
    REGISTRO_CABECALHO* cabecalho; //precisaremos dele para determinar quantos registros existem no arquivo de dados
    FILE* descritor;
    VERTICE* verticesOrdenados; //armazena o conteúdo do arquivo auxiliar
    int i;
    int comparacoesCertas = 0; //armazena o número de comparações que deram certo
    int numArestas;
    char modoAbertura[5]; //armazena o modo de abertura do arquivo para gerar o cabeçalho
    int nroReal;

    limparString(modoAbertura, 5);

    //lê o arquivo auxiliar:
    if (modo == 2) { //só vai precisar reconstruir o cabeçalho no modo 2
        registrosPadrao = lerTodosOsRegistros(nomeDados);
        if (registrosPadrao == NULL) {
            return -1;
        }
        verticesOrdenados = geraArquivoAuxiliar(registrosPadrao);
    }

    //lê o registro de cabeçalho:
    cabecalho = (REGISTRO_CABECALHO*) calloc(1, sizeof (REGISTRO_CABECALHO));
    if (cabecalho == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    //abre o arquivo:
    descritor = fopen(nomeDados, "rb+");
    if (descritor == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    lerCabecalho(cabecalho, descritor);

    nroReal = cabecalho->numeroArestas;
    if (cabecalho->status == '1') { //consistente
        for (i = 0; i < cabecalho->numeroArestas; i++) {
            comparacoesCertas += comparaRegistro(i, descritor, nomeCampo, valorCampo, modo, verticesOrdenados, cabecalho);
        }
    } else { //inconsistente
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    fclose(descritor);

    if (modo == 2) {
        free(registrosPadrao);

        //gera um outro cabeçalho:
        numArestas = nroReal - comparacoesCertas;
        free(cabecalho);
        cabecalho = geraCabecalho(verticesOrdenados, numArestas);
        free(verticesOrdenados);

        //escreve esse outro cabeçalho:
        strcpy(modoAbertura, "rb+");
        escreveCabecalho(cabecalho, nomeDados, modoAbertura);

        //free(registrosPadrao);
    }

    free(cabecalho);
    return comparacoesCertas;
}

int main(int argc, char** argv) {
    //-->DECLARAÇÕES DE VARIÁVEIS:

    int funcionalidade; //armazena a funcionalidade que o usuário deseja executar
    char nomeCsv[25]; //armazena o nome do arquivo .csv
    char nomeDados[25]; //armazena o nome do arquivo de dados
    char nomeOut[25]; //armazena o nome de um possivel arquivo de dados
    char nomeCampo[15]; //armazena o nome de um dos campos de um registro
    char valorCampo[25]; //armazena um possível valor de um campo
    char* tempCidadeOrigem; //armazena um possível valor de um campo
    char* tempCidadeDest; //armazena um possível valor de um campo
    char* tempTempo; //armazena um possível valor de um campo
    char modoAbertura[5]; //define o modo de abertura para escrever o cabeçalho
    int RRN = 0;
    int i;
    int nVezes; //número de vezes que uma funcionalidade deve ser executada
    REGISTRO_PADRAO* registros; //vetor que vai armazenar o conjunto de registros padrão do arquivo de dados
    REGISTRO_PADRAO* aux;
    VERTICE* verticesOrdenados;
    REGISTRO_CABECALHO* cabecalho;
    FILE* descritor; //descritor do arquivo de texto quando for preciso
    int flag;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    //-->LÓGICA PRINCIPAL:

    //preenche strings com '\0':
    limparString(nomeCsv, 25);
    limparString(nomeDados, 25);
    limparString(nomeOut, 25);
    limparString(nomeCampo, 15);
    limparString(valorCampo, 25);
    limparString(modoAbertura, 5);

    //pega a funcionalidade que o usuário deseja executar:
    scanf("%d", &funcionalidade);

    //sabemos que entrada_inicial[0] guarda a opção de funcionalidade, por isso, fazemos um switch nesse caractere:
    switch (funcionalidade) {

        case 1: // Funcionalidade 1: Converte um arquivo CSV em um arquivo binário
            {
                //obtém os nomes dos arquivos .csv e de dados da entrada inicial do usuário:
                scanf("%s", nomeCsv);
                scanf("%s", nomeDados);

                //lê o arquivo .csv:
                registros = lerCsvPadrao(nomeCsv);
                if (registros == NULL) {
                    return 0; //código de erro
                }

                //gera arquivo auxiliar (para contabilizar o número de vértices):
                verticesOrdenados = geraArquivoAuxiliar(registros);
                if (verticesOrdenados == NULL) {
                    free(registros);
                    return 0;
                }

                //gera o cabecalho:
                cabecalho = geraCabecalho(verticesOrdenados, registros[0].numReg);
                if (cabecalho == NULL) {
                    free(registros);
                    free(verticesOrdenados);
                    return 0;
                }

                //escreve o cabecalho no arquivo de dados:
                strcpy(modoAbertura, "wb");
                flag = escreveCabecalho(cabecalho, nomeDados, modoAbertura);
                if (flag == -1) {
                    free(registros);
                    free(verticesOrdenados);
                    free(cabecalho);
                    return 0;
                }

                //escreve o restante do arquivo:
                flag = escreveRegistrosPadrao(registros, nomeDados);
                if (flag == -1) {
                    free(registros);
                    free(verticesOrdenados);
                    free(cabecalho);
                    return 0;
                }

                //função do matheus:
                binarioNaTela1(nomeDados);

                free(registros);
                free(verticesOrdenados);
                free(cabecalho);

                break;
            }

        case 2: // Funcionalidade 2: Mostra todos os registros na tela. 
            {
                //recolhe a entrada do usuário e executa a funcionalidade:
                scanf("%s", nomeDados);
                flag = recuperaTodosOsRegistros(nomeDados);
                if (flag == -1) {
                    return 0;
                }

                break;
            }

        case 3: // Funcionalidade 3: Mostra na tela os registros que satisfazem um critério de busca.  
            {
                //obtém os parâmetros:
                scanf("%s", nomeDados);
                scanf("%s", nomeCampo);
                scan_quote_string(valorCampo);

                //executa a função responsável pela funcionalidade:
                flag = comparaTodosOsRegistros(nomeDados, nomeCampo, valorCampo, 1);
                if (flag == -1) {
                    return 0;
                }
                if (flag == 0) { //se o número de comparações certas for zero, o registro não existe ou foi removido
                    printf("Registro inexistente.\n");
                }

                break;
            }


        case 4: // Funcionalidade 4: Busca o registro em um RRN e o exibe na tela.
            {
                //obtém os parâmetros:
                scanf("%s", nomeDados);
                scanf("%d", &RRN);

                //abre o arquivo de dadosf
                descritor = fopen(nomeDados, "rb");
                if (descritor == NULL) {
                    printf("Falha no processamento do arquivo.\n");
                    return 0;
                }

                //le o cabecalho do arquivo de dados:
                cabecalho = (REGISTRO_CABECALHO*) calloc(1, sizeof (REGISTRO_CABECALHO));
                if (cabecalho == NULL) {
                    printf("Falha no processamento do arquivo.\n");
                    fclose(descritor);
                    return 0;
                }
                lerCabecalho(cabecalho, descritor);
                if (cabecalho->status == '1') { //consistente
                    if (RRN >= cabecalho->numeroArestas) { //checa se o RRN digitado está dentro dos limites
                        printf("Registro inexistente.\n");
                        free(cabecalho);
                        fclose(descritor);
                        break;
                    }
                } else {
                    printf("Falha no processamento do arquivo.\n");
                }

                //executa a funcionalidade:
                recuperaRegistro(RRN, descritor, 2);
                fclose(descritor);
                free(cabecalho);

                break;
            }


        case 5: // Funcionalidade 5: Remove registros que satisfazem um critério de busca.
            {
                //obtém os parâmetros:
                scanf("%s", nomeDados);
                scanf("%d", &nVezes);

                for (i = 0; i < nVezes; i++) {
                    scanf("%s", nomeCampo);
                    scan_quote_string(valorCampo);
                    if(strcmp(valorCampo, "NULO") == 0){
                        limparString(valorCampo, 25);
                    }

                    flag = comparaTodosOsRegistros(nomeDados, nomeCampo, valorCampo, 2);
                    if (flag == -1) {
                        return 0;
                    }
                }

                binarioNaTela1(nomeDados);

                break;
            }


        case 6: // Funcionalidade 6: Insere n registros a partir da entrada.
            {
                //obtém os parâmetros:
                scanf("%s", nomeDados);
                scanf("%d", &nVezes);

                /* aux = calloc(1, sizeof(REGISTRO_PADRAO)); */
                /* descritor = fopen(nomeDados, "rb+"); */
                /* if (descritor == NULL) { */
                /*     printf("Falha no processamento do arquivo.\n"); */
                /*     return -1; */
                /* } */
                /* fseek(descritor, 0, SEEK_END); */
                /* for (i = 0; i < nVezes; i++) { */
                /*     scanf("%s", aux -> estadoOrigem); */
                /*     scanf("%s", aux -> estadoDestino); */
                /*     scanf("%s", valorCampo); */
                /*     if (valorCampo[0] == 'n' || valorCampo[0] == 'N') */
                /*         aux -> distancia = -1; */
                /*     else aux -> distancia = atoi(valorCampo); */
                /*     scan_quote_string(aux -> cidadeOrigem); */
                /*     scan_quote_string(aux -> cidadeDestino); */
                /*     scan_quote_string(aux -> tempoViagem); */
                /*     escreveRegistroPadrao(*aux, descritor); */
                /* } */
                /* fclose(descritor); */
                /* free(registros); */

                registros = lerTodosOsRegistros(nomeDados);
                for (i = 1; i <= nVezes; i++) {
                    aux = calloc(1, sizeof(REGISTRO_PADRAO));
                    scanf("%s", aux -> estadoOrigem);
                    scanf("%s", aux -> estadoDestino);
                    scanf("%s", valorCampo);
                    if (valorCampo[0] == 'n' || valorCampo[0] == 'N')
                        aux -> distancia = -1;
                    else aux -> distancia = atoi(valorCampo);
                    scan_quote_string(aux -> cidadeOrigem);
                    scan_quote_string(aux -> cidadeDestino);
                    scan_quote_string(aux -> tempoViagem);

                    registros[registros[0].numReg++] = *aux;
                    free(aux);
                }
                verticesOrdenados = geraArquivoAuxiliar(registros);
                cabecalho = geraCabecalho(verticesOrdenados, registros[0].numReg);
                flag = escreveCabecalho(cabecalho, nomeDados, "wb");
                flag = escreveRegistrosPadrao(registros, nomeDados);
                binarioNaTela1(nomeDados);
                break;

                
            }


        case 7: // Funcionalidade 7: Atualiza um campo específico de um registro.
            {
                //obtém os parâmetros:
                scanf("%s", nomeDados);
                scanf("%d", &nVezes);
                descritor = fopen(nomeDados, "rb+");
                if (descritor == NULL) {
                    printf("Falha no processamento do arquivo.\n");
                    return -1;
                }
                //le o cabecalho do arquivo de dados:
                cabecalho = (REGISTRO_CABECALHO*) calloc(1, sizeof (REGISTRO_CABECALHO));
                if (cabecalho == NULL) {
                    printf("Falha no processamento do arquivo.\n");
                    fclose(descritor);
                    return 0;
                }
                lerCabecalho(cabecalho, descritor);

                for (i = 0; i < nVezes; i++) {
                    scanf("%d", &RRN);
                    if (RRN >= cabecalho->numeroArestas) { //checa se o RRN digitado está dentro dos limites
                        scanf("%s", nomeCampo);
                        scan_quote_string(valorCampo);
                        continue;
                    }
                    scanf("%s", nomeCampo);
                    scan_quote_string(valorCampo);
                    fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB, SEEK_SET);
                    if (!strcmp(nomeCampo, "estadoOrigem")){
                        if(strcmp(valorCampo, "NULO") == 0){
                            fputc('\0', descritor);
                            fputc('#', descritor);
                        }
                        fputs(valorCampo, descritor);
                        continue;
                    }
                    if (!strcmp(nomeCampo, "estadoDestino")){
                        fseek(descritor, 2, SEEK_CUR);
                        fputs(valorCampo, descritor);
                        continue;
                    }
                    if (!strcmp(nomeCampo, "distancia")){
                        fseek(descritor, 4, SEEK_CUR);
                        int val = (int) strtol(valorCampo, (char **)NULL, 10);
                        fwrite(&val, 4, 1, descritor);
                        continue;
                    }

                    fseek(descritor, 8, SEEK_CUR);
                    tempCidadeOrigem = lerCampoVariavel(descritor);
                    tempCidadeDest = lerCampoVariavel(descritor);
                    tempTempo = lerCampoVariavel(descritor);

                    if (!strcmp(nomeCampo, "cidadeOrigem")){
                        fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB + 8, SEEK_SET);
                        escreveCampoVariavel(descritor, valorCampo);
                        escreveCampoVariavel(descritor, tempCidadeDest);
                        escreveCampoVariavel(descritor, tempTempo);
                    }
                    if (!strcmp(nomeCampo, "cidadeDestino")){
                        fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB + 8, SEEK_SET);
                        escreveCampoVariavel(descritor, tempCidadeOrigem);
                        escreveCampoVariavel(descritor, valorCampo);
                        escreveCampoVariavel(descritor, tempTempo);
                    }
                    if (!strcmp(nomeCampo, "tempoViagem")){
                        fseek(descritor, RRN * TAM_REG_PAD + TAM_REG_CAB + 8, SEEK_SET);
                        escreveCampoVariavel(descritor, tempCidadeOrigem);
                        escreveCampoVariavel(descritor, tempCidadeDest);
                        escreveCampoVariavel(descritor, valorCampo);
                    }

                    free(tempCidadeOrigem);
                    free(tempCidadeDest);
                    free(tempTempo);

                }
                fclose(descritor);
                registros = lerTodosOsRegistros(nomeDados);
                verticesOrdenados = geraArquivoAuxiliar(registros);
                cabecalho = geraCabecalho(verticesOrdenados, registros[0].numReg);
                descritor = fopen(nomeDados, "rb+");
                fseek(descritor, 1, SEEK_SET);
                fwrite(&(cabecalho->numeroVertices), 4, 1, descritor);
                fclose(descritor);

                

                //função do matheus:
                binarioNaTela1(nomeDados);
                break;
            }


        case 8: // Funcionalidade 8: Compacta o arquivo binário.
            {
                //obtém os parâmetros:
                scanf("%s", nomeDados);
                scanf("%s", nomeOut);

                cabecalho = malloc(sizeof(REGISTRO_CABECALHO));
                if (cabecalho == NULL){
                    printf("Falha no carregamento do arquivo.");
                    return 0;
                }
                

                descritor = fopen(nomeDados, "rb");
                if (descritor == NULL){
                    printf("Falha no carregamento do arquivo.");
                    return 0;
                }
                
                lerCabecalho(cabecalho, descritor);
                if (cabecalho->status != '1') { //inconsistente
                    printf("Falha no carregamento do arquivo.");
                    return 0;
                }

                // Substitui a data da ultima compactação pela data de hoje.
                char cur_date[11];
                /* sprintf(cur_date, "%02d/%d/%d",tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900); */
                sprintf(cur_date, "01/11/2019");
                strcpy(cabecalho->dataUltimaCompactacao, cur_date);

                //escreve o cabecalho no arquivo de dados:
                flag = escreveCabecalho(cabecalho, nomeOut, "wb");
                if (flag == -1) {
                    free(aux);
                    return 0;
                }
                free(cabecalho);

                FILE* output = fopen(nomeOut, "ab");
                if (output == NULL){
                    printf("Falha no carregamento do arquivo.");
                    return 0;
                }
                fseek(descritor, TAM_REG_CAB, SEEK_SET); //muda o posição corrente para além do cabeçalho
                while ((aux = lerRegistro(descritor)) != NULL){
                    if (aux ->estadoOrigem[0] != '*'){
                        escreveRegistroPadrao(*aux, output);
                    }
                    free(aux);
                }

                fclose(descritor);
                fclose(output);

                //função do matheus:
                binarioNaTela1(nomeOut);

                break;
            }
    }

    return 0;
}
