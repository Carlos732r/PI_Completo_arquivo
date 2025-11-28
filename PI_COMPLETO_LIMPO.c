#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#include <conio.h>

// ----------------------------------------------------------------------
// --- 1. ESTRUTURAS DE DADOS ---
// ----------------------------------------------------------------------

typedef struct{
	int numero;
	char rua[60], bairro[60], cidade [60], uf[30];
}ENDERECO;

typedef struct{
	int dia, mes, ano;
}NASCIMENTO;

typedef struct{
	int ddd, telefone;
}CONTATO;

// ESTRUTURA CLIENTE
typedef struct{
		char nomeCliente[50], CPF [20]; 
		ENDERECO end;
		NASCIMENTO niver;
		CONTATO tel;
}CADASTRO;

// ESTRUTURA LABORATORIO
typedef struct{
	char nomeLaboratorio[50], CNPJ[20],nomeResponsavel[50]; 
	ENDERECO end;
	CONTATO tel;
}LABORATORIO;

// ESTRUTURA PRODUTO
typedef struct{
    char codigo[20], nomeProduto[50], lote[30];
    int quantidade;
    float precoCusto, precoVenda;
    NASCIMENTO dataValidade; 
    char cnpjLaboratorio[20]; // Chave estrangeira
}PRODUTO;

// ESTRUTURA CATEGORIA
typedef struct{
    int idCategoria;
    char nomeCategoria[50];
}CATEGORIA;

// ESTRUTURA PROMOCAO
typedef struct{
    int idPromocao;
    char nomePromocao[50];
    float desconto; 
    NASCIMENTO dataInicio, dataFim;
    char codigoProduto[20];
}PROMOCAO;

// ESTRUTURA VENDA (Relat?rios)
typedef struct{
    int idVenda;
    char cpfCliente[20]; 
    char codigoProduto[20];
    int quantidadeVendida;
    float precoTotal;
    NASCIMENTO dataVenda;
}VENDA;


// ----------------------------------------------------------------------
// --- 2. FUN??ES AUXILIARES DE BUSCA E VALIDA??O ---
// ----------------------------------------------------------------------

// Fun??o de busca (Cliente)
// Retorna a posi??o inicial do registro no arquivo ou -1 se n?o encontrado.
int busca_cliente(FILE *arquivo, char CPF[])
{
	CADASTRO cad;
	rewind(arquivo);
	while(fread(&cad, sizeof(CADASTRO), 1, arquivo) == 1)
	{
		if(stricmp(CPF, cad.CPF) == 0)
		{
			int pos = ftell(arquivo) - sizeof(CADASTRO); 
			return pos;
		}
	}
	return -1;
}

int laboratorio_existe(char CNPJ[]) {
	FILE *arq_lab = fopen("laboratorio.bin", "rb");
    if (arq_lab == NULL) {
        return 0; 
    }

    LABORATORIO lab;
    int encontrado = 0;
    
    rewind(arq_lab);
    
    while (fread(&lab, sizeof(LABORATORIO), 1, arq_lab) == 1) {
        if (stricmp(CNPJ, lab.CNPJ) == 0) {
            encontrado = 1;
        }
    }
    
    fclose(arq_lab);
    return encontrado;
}

int eh_bissexto(int ano) {
    // Retorna 1 (verdadeiro) se for bissexto, 0 (falso) caso contr?rio
    return (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
}

// Retorna o n?mero de dias de um m?s espec?fico
int dias_no_mes(int mes, int ano) {
    if (mes < 1 || mes > 12) return 0; // Evita erro se o m?s for inv?lido

    if (mes == 2) {
        return eh_bissexto(ano) ? 29 : 28;
    } 
    // Meses com 30 dias: abril (4), junho (6), setembro (9), novembro (11)
    else if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
        return 30;
    } 
    // Demais meses (1, 3, 5, 7, 8, 10, 12) t?m 31 dias
    else {
        return 31;
    }
}

// Converte data para long para compara??o
long data_to_long(NASCIMENTO data) {
    return (long)data.ano * 10000 + (long)data.mes * 100 + (long)data.dia;
}
NASCIMENTO subtrair_dias_data(NASCIMENTO data_base, int dias_limite) {
    NASCIMENTO resultado = data_base;
    
    resultado.dia -= dias_limite;

    while (resultado.dia <= 0) {
        resultado.mes -= 1;

        if (resultado.mes <= 0) {
            resultado.ano -= 1;
            resultado.mes = 12;
        }
        resultado.dia += dias_no_mes(resultado.mes, resultado.ano);
    }
    return resultado;
}
NASCIMENTO somar_dias_data(NASCIMENTO data_base, int dias_limite) {
    NASCIMENTO resultado = data_base;
    
    resultado.dia += dias_limite;

    int max_dias;
    
    while (resultado.dia > (max_dias = dias_no_mes(resultado.mes, resultado.ano))) {
        
        resultado.dia -= max_dias;
        
        resultado.mes += 1;

        if (resultado.mes > 12) {
            resultado.ano += 1;
            resultado.mes = 1; 
        }
    }
    return resultado;
}

int busca_produto(FILE *arquivo, char codigo[])
{
	PRODUTO prod;
	rewind(arquivo);
	while(fread(&prod, sizeof(PRODUTO), 1, arquivo) == 1)
	{
		if(stricmp(codigo, prod.codigo) == 0)
		{
			int pos = ftell(arquivo) - sizeof(PRODUTO); 
			return pos;
		}
	}
	return -1;
}

int busca_categoria(FILE *arquivo, int id)
{
	CATEGORIA cat;
	rewind(arquivo);
	while(fread(&cat, sizeof(CATEGORIA), 1, arquivo) == 1)
	{
		if(id == cat.idCategoria)
		{
			int pos = ftell(arquivo) - sizeof(CATEGORIA); 
			return pos;
		}
	}
	return -1;
}

int busca_promocao(FILE *arquivo, int id)
{
	PROMOCAO prm;
	rewind(arquivo);
	while(fread(&prm, sizeof(PROMOCAO), 1, arquivo) == 1)
	{
		if(id == prm.idPromocao)
		{
            // Removido o cast (int)
			int pos = ftell(arquivo) - sizeof(PROMOCAO); 
			return pos;
		}
	}
	return -1;
}


// ----------------------------------------------------------------------
// --- 3. M?DULO GERENCIAR CLIENTES ---
// ----------------------------------------------------------------------

int menu_cliente()
{
	int opc;
	system("cls");
	printf("\n--------------Gerenciar Clientes--------------");
	printf("\n1 - Cadastrar novo cliente");
	printf("\n2 - Listar todos clientes");
	printf("\n3 - Alterar dados do cliente ");
	printf("\n4 - Deletar cliente ");
	printf("\n5 - Ordenar cliente (por Nome)");
	printf("\n0 - Voltar menu principal ");
	printf("\nSelecione a opcao: ");
	scanf("%d", &opc);
	return opc;	
}

void cadastrar_cliente() {
    FILE *arquivo;
    CADASTRO cad;
    int pos;

    arquivo = fopen("cliente.bin", "ab+");
    if (arquivo == NULL) {
        printf("Erro no arquivo");
        system("pause");
        return; 
    }

    do {
        system("cls");
        printf("\n--- Cadastrar Cliente ---");
        printf("\nCPF: "); fflush(stdin);
        gets(cad.CPF);

        if (strlen(cad.CPF) == 0) break;

        pos = busca_cliente(arquivo, cad.CPF);
        if (pos == -1) {
            printf("\nNome: "); fflush(stdin); gets(cad.nomeCliente);
            printf("\nData Nascimento (Dia Mes Ano): "); scanf("%d %d %d", &cad.niver.dia, &cad.niver.mes, &cad.niver.ano);
            printf("\nDDD: "); scanf("%d", &cad.tel.ddd);
            printf("\nTelefone: "); scanf("%d", &cad.tel.telefone);
            printf("\nRua: "); fflush(stdin); gets(cad.end.rua);
            printf("\nNumero: "); scanf("%d", &cad.end.numero);
            printf("\nBairro: "); fflush(stdin); gets(cad.end.bairro);
            printf("\nCidade: "); fflush(stdin); gets(cad.end.cidade);
            printf("\nUF: "); fflush(stdin); gets(cad.end.uf);

            fwrite(&cad, sizeof(CADASTRO), 1, arquivo);
            printf("\nCliente cadastrado com sucesso! ");
        } else {
            printf("\nCliente com CPF %s ja cadastrado.", cad.CPF);
        }
        printf("\n\nDeseja Continuar Cadastrando (S/N)? ");
    } while (toupper(getche()) == 'S');

    fclose(arquivo);
    system("cls");
    system("pause");
}

void exibir_cliente() {
    FILE *arquivo;
    CADASTRO cad;
    int count = 0;

    arquivo = fopen("cliente.bin", "rb");
    if (arquivo == NULL) {
        printf("\nNenhum cliente cadastrado.");
    } else {
        system("cls");
        printf("--- Clientes Cadastrados ---\n");
        while (fread(&cad, sizeof(CADASTRO), 1, arquivo) == 1) {
            count++;
            printf("\nNome: %s", cad.nomeCliente);
            printf("\nCPF: %s", cad.CPF);
            printf("\nTel: (%d) %d", cad.tel.ddd, cad.tel.telefone);
            printf("\nEndereco: R. %s, %d, %s - %s/%s", cad.end.rua, cad.end.numero, cad.end.bairro, cad.end.cidade, cad.end.uf);
            printf("\n--------------------------------------");
        }
        if (count == 0) printf("\nNenhum cliente encontrado no arquivo.");
        
        fclose(arquivo); 
    }
    system("pause");
    system("cls");
}

void alterar_cliente() {
    FILE *arquivo;
    CADASTRO cad;
    int pos, op;

    arquivo = fopen("cliente.bin", "rb+");
    if (arquivo == NULL) {
        printf("Erro no arquivo");
        system("pause");
        return; 
    }

    system("cls");
    printf("Digite o CPF do cliente que sera alterado (deixe vazio para sair): "); fflush(stdin);
    gets(cad.CPF);

    while (strlen(cad.CPF) != 0) {
        pos = busca_cliente(arquivo, cad.CPF);
        if (pos == -1) {
            printf("Cliente nao encontrado\n");
        } else {
            fseek(arquivo, pos, 0);
            fread(&cad, sizeof(CADASTRO), 1, arquivo);
            printf("\n--------------------------");
            printf("\nNome atual: %s", cad.nomeCliente);
            printf("\nTel atual: (%d) %d", cad.tel.ddd, cad.tel.telefone);

            printf("\nDeseja alterar \n1 - Contato \n2 - Endereco ? ");
            scanf("%d", &op);

            if (op == 1) {
                printf("\nNovo DDD: "); scanf("%d", &cad.tel.ddd);
                printf("\nNovo Telefone: "); scanf("%d", &cad.tel.telefone);
                printf("\nDados alterados com sucesso!\n");
            } else if (op == 2) {
                printf("\nNova Rua: "); fflush(stdin); gets(cad.end.rua);
                printf("\nNovo Numero: "); scanf("%d", &cad.end.numero);
                printf("\nNovo Bairro: "); fflush(stdin); gets(cad.end.bairro);
                printf("\nNova Cidade: "); fflush(stdin); gets(cad.end.cidade);
                printf("\nNova UF: "); fflush(stdin); gets(cad.end.uf);
                printf("\nDados alterados com sucesso!\n");
            } else {
                printf("\nAlteracao cancelada.\n");
            }

            fseek(arquivo, pos, 0);
            fwrite(&cad, sizeof(CADASTRO), 1, arquivo);
        }
        system("pause");
        system("cls");
        printf("\nQual o CPF para alterar (deixe vazio para sair): "); fflush(stdin);
        gets(cad.CPF);
    }

    fclose(arquivo);
    system("cls");
    system("pause");
}

void excluir_cliente() {
    FILE *arquivo, *temp;
    CADASTRO cad;
    char cpf[20];
    int pos;
    int arquivo_fechado_interno = 0;

    arquivo = fopen("cliente.bin", "rb");
    if (arquivo == NULL) {
        printf("\nNenhum arquivo de cliente encontrado para exclusao.");
        system("pause");
        return; 
    }

    system("cls");
    printf("Informe o CPF do cliente que deseja excluir: "); fflush(stdin);
    gets(cpf);

    if (strlen(cpf) != 0) {
        pos = busca_cliente(arquivo, cpf);
        if (pos == -1) {
            printf("Cliente nao encontrado");
        } else {
            fseek(arquivo, pos, 0);
            fread(&cad, sizeof(CADASTRO), 1, arquivo);

            printf("\nNome: %s", cad.nomeCliente);
            printf("\nCPF: %s", cad.CPF);

            printf("\nDeseja Excluir S/N? ");
            if (toupper(getche()) == 'S') {
                temp = fopen("auxiliar_cli.bin", "wb");
                if (temp == NULL) {
                    printf("\nErro ao criar arquivo auxiliar. Exclusao cancelada.\n");
                } else {
                    rewind(arquivo);

                    while (fread(&cad, sizeof(CADASTRO), 1, arquivo) == 1) {
                        if (stricmp(cpf, cad.CPF) != 0) {
                            fwrite(&cad, sizeof(CADASTRO), 1, temp);
                        }
                    }

                    fclose(arquivo);
                    fclose(temp);
                    arquivo_fechado_interno = 1;

                    remove("cliente.bin");
                    rename("auxiliar_cli.bin", "cliente.bin");
                    printf("\n\nProcesso de exclusao concluido.\n");
                }
            } else {
                printf("\n\nExclusao cancelada pelo usuario.\n");
            }
        }
    } else {
        printf("\n\nCPF nao informado. Exclusao cancelada.\n");
    }
    
    if (arquivo != NULL && arquivo_fechado_interno == 0) {
        fclose(arquivo);
    }

    system("pause");
    system("cls");
}

void ordenar_cliente()
{
    FILE *arquivo;
    CADASTRO cad, cad_aux;
    int i, qtde, passo = 0;
    
    arquivo = fopen("cliente.bin", "rb+");
    if (arquivo == NULL)
    {
        printf("\nErro no arquivo ou nenhum cliente cadastrado.");
        system("pause");
        return; 
    }

    fseek(arquivo, 0, 2);
    qtde = ftell(arquivo) / sizeof(CADASTRO); 
    
    if (qtde <= 1) {
        printf("\nNao ha necessidade de ordenacao.");
    } else {
        printf("\nOrdenando %d clientes por nome...\n", qtde);
        
        int trocou;
        for (passo = 0; passo < qtde - 1; passo++) {
            trocou = 0;
            for (i = 0; i < qtde - 1 - passo; i++) {
                
                fseek(arquivo, i * sizeof(CADASTRO), 0);
                fread(&cad, sizeof(CADASTRO), 1, arquivo);
                
                fseek(arquivo, (i + 1) * sizeof(CADASTRO), 0);
                fread(&cad_aux, sizeof(CADASTRO), 1, arquivo);
                
                if(stricmp(cad.nomeCliente, cad_aux.nomeCliente) > 0)
                {
                    fseek(arquivo, i * sizeof(CADASTRO), 0);
                    fwrite(&cad_aux, sizeof(CADASTRO), 1, arquivo);
                    
                    fseek(arquivo, (i + 1) * sizeof(CADASTRO), 0);
                    fwrite(&cad, sizeof(CADASTRO), 1, arquivo);
                    
                    trocou = 1;
                }
            }
            if (trocou == 0) break; 
        }
        printf("\nArquivo ordenado com sucesso.\n");
    }
    
    fclose(arquivo); 
    system("pause");
}

void gerenciar_clientes_sub_menu() {
    int opc_cli;
    do {
        opc_cli = menu_cliente();
        switch (opc_cli) {
            case 1: cadastrar_cliente(); break;
            case 2: exibir_cliente(); break;
            case 3: alterar_cliente(); break;
            case 4: excluir_cliente(); break;
            case 5: ordenar_cliente(); break;
            case 0: printf("\nVoltando ao menu principal . . . \n"); break;
            default: printf("\nOpcao invalida. "); system("pause");
        }
    } while (opc_cli != 0);
}

// ----------------------------------------------------------------------
// --- 4. M?DULO GERENCIAR LABORAT?RIO --- ok
// ----------------------------------------------------------------------

int menu_laboratorio()
{
	int opc;
	system("cls");
	printf("\n--------------Gerenciar Laboratorio--------------");
	printf("\n1 - Cadastrar novo laboratorio");
	printf("\n2 - Listar todos laboratorios");
	printf("\n3 - Alterar dados do laboratorio");
	printf("\n4 - Deletar laboratorio");
	printf("\n5 - Ordenar laboratorios (por Nome)");
	printf("\n0 - Voltar menu principal");
	printf("\nSelecione a opcao: ");
	scanf("%d", &opc);
	return opc;	
}

int busca_laboratorio(FILE *arquivo, char CNPJ[])
{
	LABORATORIO lab;
	int pos;
	rewind(arquivo);
	// CORRIGIDO: Loop de leitura padr?o.
	while(fread(&lab,sizeof(LABORATORIO),1,arquivo)==1)
	{
		if(stricmp(CNPJ,lab.CNPJ) == 0)
		{
			pos = ftell(arquivo)-sizeof(LABORATORIO);
			return pos;
		}
	}
	return -1;
}

void cadastrar_laboratorio()
{
	FILE *arquivo;
	LABORATORIO lab;
	int pos;
	
	arquivo = fopen("laboratorio.bin", "ab+");
	if(arquivo == NULL) {
		printf("Erro no arquivo");
        system("pause");
        return; // Padr?o: Retorna imediatamente em erro.
    }
	
	do{
		system("cls");
		printf("\nCNPJ: ");fflush(stdin);
		gets(lab.CNPJ);
        
        if (strlen(lab.CNPJ) == 0) break; // Sa?da se o CNPJ for vazio

		pos = busca_laboratorio(arquivo, lab.CNPJ);
		if(pos == -1)
		{
			printf("\nNome do Laboratorio: "); fflush(stdin);
			gets(lab.nomeLaboratorio);
			printf("\nEndereco ");
			printf("\nRua: "); fflush(stdin);
			gets(lab.end.rua);
			printf("\nNumero: ");
			scanf("%d", &lab.end.numero);
			printf("\nBairro: "); fflush(stdin);
			gets(lab.end.bairro);
			printf("\nCidade: "); fflush(stdin);
			gets(lab.end.cidade);
			printf("\nEstado: "); fflush(stdin);
			gets(lab.end.uf);
			printf("\nContato ");
			printf("\nNome do responsavel ");fflush(stdin);
			gets(lab.nomeResponsavel);
			printf("\nDDD: ");
			scanf("%d", &lab.tel.ddd);
			printf("\nTelefone: ");
			scanf("%d", &lab.tel.telefone);
			
			fwrite(&lab, sizeof(LABORATORIO), 1, arquivo);
            printf("\nLaboratorio cadastrado com sucesso!");
		}
		else
		{
			printf("\nO laboratorio com CNPJ %s ja esta cadastrado.", lab.CNPJ);
		}
		printf("\nDeseja Continuar Cadastrando (S/N)? ");
	}while(toupper(getche())=='S');
	
	fclose(arquivo);
	system("cls");
	system("pause");
}

void exibir_laboratorio()
{
	FILE *arquivo;
	LABORATORIO lab;
    int count = 0;
    
	arquivo = fopen("laboratorio.bin", "rb");
	if(arquivo == NULL) {
		printf("\nNenhum laboratorio cadastrado.");
        system("pause");
        return; // Padr?o: Retorna imediatamente em erro.
    }

    system("cls");
    printf("--- Laboratorios Cadastrados ---\n");
    
    while(fread(&lab, sizeof(LABORATORIO), 1, arquivo) == 1)
    {
        count++;
        printf("\nNome: %s", lab.nomeLaboratorio);
        printf("\nCNPJ: %s", lab.CNPJ);
        printf("\nEndereco: Rua: %s, Numero: %d, Bairro: %s, Cidade: %s, Estado: %s", lab.end.rua, lab.end.numero, lab.end.bairro, lab.end.cidade, lab.end.uf);
        printf("\nContato:%s (%d) %d",lab.nomeResponsavel, lab.tel.ddd, lab.tel.telefone);
        printf("\n--------------------------------------");
    }

    if (count == 0) {
        printf("\nNenhum laboratorio encontrado no arquivo.");
    }
    
	fclose(arquivo);
	system("pause");
	system("cls");
}

void alterar_laboratorio()
{
	FILE *arquivo;
	LABORATORIO lab;
	int pos, op;
	
	arquivo = fopen("laboratorio.bin", "rb+");
	if(arquivo == NULL) {
		printf("Erro no arquivo");
        system("pause");
        return; // Padr?o: Retorna imediatamente em erro.
    }
	
	system("cls");
	printf("Digite o CNPJ que sera alterado (deixe vazio para sair): "); fflush(stdin);
	gets(lab.CNPJ);
	
	while(strlen(lab.CNPJ) != 0) // Padr?o: Usado strlen
	{
		pos = busca_laboratorio(arquivo, lab.CNPJ);
		if(pos == -1) {
			printf("Laboratorio nao encontrado\n");
		} else {
			fseek(arquivo, pos, SEEK_SET);
			fread(&lab, sizeof(LABORATORIO), 1, arquivo);
			printf("\n--------------------------");
			printf("\nNome: %s", lab.nomeLaboratorio);
			printf("\nCNPJ: %s", lab.CNPJ);
			printf("\nEndereco: Rua: %s, Numero: %d, Bairro: %s, Cidade: %s, Estado: %s", lab.end.rua, lab.end.numero, lab.end.bairro, lab.end.cidade, lab.end.uf);
			printf("\nContato: %s (%d) %d",lab.nomeResponsavel, lab.tel.ddd, lab.tel.telefone);
            
			printf("\nDeseja alterar \n1 - Endereco \n2 - Contato ? ");
			scanf("%d", &op);
            
			if(op == 1)
			{
				printf("\nNovo endereco ");
				printf("\nRua: "); fflush(stdin);
				gets(lab.end.rua);
				printf("\nNumero: ");
				scanf("%d", &lab.end.numero);
				printf("\nBairro: "); fflush(stdin);
				gets(lab.end.bairro);
				printf("\nCidade: "); fflush(stdin);
				gets(lab.end.cidade);
				printf("\nEstado: "); fflush(stdin);
				gets(lab.end.uf);
                printf("\nEndereco alterado com sucesso!");
			}
			else if(op == 2) // Usado else if
			{
				printf("\nNovo contato");
				printf("\nResponsavel: ");fflush(stdin);
				gets(lab.nomeResponsavel);
				printf("\nDDD: ");
				scanf("%d", &lab.tel.ddd);
				printf("\nTelefone: ");
				scanf("%d", &lab.tel.telefone);
                printf("\nContato alterado com sucesso!");
			}
            else {
                printf("\nAlteracao cancelada.");
            }
            
            fseek(arquivo, pos, SEEK_SET);
            fwrite(&lab, sizeof(LABORATORIO), 1, arquivo); // Escreve o registro completo
		}
        
		system("pause");
        system("cls");
		printf("\nQual o CNPJ para alterar (deixe vazio para sair): "); fflush(stdin);
		gets(lab.CNPJ);
	}
    
	fclose(arquivo);
	system("cls");
	system("pause");
}

void excluir_laboratorio()
{
	FILE *arquivo;
	LABORATORIO lab;
	int pos;
	char cnpj[20];
    int arquivo_fechado_interno = 0;
    
	arquivo = fopen("laboratorio.bin", "rb");
	if(arquivo == NULL) {
		printf("\nNenhum arquivo de laboratorio encontrado.");
        system("pause");
        return; // Padr?o: Retorna imediatamente em erro.
    }
	
	system("cls");
	printf("Informe o CNPJ do laboratorio que deseja excluir (deixe vazio para sair): "); fflush(stdin);
	gets(cnpj);
	
	if(strlen(cnpj) != 0) // Padr?o: Usado strlen
	{
		pos = busca_laboratorio(arquivo, cnpj);
		if(pos == -1) {
			printf("Laboratorio nao encontrado");
		} else {
			fseek(arquivo, pos, SEEK_SET);
			fread(&lab, sizeof(LABORATORIO), 1, arquivo);
			
			printf("\nNome: %s", lab.nomeLaboratorio);
			printf("\nCNPJ: %s", lab.CNPJ);
			printf("\nEndereco: Rua: %s, Numero: %d, Bairro: %s, Cidade: %s, Estado: %s", lab.end.rua, lab.end.numero, lab.end.bairro, lab.end.cidade, lab.end.uf);
			printf("\nContato: (%d) %d", lab.tel.ddd, lab.tel.telefone);
			printf("\nDeseja Excluir S/N? ");
            
			if(toupper(getche()) == 'S')
			{
				FILE *temp;
				temp = fopen("auxiliar.bin", "wb");
                
                if (temp == NULL) {
                    printf("\nErro ao criar arquivo auxiliar. Exclusao cancelada.\n");
                } else {
                    rewind(arquivo);
                    
                    // CR?TICO CORRIGIDO: Loop de leitura padr?o para evitar o "problema do ?ltimo registro"
                    while(fread(&lab, sizeof(LABORATORIO), 1, arquivo) == 1)
                    {
                        if(stricmp(cnpj, lab.CNPJ) != 0)
                            fwrite(&lab, sizeof(LABORATORIO), 1, temp);
                    }
                    
                    fclose(arquivo);
                    fclose(temp);
                    arquivo_fechado_interno = 1;

                    remove("laboratorio.bin");
                    rename("auxiliar.bin", "laboratorio.bin");
                    printf("\n\nExclusao concluida com sucesso.\n");
                }
			} else {
                printf("\n\nExclusao cancelada pelo usuario.\n");
            }
		}
	} else {
        printf("\n\nCNPJ nao informado. Exclusao cancelada.\n");
    }
    
    // Fechamento condicional para o caso de erro no arquivo tempor?rio
    if (arquivo != NULL && arquivo_fechado_interno == 0) {
        fclose(arquivo);
    }
    
    system("pause");
	system("cls");
}

void ordenar_laboratorio()
{
    FILE *arquivo;
    LABORATORIO lab, lab_aux;
    int i,passo , qtde = 0;
    
    arquivo = fopen("laboratorio.bin","rb+");
    if(arquivo == NULL) {
    	printf("\nErro no arquivo ou nenhum laboratorio cadastrado.");
        system("pause");
        return; // Padr?o: Retorna imediatamente em erro.
    }
    
    fseek(arquivo, 0, SEEK_END);
    qtde = ftell(arquivo) / sizeof(LABORATORIO);
    
    if (qtde <= 1) {
        printf("\nNao ha necessidade de ordenacao.");
    } else {
        printf("\nOrdenando %d laboratorios por nome...\n", qtde);
        
        int trocou;
        // CORRIGIDO: L?gica Bubble Sort com for aninhado e flag 'trocou'
        for (passo = 0; passo < qtde - 1; passo++) {
            trocou = 0;
            for (i = 0; i < qtde - 1 - passo; i++) {
                
                fseek(arquivo, i * sizeof(LABORATORIO), SEEK_SET);
                fread(&lab, sizeof(LABORATORIO), 1, arquivo);
                
                fseek(arquivo, (i + 1) * sizeof(LABORATORIO), SEEK_SET);
                fread(&lab_aux, sizeof(LABORATORIO), 1, arquivo);
                
                if(stricmp(lab.nomeLaboratorio, lab_aux.nomeLaboratorio) > 0)
                {
                    // CR?TICO CORRIGIDO: Escreve a estrutura COMPLETA (&lab_aux)
                    fseek(arquivo, i * sizeof(LABORATORIO), SEEK_SET);
                    fwrite(&lab_aux, sizeof(LABORATORIO), 1, arquivo); 
                    
                    // CR?TICO CORRIGIDO: Escreve a estrutura COMPLETA (&lab)
                    fseek(arquivo, (i + 1) * sizeof(LABORATORIO), SEEK_SET);
                    fwrite(&lab, sizeof(LABORATORIO), 1, arquivo);
                    
                    trocou = 1;
                }
            }
            if (trocou == 0) break; 
        }
        printf("\nArquivo ordenado com sucesso.\n");
    }

    fclose(arquivo);
    system("pause");
    system("cls");
}

void gerenciar_laboratorio_sub_menu() {
    int opc_lab;
    do {
        opc_lab = menu_laboratorio();
        switch (opc_lab) {
            case 1: cadastrar_laboratorio(); break;
            case 2: exibir_laboratorio(); break;
            case 3: alterar_laboratorio(); break;
            case 4: excluir_laboratorio(); break;
            case 5: ordenar_laboratorio(); break;
            case 0: printf("\nVoltando ao menu principal . . . \n"); break;
            default: printf("\nOpcao invalida. "); system("pause");
        }
    } while (opc_lab != 0);
}


// ----------------------------------------------------------------------
// --- 5. M?DULO GERENCIAR PRODUTOS (Com Valida??o de Laborat?rio) --- ok
// ----------------------------------------------------------------------

int menu_produto()
{
	int opc;
	system("cls");
	printf("\n--------------Gerenciar Produtos--------------");
	printf("\n1 - Cadastrar novo produto");
	printf("\n2 - Listar todos produtos");
	printf("\n3 - Alterar dados do produto");
	printf("\n4 - Deletar produto");
	printf("\n5 - Ordenar produto (por Nome)");
	printf("\n0 - Voltar menu principal");
	printf("\nSelecione a opcao: ");
	scanf("%d", &opc);
	return opc;	
}

void cadastrar_produto() {
    FILE *arquivo;
    PRODUTO prod;
    int pos;

    arquivo = fopen("produto.bin", "ab+");
    if(arquivo == NULL) {
        printf("Erro no arquivo");
        system("pause");
        return; // Sai da fun??o em caso de erro.
    }

    do {
        system("cls");
        printf("\n--- Cadastrar Produto ---");
        printf("\nCodigo: "); fflush(stdin);
        gets(prod.codigo);

        if (strlen(prod.codigo) == 0) break; 

        pos = busca_produto(arquivo, prod.codigo);
        if (pos == -1) {
            printf("\nNome: "); fflush(stdin);
            gets(prod.nomeProduto);
            printf("\nQuantidade: "); scanf("%d", &prod.quantidade);
            printf("\nPreco Custo: "); scanf("%f", &prod.precoCusto);
            printf("\nPreco Venda: "); scanf("%f", &prod.precoVenda);
            printf("\nLote: "); fflush(stdin);
            gets(prod.lote);
            printf("\nData Validade (Dia Mes Ano): "); 
            scanf("%d %d %d", &prod.dataValidade.dia, &prod.dataValidade.mes, &prod.dataValidade.ano);
            
            // --- VALIDA??O DO CNPJ DO LABORAT?RIO (RESTRICAO APLICADA) ---
            int lab_valido = 0;
            char temp_cnpj[20];

            do {
                printf("\nCNPJ Laboratorio (Obrigatorio): "); fflush(stdin);
                gets(temp_cnpj);

                if (strlen(temp_cnpj) > 0) {
                    if (laboratorio_existe(temp_cnpj)) {
                        strcpy(prod.cnpjLaboratorio, temp_cnpj);
                        lab_valido = 1; 
                        printf("Laboratorio valido encontrado. Prosseguindo...\n");
                    } else {
                        printf("\nCNPJ de Laboratorio nao encontrado. Tente novamente.\n");
                        lab_valido = 0;
                    }
                } else {
                    printf("\nO CNPJ do Laboratorio nao pode ser vazio.\n");
                    lab_valido = 0;
                }
            } while (lab_valido == 0);
            // --------------------------------------------------------------
            
            fwrite(&prod, sizeof(PRODUTO), 1, arquivo);
            printf("\nProduto cadastrado com sucesso! ");
        } else {
            printf("\nProduto com Codigo %s ja cadastrado.", prod.codigo);
        }
        printf("\n\nDeseja Continuar Cadastrando (S/N)? ");
    } while (toupper(getche()) == 'S');
    
    fclose(arquivo);
    system("cls");
    system("pause");
}

void exibir_produto()
{
	FILE *arquivo;
	PRODUTO prod;
	arquivo = fopen("produto.bin", "rb");
	if(arquivo == NULL)
	{
		printf("\nNenhum produto cadastrado.");
	}
	else
	{
		system("cls");
        printf("--- Produtos Cadastrados ---\n");
        while(!feof(arquivo) == 0)
		{
			fread(&prod, sizeof(PRODUTO), 1, arquivo);
			printf("\nCodigo: %s", prod.codigo);
			printf("\nNome: %s", prod.nomeProduto);
			printf("\nEstoque: %d", prod.quantidade);
            printf("\nPreco Venda: R$ %.2f", prod.precoVenda);
            printf("\nValidade: %02d/%02d/%d", prod.dataValidade.dia, prod.dataValidade.mes, prod.dataValidade.ano);
            printf("\nCNPJ Lab: %s", prod.cnpjLaboratorio);
			printf("\n--------------------------------------");
		}
        fclose(arquivo);
	}
	system("pause");
	system("cls");
}
void alterar_produto()
{
	FILE *arquivo;
	PRODUTO prod;
	int pos, op;
	
	arquivo = fopen("produto.bin", "rb+");
	if(arquivo == NULL)
	{
		printf("Erro no arquivo");
		system("pause");
		return;
	}
	
    system("cls");
	printf("Digite o CODIGO do produto que sera alterado (deixe vazio para sair): "); fflush(stdin);
	gets(prod.codigo);
	
	while(strlen(prod.codigo) != 0)
	{
		pos = busca_produto(arquivo, prod.codigo);
		if(pos == -1)
		{
			printf("Produto nao encontrado\n");
		}
		else
		{
			fseek(arquivo, pos, 0);
			fread(&prod, sizeof(PRODUTO), 1, arquivo);
			printf("\n--------------------------");
			printf("\nNome atual: %s", prod.nomeProduto);
			printf("\nEstoque atual: %d", prod.quantidade);
			printf("\nPreco Venda atual: %.2f", prod.precoVenda);

			printf("\nDeseja alterar \n1 - Precos e Estoque \n2 - Validade ? ");
			scanf("%d", &op);
            
			if(op == 1)
			{
				printf("\nNova Quantidade em Estoque: ");
				scanf("%d", &prod.quantidade);
				printf("\nNovo Preco de Custo: ");
				scanf("%f", &prod.precoCusto);
				printf("\nNovo Preco de Venda: ");
				scanf("%f", &prod.precoVenda);
                
				fseek(arquivo, pos, 0);
				fwrite(&prod, sizeof(PRODUTO), 1, arquivo);
				printf("\nDados alterados com sucesso!\n");
			}
			else if(op == 2)
			{
				printf("\nNova Data Validade (Dia Mes Ano): ");
                scanf("%d %d %d", &prod.dataValidade.dia, &prod.dataValidade.mes, &prod.dataValidade.ano);
                
				fseek(arquivo, pos, 0);
				fwrite(&prod, sizeof(PRODUTO), 1, arquivo);
				printf("\nValidade alterada com sucesso!\n");
			} else {
                printf("\nAlteracao cancelada.\n");
            }
		}
		system("pause");
		system("cls");
		printf("\nQual o CODIGO para alterar (deixe vazio para sair): ");fflush(stdin);
		gets(prod.codigo);
	}
	
	fclose(arquivo);
	system("cls");
	system("pause");
}
void excluir_produto()
{
	FILE *arquivo, *temp;
	PRODUTO prod;
	char codigo[20];
	int pos;
    
	arquivo = fopen("produto.bin", "rb");
	if(arquivo == NULL)
	{
		printf("\nNenhum arquivo de produto encontrado para exclusao.");	
		system("pause");
		return;
	}
	
	system("cls");
	printf("Informe o CODIGO do produto que deseja excluir: ");fflush(stdin);
	gets(codigo);
	
	if(strlen(codigo) != 0)
	{
		pos = busca_produto(arquivo, codigo);
		if(pos == -1)
		{
			printf("Produto nao encontrado");
		}
		else
		{
			fseek(arquivo, pos, 0);
			fread(&prod, sizeof(PRODUTO), 1, arquivo);
			
			printf("\nNome: %s", prod.nomeProduto);
			printf("\nCodigo: %s", prod.codigo);
			
			printf("\nDeseja Excluir S/N? ");
			if(toupper(getche()) == 'S')
			{
				temp = fopen("auxiliar_prod.bin", "wb"); 
				if (temp == NULL) {
                    printf("\nErro ao criar arquivo auxiliar. Exclusao cancelada.\n");
                    fclose(arquivo); // Fecha o original e sai
                    system("pause");
                    return;
                }
				
				rewind(arquivo);
				
		        while(!feof(arquivo) == 0)
				{
					fread(&prod, sizeof(PRODUTO), 1, arquivo);
					if(stricmp(codigo, prod.codigo) != 0)
					{
						fwrite(&prod, sizeof(PRODUTO), 1, temp);
					}
				}
				
				fclose(arquivo);
				fclose(temp);
                remove("produto.bin");
                rename("auxiliar_prod.bin", "produto.bin");
                printf("\n\nProcesso de exclusao concluido.\n");
                system("pause");
                system("cls");
                return;
			} else {
                printf("\n\nExclusao cancelada pelo usuario.\n");
            }
		}
	} else {
	    printf("\n\nCodigo nao informado. Exclusao cancelada.\n");
	}
    
    fclose(arquivo);
    system("pause");
    system("cls");
}
void ordenar_produto()
{
    FILE *arquivo;
    PRODUTO prod, prod_aux;
    int i,passo, qtde = 0;
    
    arquivo = fopen("produto.bin", "rb+");
    if (arquivo == NULL)
    {
        printf("Erro no arquivo");
        system("pause");
        return; // Sai da fun??o em caso de erro.
    }
    
    fseek(arquivo, 0, 2);
    qtde = ftell(arquivo) / sizeof(PRODUTO);
    
    // A verifica??o qtde <= 1 n?o ? mais um return, apenas uma mensagem.
    if (qtde <= 1) {
        printf("\nNao ha necessidade de ordenacao.");
    } else {
        printf("\nOrdenando %d produtos por nome...\n", qtde);

        // Algoritmo Bubble Sort
        int trocou;
        for (passo = 0; passo < qtde - 1; passo++) {
            trocou = 0;
            for (i = 0; i < qtde - 1 - passo; i++) {
                
                // L? prod
                fseek(arquivo, i * sizeof(PRODUTO), 0);
                fread(&prod, sizeof(PRODUTO), 1, arquivo); 
                
                // L? prod_aux
                fseek(arquivo, (i + 1) * sizeof(PRODUTO), 0);
                fread(&prod_aux, sizeof(PRODUTO), 1, arquivo);
                
                if(stricmp(prod.nomeProduto, prod_aux.nomeProduto) > 0)
                {
                    // Troca a posi??o das estruturas no arquivo
                    fseek(arquivo, i * sizeof(PRODUTO), 0);
                    fwrite(&prod_aux, sizeof(PRODUTO), 1, arquivo);
                    
                    fseek(arquivo, (i + 1) * sizeof(PRODUTO), 0);
                    fwrite(&prod, sizeof(PRODUTO), 1, arquivo);
                    
                    trocou = 1; 
                }
            }
            if (trocou == 0) break; 
        }
        printf("\nArquivo ordenado com sucesso.\n");
    }
    
    fclose(arquivo);
    system("pause");
    system("cls");
}
void gerenciar_produtos_sub_menu() {
    int opc_prod;
    do {
        opc_prod = menu_produto();
        switch (opc_prod) {
            case 1: cadastrar_produto(); break;
            case 2: exibir_produto(); break;
            case 3: alterar_produto(); break;
            case 4: excluir_produto(); break;
            case 5: ordenar_produto(); break;
            case 0: printf("\nVoltando ao menu principal . . . \n"); break;
            default: printf("\nOpcao invalida. "); system("pause");
        }
    } while (opc_prod != 0);
}


// ----------------------------------------------------------------------
// --- 6. M?DULO GERENCIAR CATEGORIAS ---ok
// ----------------------------------------------------------------------

int menu_categoria()
{
	int opc;
	system("cls");
	printf("\n--------------Gerenciar Categorias--------------");
	printf("\n1 - Cadastrar nova categoria");
	printf("\n2 - Listar todas categorias");
	printf("\n3 - Alterar nome da categoria");
	printf("\n4 - Deletar categoria");
	printf("\n5 - Ordenar categorias (por Nome)");
	printf("\n0 - Voltar menu principal");
	printf("\nSelecione a opcao: ");
	scanf("%d", &opc);
	return opc;	
}

void cadastrar_categoria()
{
	FILE *arquivo;
	CATEGORIA cat;
	int pos;
    char continuar = 'S'; // Flag de controle
	
	arquivo = fopen("categoria.bin", "ab+");
	if(arquivo == NULL)
	{
		printf("Erro no arquivo");
		system("pause");
		return;
	}
	
	do{
		system("cls");
		printf("\n--- Cadastrar Categoria ---");
		printf("\nID da Categoria (Digite 0 para sair): ");
		scanf("%d", &cat.idCategoria);
		
		if (cat.idCategoria != 0) 
		{ 
            // L?gica principal s? executa se o ID for diferente de 0
			pos = busca_categoria(arquivo, cat.idCategoria);
			if(pos == -1)
			{
				printf("\nNome da Categoria: "); fflush(stdin);
				gets(cat.nomeCategoria);
				
				fwrite(&cat, sizeof(CATEGORIA), 1, arquivo);
				printf("\nCategoria cadastrada com sucesso! ");
			}
			else
			{
				printf("\nCategoria com ID %d ja cadastrada.", cat.idCategoria);
			}
			printf("\n\nDeseja Continuar Cadastrando (S/N)? ");
            continuar = toupper(getche());
		}
		else
		{
            continuar = 'N'; // For?a a sa?da do la?o se o ID for 0
		}
	}while(continuar == 'S');
	
	fclose(arquivo);
	system("cls");
	system("pause");
}

void exibir_categoria()
{
	FILE *arquivo;
	CATEGORIA cat;
    int count = 0;
    
	arquivo = fopen("categoria.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nNenhuma categoria cadastrada.");
	}
	else
	{
        // Se o arquivo abriu, executa a l?gica de listagem
		system("cls");
        printf("--- Categorias Cadastradas ---\n");
        while(!feof(arquivo) == 0)
		{
			fread(&cat, sizeof(CATEGORIA), 1, arquivo);
            count++;
			printf("\nID: %d", cat.idCategoria);
			printf("\nNome: %s", cat.nomeCategoria);
			printf("\n--------------------------------------");
		}
        if (count == 0) printf("\nNenhuma categoria encontrada no arquivo.");
        
        fclose(arquivo); // Fechamento seguro
	}
    
	system("pause");
	system("cls");
}

void alterar_categoria()
{
	FILE *arquivo;
	CATEGORIA cat;
	int pos, id_busca;
	
	arquivo = fopen("categoria.bin", "rb+");
	if(arquivo == NULL)
	{
		printf("Erro no arquivo");
		system("pause");
		return;
	}
	
    system("cls");
	printf("Digite o ID da categoria que sera alterada (0 para sair): ");
	scanf("%d", &id_busca);
	
	while(id_busca != 0)
	{
		pos = busca_categoria(arquivo, id_busca);
		if(pos == -1)
		{
			printf("Categoria nao encontrada\n");
		}
		else
		{
			fseek(arquivo, pos, 0);
			fread(&cat, sizeof(CATEGORIA), 1, arquivo);
			printf("\n--------------------------");
			printf("\nNome atual: %s", cat.nomeCategoria);

			printf("\nNovo Nome: "); fflush(stdin);
			gets(cat.nomeCategoria);
				
			fseek(arquivo, pos, 0);
			fwrite(&cat, sizeof(CATEGORIA), 1, arquivo);
			printf("\nNome alterado com sucesso!\n");
		}
		system("pause");
		system("cls");
		printf("\nQual o ID para alterar (0 para sair): ");
		scanf("%d", &id_busca);
	}
	
	fclose(arquivo);
	system("cls");
	system("pause");
}

void excluir_categoria()
{
	FILE *arquivo, *temp;
	CATEGORIA cat;
	int id_excluir;
	int pos;
    
	arquivo = fopen("categoria.bin", "rb");
	if(arquivo == NULL)
	{
		printf("\nNenhum arquivo de categoria encontrado para exclusao.");	
		system("pause");
		return;
	}
	
	system("cls");
	printf("Informe o ID da categoria que deseja excluir (0 para sair): ");
	scanf("%d", &id_excluir);
	
	if(id_excluir != 0)
	{
		pos = busca_categoria(arquivo, id_excluir);
		if(pos == -1)
		{
			printf("Categoria nao encontrada");
		}
		else
		{
			fseek(arquivo, pos, 0);
			fread(&cat, sizeof(CATEGORIA), 1, arquivo);
			
			printf("\nID: %d", cat.idCategoria);
			printf("\nNome: %s", cat.nomeCategoria);
			
			printf("\nDeseja Excluir S/N? ");
			if(toupper(getche()) == 'S')
			{
				temp = fopen("auxiliar_cat.bin", "wb"); 
				if (temp != NULL) 
                {
                    rewind(arquivo);
                    
			        while(!feof(arquivo) == 0)
					{
						fread(&cat, sizeof(CATEGORIA), 1, arquivo);
                        if(id_excluir != cat.idCategoria)
                        {
                            fwrite(&cat, sizeof(CATEGORIA), 1, temp);
                        }
                    }
                    
                    fclose(arquivo);
                    fclose(temp);
                    
                    remove("categoria.bin");
                    rename("auxiliar_cat.bin", "categoria.bin");
                    printf("\n\nProcesso de exclusao concluido.\n");
                } 
                else 
                {
                    printf("\nErro ao criar arquivo auxiliar. Exclusao cancelada.\n");
                    // Apenas fecha o arquivo original, j? que o temp n?o abriu
                    fclose(arquivo);
                }
			} 
            else 
            {
                printf("\n\nExclusao cancelada pelo usuario.\n");
            }
		}
	} 
    else 
    {
	    printf("\n\nExclusao cancelada pelo usuario.\n");
	}
    system("pause");
    system("cls");
}

void ordenar_categoria()
{
    FILE *arquivo;
    CATEGORIA cat, cat_aux;
    int i;
    int qtde = 0; // Se mantiver int, o compilador pode avisar sobre ftell

    arquivo = fopen("categoria.bin", "rb+");
    
    if (arquivo == NULL)
    {
        printf("Erro no arquivo");
    }
    else
    {
        fseek(arquivo, 0, 2);
        qtde = ftell(arquivo) / sizeof(CATEGORIA);
        
        printf("\nOrdenando categorias por nome...\n");
        
        // Algoritmo Bubble Sort
        int trocou = 1;
        while(trocou == 1) {
            trocou = 0;
            for (i = 0; i < qtde - 1; i++) {
                
                fseek(arquivo, i * sizeof(CATEGORIA), 0);
                fread(&cat, sizeof(CATEGORIA), 1, arquivo); 
                
                fseek(arquivo, (i + 1) * sizeof(CATEGORIA), 0);
                fread(&cat_aux, sizeof(CATEGORIA), 1, arquivo);
                
                if(stricmp(cat.nomeCategoria, cat_aux.nomeCategoria) > 0)
                {
                    // Troca a posi??o das estruturas no arquivo
                    fseek(arquivo, i * sizeof(CATEGORIA), 0);
                    fwrite(&cat_aux, sizeof(CATEGORIA), 1, arquivo);
                    
                    fseek(arquivo, (i + 1) * sizeof(CATEGORIA), 0);
                    fwrite(&cat, sizeof(CATEGORIA), 1, arquivo);
                    
                    trocou = 1; 
                }
            }
        }
        
        printf("\nArquivo ordenado com sucesso.\n");
        
        fclose(arquivo); // Fechamento seguro
    }
    
    system("pause");
    system("cls");
}

void gerenciar_categorias_sub_menu() {
    int opc_cat;
    do {
        opc_cat = menu_categoria();
        switch (opc_cat) {
            case 1: cadastrar_categoria(); break;
            case 2: exibir_categoria(); break;
            case 3: alterar_categoria(); break;
            case 4: excluir_categoria(); break;
            case 5: ordenar_categoria(); break;
            case 0: printf("\nVoltando ao menu principal . . . \n"); break;
            default: printf("\nOpcao invalida. "); system("pause");
        }
    } while (opc_cat != 0);
}
// ----------------------------------------------------------------------
// --- 7. M?DULO GERAR PROMO??ES ---
// ----------------------------------------------------------------------

int menu_promocao()
{
	int opc;
	system("cls");
	printf("\n--------------Gerenciar Promocoes--------------");
	printf("\n1 - Cadastrar nova promocao");
	printf("\n2 - Listar todas promocoes");
	printf("\n3 - Alterar dados da promocao");
	printf("\n4 - Deletar promocao");
	printf("\n5 - Cadastrar promocao por produto proximo do vencimento");
	printf("\n6- Cadastrar promocao por lote proximo do vencimento");
	printf("\n0 - Voltar menu principal");
	printf("\nSelecione a opcao: ");
	scanf("%d", &opc);
	return opc;	
}
void cadastrar_promocao()
{
	FILE *arquivo;
	PROMOCAO prm;
	int pos;
    char continuar = 'S'; 
	
	arquivo = fopen("promocao.bin", "ab+");
	
	if(arquivo == NULL)
	{
		printf("Erro no arquivo");
	}
	else
	{
		do{
			system("cls");
			printf("\n--- Cadastrar Promocao ---");
			printf("\nID da Promocao (Digite 0 para sair): ");
			scanf("%d", &prm.idPromocao);
			
			if (prm.idPromocao != 0) 
			{ 
				pos = busca_promocao(arquivo, prm.idPromocao);
				if(pos == -1)
				{
					printf("\nNome da Promocao: "); fflush(stdin);
					gets(prm.nomePromocao);
					
                    printf("\nData de Inicio (Dia Mes Ano): ");
                    scanf("%d %d %d", &prm.dataInicio.dia, &prm.dataInicio.mes, &prm.dataInicio.ano);

                    printf("\nData de Fim (Dia Mes Ano): ");
                    scanf("%d %d %d", &prm.dataFim.dia, &prm.dataFim.mes, &prm.dataFim.ano);
                    
					fwrite(&prm, sizeof(PROMOCAO), 1, arquivo);
					printf("\nPromocao cadastrada com sucesso! ");
				}
				else
				{
					printf("\nPromocao com ID %d ja cadastrada.", prm.idPromocao);
				}
				printf("\n\nDeseja Continuar Cadastrando (S/N)? ");
                continuar = toupper(getche());
			}
			else
			{
                continuar = 'N'; 
			}
		}while(continuar == 'S');
        
        fclose(arquivo);
	}
	
	system("cls");
	system("pause"); 
}
void exibir_promocao()
{
	FILE *arquivo;
	PROMOCAO prm;
    int count = 0;
    
	arquivo = fopen("promocao.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nNenhuma promocao cadastrada.");
	}
	else
	{
		system("cls");
        printf("--- Promocoes Cadastradas ---\n");
        while(!feof(arquivo) == 0)
		{
			fread(&prm, sizeof(PROMOCAO), 1, arquivo);
            count++;
			printf("\nID: %d", prm.idPromocao);
			printf("\nNome: %s", prm.nomePromocao);
            printf("\nValidade: %02d/%02d/%d a %02d/%02d/%d", prm.dataInicio.dia, prm.dataInicio.mes, prm.dataInicio.ano, prm.dataFim.dia, prm.dataFim.mes, prm.dataFim.ano);
			printf("\n--------------------------------------");
		}
        if (count == 0) printf("\nNenhuma promocao encontrada no arquivo.");
        
        fclose(arquivo);
	}
    
	system("pause");
	system("cls");
}
void alterar_promocao()
{
	FILE *arquivo;
	PROMOCAO prm;
	int pos, id_busca;
	
	arquivo = fopen("promocao.bin", "rb+");
	
	if(arquivo == NULL)
	{
		printf("Erro no arquivo");
	}
	else
	{
        system("cls");
        printf("Digite o ID da promocao que sera alterada (0 para sair): ");
        scanf("%d", &id_busca);
        
        while(id_busca != 0)
        {
            pos = busca_promocao(arquivo, id_busca);
            if(pos == -1)
            {
                printf("Promocao nao encontrada\n");
            }
            else
            {
                fseek(arquivo, pos, 0);
                fread(&prm, sizeof(PROMOCAO), 1, arquivo);
                printf("\n--------------------------");
                printf("\nNome atual: %s", prm.nomePromocao);

                printf("\nNovo Nome: "); fflush(stdin);
                gets(prm.nomePromocao);
                
                printf("\nNova Data de Inicio (Dia Mes Ano): ");
                scanf("%d %d %d", &prm.dataInicio.dia, &prm.dataInicio.mes, &prm.dataInicio.ano);

                printf("\nNova Data de Fim (Dia Mes Ano): ");
                scanf("%d %d %d", &prm.dataFim.dia, &prm.dataFim.mes, &prm.dataFim.ano);
                    
                fseek(arquivo, pos, 0);
                fwrite(&prm, sizeof(PROMOCAO), 1, arquivo);
                printf("\nPromocao alterada com sucesso!\n");
            }
            system("pause");
            system("cls");
            printf("\nQual o ID para alterar (0 para sair): ");
            scanf("%d", &id_busca);
        }
        
        fclose(arquivo);
	}
	
	system("cls");
	system("pause");
}
void excluir_promocao()
{
	FILE *arquivo;
    FILE *temp = NULL; 
	PROMOCAO prm;
	int id_excluir;
	int pos;
    
	arquivo = fopen("promocao.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nNenhum arquivo de promocao encontrado para exclusao.");	
	}
	else
	{
        int arquivo_fechado_interno = 0; 
        
        system("cls");
        printf("Informe o ID da promocao que deseja excluir (0 para sair): ");
        scanf("%d", &id_excluir);
        
        if(id_excluir != 0)
        {
            pos = busca_promocao(arquivo, id_excluir);
            if(pos == -1)
            {
                printf("Promocao nao encontrada");
            }
            else
            {
                fseek(arquivo, pos, 0);
                fread(&prm, sizeof(PROMOCAO), 1, arquivo);
                
                printf("\nID: %d", prm.idPromocao);
                printf("\nNome: %s", prm.nomePromocao);
                
                printf("\nDeseja Excluir S/N? ");
                if(toupper(getche()) == 'S')
                {
                    temp = fopen("auxiliar_prm.bin", "wb"); 
                    if (temp != NULL) 
                    {
                        rewind(arquivo);
                        
				        while(!feof(arquivo) == 0)
						{
							fread(&prm, sizeof(PROMOCAO), 1, arquivo);
                            if(id_excluir != prm.idPromocao)
                            {
                                fwrite(&prm, sizeof(PROMOCAO), 1, temp);
                            }
                        }
                        
                        fclose(arquivo);
                        fclose(temp);
                        arquivo_fechado_interno = 1; 
                        
                        remove("promocao.bin");
                        rename("auxiliar_prm.bin", "promocao.bin");
                        printf("\n\nProcesso de exclusao concluido.\n");
                    } 
                    else 
                    {
                        printf("\nErro ao criar arquivo auxiliar. Exclusao cancelada.\n");
                    }
                } 
                else 
                {
                    printf("\n\nExclusao cancelada pelo usuario.\n");
                }
            }
        } 
        else 
        {
            printf("\n\nExclusao cancelada pelo usuario.\n");
        }
        
        if (arquivo_fechado_interno == 0) 
        {
            fclose(arquivo);
        }
	}
    
    system("pause");
    system("cls");
}

	void gerar_promocao_por_lote()
{
    FILE *arquivo;
    PRODUTO prod;
    int dias_limite;
    float percentual_desconto;
    NASCIMENTO data_hoje;
    char lote_promocao[30];
    
    system("cls");
    printf("--- Gerar Promocao por Lote Proximo ao Vencimento ---\n");
    
    printf("\nInforme a DATA DE HOJE (Dia Mes Ano): ");
    scanf("%d %d %d", &data_hoje.dia, &data_hoje.mes, &data_hoje.ano);
    
    printf("Informe o NUMERO DE DIAS LIMITE: ");
    scanf("%d", &dias_limite);
    
    printf("Informe o PERCENTUAL DE DESCONTO (Ex: 0.20 para 20%%): ");
    scanf("%f", &percentual_desconto);
    
    printf("Informe o LOTE para promocao: "); fflush(stdin);
    gets(lote_promocao);
    
    NASCIMENTO data_limite = somar_dias_data(data_hoje, dias_limite);
    long limite_int = data_to_long(data_limite);
    
    arquivo = fopen("produto.bin", "rb+");
    if(arquivo == NULL) {
        printf("\nErro ao abrir arquivo de produtos.");
        system("pause");
        return;
    }
    
    int count = 0;
    long pos_atual;
    
	if(arquivo == NULL)
	{
		printf("\nErro no arquivo ou nenhum produto cadastrado.");
	}
	else
	{
        printf("\n-----------------------------------------------------");
        printf("\nData de Hoje: %02d/%02d/%d", data_hoje.dia, data_hoje.mes, data_hoje.ano);
        printf("\nLimite de Vencimento: %02d/%02d/%d (Proximos %d dias)", data_limite.dia, data_limite.mes, data_limite.ano, dias_limite);
        printf("\nDesconto Aplicado: %.2f%%\n", percentual_desconto * 100);
        printf("-----------------------------------------------------\n");
		
        long pos_atual;
        
        // Loop para ler o arquivo e aplicar a promocao
        while(!feof(arquivo) == 0)
		{
			fread(&prod, sizeof(PRODUTO), 1, arquivo);
            long validade_int = data_to_long(prod.dataValidade);
            
            if(validade_int <= limite_int) {
                pos_atual = ftell(arquivo) - sizeof(PRODUTO);
                float preco_original = prod.precoVenda;
                prod.precoVenda = prod.precoVenda * (1.0 - percentual_desconto);
                
                fseek(arquivo, pos_atual, SEEK_SET);
                fwrite(&prod, sizeof(PRODUTO), 1, arquivo);
                
                count++;
                printf("\nPROMOÇÃO APLICADA (%d):", count);
                printf("\nCodigo: %s - %s", prod.codigo, prod.nomeProduto);
                printf("\nValidade: %02d/%02d/%d", prod.dataValidade.dia, prod.dataValidade.mes, prod.dataValidade.ano);
                printf("\nPreco: R$ %.2f -> R$ %.2f", preco_original, prod.precoVenda);
                printf("\n--------------------------------------");
                
    			break;
            }
        }
    }
    
    printf("\n\nTotal de produtos do lote em promocao: %d\n", count);
    fclose(arquivo);
    system("pause");
}

void gerar_promocoes_sub_menu() {
    int opc_prm;
    do {
        opc_prm = menu_promocao();
        switch (opc_prm) {
            case 1: cadastrar_promocao(); 
				break;
            case 2: exibir_promocao(); 
				break;
            case 3: alterar_promocao(); 
				break;
            case 4: excluir_promocao(); 
				break;
            case 5: gerar_promocoes_vencimento(); 
				break;
				  case 6: gerar_promocao_por_lote();
				  break;
            case 0: printf("\nVoltando ao menu principal . . . \n"); break;
            default: printf("\nOpcao invalida. "); system("pause");
        }
    } while (opc_prm != 0);
}

// ----------------------------------------------------------------------
// --- 8. M?DULO EMITIR RELAT?RIOS ---
// ----------------------------------------------------------------------

int menu_relatorios()
{
	int opc;
	system("cls");
	printf("\n--------------- Emitir Relatorios --------------");
	printf("\n1 - Relatorio de Produtos em Estoque Baixo");
	printf("\n2 - Relatorio de Vendas por Cliente (CPF)");
	printf("\n3 - Relatorio de Vendas por Periodo");
	printf("\n4 - Relatorio de Produtos Proximos ao Vencimento");
	printf("\n0 - Voltar menu principal");
	printf("\nSelecione a opcao: ");
	scanf("%d", &opc);
	return opc;	
}

void relatorio_estoque_baixo()
{
	FILE *arquivo;
	PRODUTO prod;
    int limite = 10; // Define o limite de estoque
    int count = 0;
    
	arquivo = fopen("produto.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nNenhum produto cadastrado para analise.");
	}
	else
	{
		system("cls");
        printf("--- Relatorio: Produtos com Estoque < %d ---\n", limite);
        while(!feof(arquivo) == 0)
		{
			fread(&prod, sizeof(PRODUTO), 1, arquivo);
            if (prod.quantidade < limite)
            {
                count++;
                printf("\nCodigo: %s", prod.codigo);
                printf("\nNome: %s", prod.nomeProduto);
                printf("\nEstoque Atual: %d", prod.quantidade);
                printf("\nPreco Venda: R$ %.2f", prod.precoVenda);
                printf("\n--------------------------------------");
            }
		}
        if (count == 0) printf("\nNenhum produto com estoque abaixo de %d encontrado.", limite);
        
        fclose(arquivo);
	}
    
	system("pause");
	system("cls");
}
void relatorio_vendas_por_cliente()
{
	FILE *arquivo;
	VENDA venda;
	char cpf_busca[20];
    int count = 0;
	
	arquivo = fopen("venda.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nErro no arquivo (Nenhum registro de vendas encontrado).");
	}
	else
	{
		system("cls");
        printf("--- Relatorio: Vendas por Cliente ---\n");
        printf("Digite o CPF do cliente: "); fflush(stdin);
        gets(cpf_busca);
        
        while(!feof(arquivo) == 0)
		{
			fread(&venda, sizeof(PRODUTO), 1, arquivo);
            if (stricmp(cpf_busca, venda.cpfCliente) == 0)
            {
                count++;
                printf("\nID Venda: %d", venda.idVenda);
                printf("\nProduto: %s", venda.codigoProduto); 
                printf("\nQuantidade: %d", venda.quantidadeVendida);
                printf("\nValor Total: R$ %.2f", venda.precoTotal);
                printf("\nData: %02d/%02d/%d", venda.dataVenda.dia, venda.dataVenda.mes, venda.dataVenda.ano);
                printf("\n--------------------------------------");
            }
		}
        
        if (count == 0) 
            printf("\nNenhuma venda encontrada para o CPF %s.", cpf_busca);
        
        fclose(arquivo);
	}
    
	system("pause");
	system("cls");
}
void relatorio_vendas_por_periodo()
{
    FILE *arquivo;
	VENDA venda;
    NASCIMENTO data_inicio, data_fim;
    int count = 0;
    float total_vendas = 0.0;
    
	arquivo = fopen("venda.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nErro no arquivo (Nenhum registro de vendas encontrado).");
	}
	else
	{
		system("cls");
        printf("--- Relatorio: Vendas por Periodo ---\n");
        printf("Data de Inicio (Dia Mes Ano): ");
        scanf("%d %d %d", &data_inicio.dia, &data_inicio.mes, &data_inicio.ano);
        printf("Data de Fim (Dia Mes Ano): ");
        scanf("%d %d %d", &data_fim.dia, &data_fim.mes, &data_fim.ano);
        
        // Convers?o das datas de in?cio e fim para um n?mero inteiro compar?vel (AAAA MM DD)
        long inicio_int = (long)data_inicio.ano * 10000 + (long)data_inicio.mes * 100 + data_inicio.dia;
        long fim_int = (long)data_fim.ano * 10000 + (long)data_fim.mes * 100 + data_fim.dia;
        
        printf("\nResultados:\n");
        
        while(!feof(arquivo) == 0)
		{
            // Converte a data da venda para um n?mero inteiro compar?vel
			fread(&venda, sizeof(VENDA), 1, arquivo);
            long venda_int = (long)venda.dataVenda.ano * 10000 + (long)venda.dataVenda.mes * 100 + venda.dataVenda.dia;

            // Verifica se a data da venda est? dentro do per?odo (>= inicio E <= fim)
            if (venda_int >= inicio_int && venda_int <= fim_int)
            {
                count++;
                total_vendas += venda.precoTotal;
                
                printf("\nID Venda: %d", venda.idVenda);
                printf("\nCliente CPF: %s", venda.cpfCliente);
                printf("\nValor Total: R$ %.2f", venda.precoTotal);
                printf("\nData: %02d/%02d/%d", venda.dataVenda.dia, venda.dataVenda.mes, venda.dataVenda.ano);
                printf("\n--------------------------------------");
            }
		}

        printf("\n\nTotal de Vendas no Periodo: %d", count);
        printf("\nValor Total Arrecadado: R$ %.2f", total_vendas);
        
        fclose(arquivo);
	}
    
	system("pause");
	system("cls");
}

void relatorio_produtos_vencimento()
{
	FILE *arquivo;
	PRODUTO prod;
    int count = 0;
    int dias_limite;
    NASCIMENTO data_hoje;
    
    system("cls");
    printf("--- Relatorio: Produtos Proximos ao Vencimento ---\n");
    
    printf("\nInforme a DATA DE HOJE (Dia Mes Ano): ");
    scanf("%d %d %d", &data_hoje.dia, &data_hoje.mes, &data_hoje.ano);
    
    printf("Informe o NUMERO DE DIAS LIMITE (Ex: 90 dias): ");
    scanf("%d", &dias_limite);
    
    NASCIMENTO data_limite = somar_dias_data(data_hoje, dias_limite); 

    long limite_int = data_to_long(data_limite);
    
	arquivo = fopen("produto.bin", "rb");
    
	if(arquivo == NULL)
	{
		printf("\nErro no arquivo (Nenhum produto cadastrado para analise).");
	}
	else
	{
        printf("\n-----------------------------------------------------");
        printf("\nData de Hoje: %02d/%02d/%d", data_hoje.dia, data_hoje.mes, data_hoje.ano);
        printf("\nLimite de Vencimento (Proximos %d dias): %02d/%02d/%d", dias_limite, data_limite.dia, data_limite.mes, data_limite.ano);
        printf("\n-----------------------------------------------------\n");
		
        while(!feof(arquivo) == 0)
		{
			fread(&prod, sizeof(PRODUTO), 1, arquivo);
            long validade_int = data_to_long(prod.dataValidade);
            
            // O produto est? perto do vencimento ou j? venceu (validade <= limite futuro)
            if (validade_int <= limite_int)
            {
                count++;
                printf("\nCodigo: %s", prod.codigo);
                printf("\nNome: %s", prod.nomeProduto);
                printf("\nEstoque: %d", prod.quantidade);
                printf("\nValidade: %02d/%02d/%d", prod.dataValidade.dia, prod.dataValidade.mes, prod.dataValidade.ano);
                printf("\n--------------------------------------");
            }
            
		}
        
        if (count == 0) 
            printf("\nNenhum produto encontrado com vencimento proximo (%d dias).\n", dias_limite);
        
        fclose(arquivo); 
        
	}
    
	system("pause");
	system("cls");
}

void emitir_relatorios_sub_menu() {
    int opc_rel;
    do {
        opc_rel = menu_relatorios();
        switch (opc_rel) {
            case 1: relatorio_estoque_baixo();
				break;
            case 2: relatorio_vendas_por_cliente(); 
				break;
            case 3: relatorio_vendas_por_periodo(); 
				break;
            case 4: relatorio_produtos_vencimento(); 
				break;
            case 0: printf("\nVoltando ao menu principal . . . \n"); 
				break;
            default: printf("\nOpcao invalida. "); system("pause");
        }
    } while (opc_rel != 0);
}

void efetuar_venda()
{
    FILE *arq_venda, *arq_prod, *arq_cli;
    VENDA venda;
    PRODUTO prod;
    CADASTRO cliente;
    char cpf[20], codigo_prod[20];
    int pos_prod, pos_cli;
    
    system("cls");
    printf("--- Efetuar Venda ---\n");
    
    // Verifica cliente
    arq_cli = fopen("cliente.bin", "rb");
    if(arq_cli == NULL) {
        printf("\nErro: Nenhum cliente cadastrado.");
        system("pause");
        return;
    }
    
    printf("\nCPF do Cliente: "); fflush(stdin);
    gets(cpf);
    
    pos_cli = busca_cliente(arq_cli, cpf);
    if(pos_cli == -1) {
        printf("\nCliente nao encontrado!");
        fclose(arq_cli);
        system("pause");
        return;
    }
    fclose(arq_cli);
    
    // Verifica produto
    arq_prod = fopen("produto.bin", "rb+");
    if(arq_prod == NULL) {
        printf("\nErro: Nenhum produto cadastrado.");
        system("pause");
        return;
    }
    
    printf("\nCodigo do Produto: "); fflush(stdin);
    gets(codigo_prod);
    
    pos_prod = busca_produto(arq_prod, codigo_prod);
    if(pos_prod == -1) {
        printf("\nProduto nao encontrado!");
        fclose(arq_prod);
        system("pause");
        return;
    }
    
    fseek(arq_prod, pos_prod, SEEK_SET);
    fread(&prod, sizeof(PRODUTO), 1, arq_prod);
    
    printf("\nProduto: %s", prod.nomeProduto);
    printf("\nEstoque disponivel: %d", prod.quantidade);
    printf("\nPreco unitario: R$ %.2f", prod.precoVenda);
    
    printf("\n\nQuantidade a vender: ");
    scanf("%d", &venda.quantidadeVendida);
    
    if(venda.quantidadeVendida > prod.quantidade) {
        printf("\nEstoque insuficiente!");
        fclose(arq_prod);
        system("pause");
        return;
    }
    
    // Atualiza estoque
    prod.quantidade -= venda.quantidadeVendida;
    fseek(arq_prod, pos_prod, SEEK_SET);
    fwrite(&prod, sizeof(PRODUTO), 1, arq_prod);
    fclose(arq_prod);
    
    // Registra venda
    arq_venda = fopen("venda.bin", "ab+");
    if(arq_venda == NULL) {
        printf("\nErro ao abrir arquivo de vendas.");
        system("pause");
        return;
    }
    
    // Gera ID da venda
    fseek(arq_venda, 0, SEEK_END);
    venda.idVenda = (ftell(arq_venda) / sizeof(VENDA)) + 1;
    
    strcpy(venda.cpfCliente, cpf);
    strcpy(venda.codigoProduto, codigo_prod);
    venda.precoTotal = prod.precoVenda * venda.quantidadeVendida;
    
    printf("\nData da Venda (Dia Mes Ano): ");
    scanf("%d %d %d", &venda.dataVenda.dia, &venda.dataVenda.mes, &venda.dataVenda.ano);
    
    fwrite(&venda, sizeof(VENDA), 1, arq_venda);
    fclose(arq_venda);
    
    printf("\n\n=== VENDA REALIZADA COM SUCESSO ===");
    printf("\nID Venda: %d", venda.idVenda);
    printf("\nValor Total: R$ %.2f", venda.precoTotal);
    printf("\nNovo Estoque: %d\n", prod.quantidade);
    
    system("pause");
}

// ----------------------------------------------------------------------
// --- 9. MÓDULO EFETUAR COMPRA ---
// ----------------------------------------------------------------------

void efetuar_compra()
{
    FILE *arq_prod;
    PRODUTO prod;
    char codigo[20];
    int pos, qtd_compra;
    
    system("cls");
    printf("--- Efetuar Compra (Entrada de Estoque) ---\n");
    
    arq_prod = fopen("produto.bin", "rb+");
    if(arq_prod == NULL) {
        printf("\nErro ao abrir arquivo de produtos.");
        system("pause");
        return;
    }
    
    printf("\nCodigo do Produto: "); fflush(stdin);
    gets(codigo);
    
    pos = busca_produto(arq_prod, codigo);
    if(pos == -1) {
        printf("\nProduto nao encontrado!");
        fclose(arq_prod);
        system("pause");
        return;
    }
    
    fseek(arq_prod, pos, SEEK_SET);
    fread(&prod, sizeof(PRODUTO), 1, arq_prod);
    
    printf("\nProduto: %s", prod.nomeProduto);
    printf("\nEstoque atual: %d", prod.quantidade);
    
    printf("\n\nQuantidade a comprar: ");
    scanf("%d", &qtd_compra);
    
    if(qtd_compra <= 0) {
        printf("\nQuantidade invalida!");
        fclose(arq_prod);
        system("pause");
        return;
    }
    
    printf("\nNovo Preco de Custo (atual: %.2f): ", prod.precoCusto);
    scanf("%f", &prod.precoCusto);
    
    printf("Novo Preco de Venda (atual: %.2f): ", prod.precoVenda);
    scanf("%f", &prod.precoVenda);
    
    // Atualiza estoque
    prod.quantidade += qtd_compra;
    
    fseek(arq_prod, pos, SEEK_SET);
    fwrite(&prod, sizeof(PRODUTO), 1, arq_prod);
    fclose(arq_prod);
    
    printf("\n\n=== COMPRA REGISTRADA COM SUCESSO ===");
    printf("\nQuantidade adicionada: %d", qtd_compra);
    printf("\nNovo Estoque: %d\n", prod.quantidade);
    
    system("pause");
}



int gerar_promocoes_vencimento()
{
    FILE *arquivo;
    PRODUTO prod;
    int dias_limite;
    float percentual_desconto;
    NASCIMENTO data_hoje;
    
    system("cls");
    printf("--- Gerar Promocao por Vencimento Proximo ---\n");
    
    printf("\nInforme a DATA DE HOJE (Dia Mes Ano): ");
    scanf("%d %d %d", &data_hoje.dia, &data_hoje.mes, &data_hoje.ano);
    
    printf("Informe o NUMERO DE DIAS LIMITE: ");
    scanf("%d", &dias_limite);
    
    printf("Informe o PERCENTUAL DE DESCONTO (Ex: 0.20 para 20%%): ");
    scanf("%f", &percentual_desconto);
    
    NASCIMENTO data_limite = somar_dias_data(data_hoje, dias_limite);
    long limite_int = data_to_long(data_limite);
    
    arquivo = fopen("produto.bin", "rb+");
    if(arquivo == NULL) {
        printf("\nErro ao abrir arquivo de produtos.");
        system("pause");
        return;
    }
    
    int count = 0;
    long pos_atual;
    
    printf("\n--- Produtos em Promocao ---\n");
    
    while(fread(&prod, sizeof(PRODUTO), 1, arquivo) == 1) {
        long validade_int = data_to_long(prod.dataValidade);
        
        if(validade_int <= limite_int) {
            pos_atual = ftell(arquivo) - sizeof(PRODUTO);
            float preco_original = prod.precoVenda;
            prod.precoVenda = prod.precoVenda * (1.0 - percentual_desconto);
            
            fseek(arquivo, pos_atual, SEEK_SET);
            fwrite(&prod, sizeof(PRODUTO), 1, arquivo);
            
            count++;
            printf("\nProduto: %s", prod.nomeProduto);
            printf("\nPreco Original: R$ %.2f", preco_original);
            printf("\nNovo Preco: R$ %.2f", prod.precoVenda);
            printf("\nDesconto: %.0f%%", percentual_desconto * 100);
            printf("\n--------------------------------------");
        }
    }
    
    printf("\n\nTotal de produtos em promocao: %d\n", count);
    fclose(arquivo);
    system("pause");
}




// ----------------------------------------------------------------------
// --- 10. MÓDULO EFETUAR DESCARTE ---
// ----------------------------------------------------------------------

void efetuar_descarte()
{
    FILE *arq_prod;
    PRODUTO prod;
    char codigo[20];
    int pos, qtd_descarte;
    char motivo[100];
    
    system("cls");
    printf("--- Efetuar Descarte de Produto ---\n");
    
    arq_prod = fopen("produto.bin", "rb+");
    if(arq_prod == NULL) {
        printf("\nErro ao abrir arquivo de produtos.");
        system("pause");
        return;
    }
    
    printf("\nCodigo do Produto: "); fflush(stdin);
    gets(codigo);
    
    pos = busca_produto(arq_prod, codigo);
    if(pos == -1) {
        printf("\nProduto nao encontrado!");
        fclose(arq_prod);
        system("pause");
        return;
    }
    
    fseek(arq_prod, pos, SEEK_SET);
    fread(&prod, sizeof(PRODUTO), 1, arq_prod);
    
    printf("\nProduto: %s", prod.nomeProduto);
    printf("\nEstoque atual: %d", prod.quantidade);
    printf("\nValidade: %02d/%02d/%d", prod.dataValidade.dia, prod.dataValidade.mes, prod.dataValidade.ano);
    
    printf("\n\nQuantidade a descartar: ");
    scanf("%d", &qtd_descarte);
    
    if(qtd_descarte <= 0 || qtd_descarte > prod.quantidade) {
        printf("\nQuantidade invalida!");
        fclose(arq_prod);
        system("pause");
        return;
    }
    
    printf("Motivo do descarte: "); fflush(stdin);
    gets(motivo);
    
    printf("\n\nConfirma o descarte de %d unidades? (S/N): ", qtd_descarte);
    if(toupper(getche()) == 'S') {
        // Atualiza estoque
        prod.quantidade -= qtd_descarte;
        
        fseek(arq_prod, pos, SEEK_SET);
        fwrite(&prod, sizeof(PRODUTO), 1, arq_prod);
        
        printf("\n\n=== DESCARTE REGISTRADO COM SUCESSO ===");
        printf("\nQuantidade descartada: %d", qtd_descarte);
        printf("\nMotivo: %s", motivo);
        printf("\nNovo Estoque: %d\n", prod.quantidade);
    } else {
        printf("\n\nDescarte cancelado.");
    }
    
    fclose(arq_prod);
    system("pause");
}

// ----------------------------------------------------------------------
// --- 11. MENU PRINCIPAL (Main) ---
// ----------------------------------------------------------------------

int menu_principal() {
    int opc;
    system("cls");
    printf("\n============= DROGAMAIS - MENU PRINCIPAL =============\n");
    printf("\n1 - Gerenciar Clientes");
    printf("\n2 - Gerenciar Produtos");
    printf("\n3 - Gerenciar Laboratorio");
    printf("\n4 - Gerenciar Categorias");
    printf("\n5 - Gerar Promocoes");
    printf("\n6 - Emitir Relatorios");
    printf("\n7 - Efetuar Venda"); 
    printf("\n8 - Efetuar Compra"); 
    printf("\n9 - Realizar descarte");
    printf("\n0 - Sair do Sistema");
    printf("\nSelecione a opcao: ");
    scanf("%d", &opc);
    return opc;
}

int main()
{
	int opc_principal;
	
	do{
		opc_principal = menu_principal();
		switch(opc_principal)
		{
			case 1: gerenciar_clientes_sub_menu();
				break;
			case 2: gerenciar_produtos_sub_menu(); 
				break; 
			case 3: gerenciar_laboratorio_sub_menu(); 
				break;
			case 4: gerenciar_categorias_sub_menu(); 
				break; 
			case 5: gerar_promocoes_sub_menu(); 
				break; 
			case 6: emitir_relatorios_sub_menu(); 
				break;
				case 7: efetuar_venda();  
				break;
				case 8: efetuar_compra();
				break;
				case 9: efetuar_descarte();
				break;
				
			case 0: printf("\nEncerrando o sistema DrogaMais. Ate mais! \n"); 
				break;
			default: printf("\nOpcao invalida. "); system("pause");
		}
	}while(opc_principal != 0);
    return 0;
}
