#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>	//Biblioteca para Sockets
#include<sys/types.h>	//
#include<errno.h>	//Biblioteca para descricao de erros
#include<netinet/in.h>	//Biblioteca para structs de sockets
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<openssl/aes.h> //Biblioteca para Criptografia AES
#include<math.h>

#define DH_P 1232131
#define DH_G 9
#define tM 312

struct sockaddr_in addr_servidor, addr_cliente;
int porta_ini;
char chave[16];
char chave2[16];
char buffer_mensagem_out[312];
char buffer_mensagem_in[312];
long long int _Chave_;
unsigned long long int ChaveCli, ChavePub, ChavePri;
AES_KEY chave_aes;
char texto_claro[tM];

int secreto = 0;
int exit_con = 1;

int socket_clienteRelay;	
struct sockaddr_in addr_Relay;

int flag = 1;

char* DH_GeraChave(int socket_cliente)
{
			unsigned long long int _Chave_;
			int t;

			memset(buffer_mensagem_in, 0x0, tM);
			memset(buffer_mensagem_out, 0x0, tM);
			strcpy(buffer_mensagem_out, "-d");
			send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);

			if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0)
			{
				memset(buffer_mensagem_in, 0x0, tM);

				unsigned long long int ChavePri = secreto;//rand() % 10;
				unsigned long long int ChavePub = (unsigned long long int)pow(DH_G,ChavePri) % DH_P;

				printf("		|___Gerando Chave (Protocolo DH)\n");
				printf("		    |___Chave Temp Privada: %lld\n", ChavePri);
				printf("		    |___Chave Temp Publica: %lld\n", ChavePub);
				printf("		    |___Enviando Chave ao Servidor\n");

				send(socket_cliente, &ChavePub, sizeof(ChavePub), 0);
	
				unsigned long long int ChaveSev;

				if ((t = recv(socket_cliente, &ChaveSev, sizeof(ChaveSev), 0)) > 0)
				{
						printf("		    |___Chave Temp Publica Servidor: %lld\n", ChaveSev);

						_Chave_ = (long long int)pow(ChaveSev,ChavePri) % DH_P;
						printf("		    |___*CHAVE: %lld\n", _Chave_);
				}
				memset(buffer_mensagem_out, 0x0, tM);

				static char _Chave_Texto_[16];
				memcpy(_Chave_Texto_, &_Chave_, sizeof(_Chave_));
				
				return _Chave_Texto_;

			} 
}

char * AES_Encripta(char* _Chave_, char *texto_claro)
{
			memset(buffer_mensagem_in, 0x0, tM);
			int t;

			//memcpy(chave, &_Chave_, sizeof(_Chave_));

			AES_KEY chave_aes;
			AES_set_encrypt_key(_Chave_, 128, &chave_aes);

			static char texto_cifrado[tM];
			AES_encrypt(texto_claro, texto_cifrado, &chave_aes);
			printf("		|___Criptografando...\n");

			printf("		    |___Texto Crifrado: ");
			printf("\033[0;34m");			
			printf("%s\n", texto_cifrado);
			printf("\033[0m");	

			memset(texto_claro, 0x0, tM);
			AES_set_decrypt_key(_Chave_, 128, &chave_aes);
			AES_decrypt(texto_cifrado, texto_claro, &chave_aes);
			printf("		    |___Texto Claro: ");
			printf("\033[0;33m");		
			printf("%s\n", texto_claro);
			printf("\033[0m");
		
			return texto_cifrado;
}

char * AES_Decripta(char* _Chave_, char *texto_cifrado)
{
			int t;

			//memcpy(chave, &_Chave_, sizeof(_Chave_));

			AES_KEY chave_aes;

			static char texto_claro[tM];
			memset(texto_claro, 0x0, tM);

			AES_set_decrypt_key(_Chave_, 128, &chave_aes);
			AES_decrypt(texto_cifrado, texto_claro, &chave_aes);

			printf("		|___Decriptografando...\n");
			printf("		    |___Texto Claro: ");
			printf("\033[0;33m");		
			printf("%s\n", texto_claro);
			printf("\033[0m");
		
			memset(buffer_mensagem_in, 0x0, tM);

			return texto_claro;
}

void AES_Envia(char * msg, int socket_cliente)
{

	char texto_temp[tM];
	memset(texto_temp, 0x0, tM);

	strcpy(texto_temp, msg);
						
	memset(buffer_mensagem_out, 0x0, tM);
	strcpy(buffer_mensagem_out, "-e ");
	strcat(buffer_mensagem_out, texto_temp);
	strcat(buffer_mensagem_out, ">");

	send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);

}

int ConectaProxRelay(int porta, char * endereco)
{
		int t;
		addr_Relay.sin_family 			= AF_INET;
		addr_Relay.sin_port					= htons(porta);
		addr_Relay.sin_addr.s_addr	= inet_addr(endereco);
		
		memset(buffer_mensagem_in, 0x0, tM);
		memset(buffer_mensagem_out, 0x0, tM);

		if (connect(socket_clienteRelay, (struct sockaddr*) &addr_Relay, sizeof(addr_Relay)) == -1) 
		{
					printf("\033[0;31m");
        	printf("	|___Falha ao Tentar se Conectar: %s\n", endereco);
					printf("\033[0m");

					return 1;
	  }
		else if ((t = recv(socket_clienteRelay, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0) 
		{
					printf("\033[0;32m");
	        printf("|___Conectado!\n");
					printf("\033[0m");		
					printf("	|___Disse: %s\n", buffer_mensagem_in);

					return 0;
	  }


}

int CharArrey2Int(char * temp)
{

			int res = 0;

			for (int i = 0; i < strlen(temp); i++)
			{
				res += (temp[i] - '0')*(pow(10,i));
			}		

			return res;

}

char chave_prox_convert[16];

void verifica(int socket_cliente, char comando[312])
{
	int t;
	char conteudo[tM], temp[tM];
	//printf("Entrei::%c::%c::\n", comando[0], comando[1]);
	if (comando[0] == '-')
			{
				char opcao = comando[1];
				int porta = 0;
				char texto_cifrado[tM];

				switch (opcao)
				{
					case 'i':
						
						printf("*Tenho que pegar senha\n");
						sscanf(comando,"-i%[^:]s", conteudo);
						printf("ENDERECO: %s\n", conteudo);
						strcpy(temp, "-i ");
						strcat(temp, conteudo);
						strcat(temp, ":%s");
						sscanf(comando, temp, temp);

						ConectaProxRelay(CharArrey2Int(temp), conteudo);
						memset(chave_prox_convert, 0x0, 16);
						strcpy(chave_prox_convert, DH_GeraChave(socket_clienteRelay));

						memset(texto_cifrado, 0x0, tM);
						printf("   |___Chave Clara: %s\n", chave_prox_convert);

						AES_set_encrypt_key(chave, 128, &chave_aes);
						printf("VOU USAR ENCRIP: %s\n", chave);
						AES_encrypt(chave_prox_convert, texto_cifrado, &chave_aes);
						printf("	|___Cifrado: %s\n", texto_cifrado);
						strcpy(buffer_mensagem_out, texto_cifrado);
						strcat(buffer_mensagem_out, "\0");
						send(socket_cliente,buffer_mensagem_out,strlen(buffer_mensagem_out),0);
						memset(buffer_mensagem_in, 0x0, tM);
						t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0);
						//scanf("%d", &t);
						buffer_mensagem_in[t] = '\0';
						verifica(socket_cliente, buffer_mensagem_in);
						//flag = 0;

					break;
					
					case 'e':

						memset(texto_claro, 0x0, tM);
						sscanf(comando,"-e %[^>]s", conteudo);
						printf("   |___Mensagem Encriptada: %s\n", conteudo);

						AES_set_decrypt_key(chave, 128, &chave_aes);
						AES_decrypt(conteudo, texto_claro, &chave_aes);
						printf("	|___Texto Claro: %s\n", texto_claro);

						memset(buffer_mensagem_out, 0x0, tM);
						verifica(socket_cliente, texto_claro);

					break;

					case 'd':
	
						strcpy(buffer_mensagem_out, "Mensagem Descriptografada...\0");
						send(socket_cliente,buffer_mensagem_out,strlen(buffer_mensagem_out),0);
						printf("   |___Gerando Chave (Protocolo DH)\n");

						recv(socket_cliente, &ChaveCli, sizeof(ChaveCli), 0);

						printf("       |___Chave Temp Publica Cliente: %lld\n", ChaveCli);
						
						ChavePri = secreto + 1;//(rand() % 10) - (rand() % 5);						
						ChavePub = (unsigned long long int)pow(DH_G, ChavePri) % DH_P;

						printf("       |___Chave Temp Privada: %lld\n", ChavePri);
						printf("       |___Chave Temp Publica: %lld\n", ChavePub);
						printf("       |___Enviando Chave ao Cliente\n");

						send(socket_cliente, &ChavePub, sizeof(ChavePub), 0);	
						
						_Chave_ = (unsigned long long int)pow(ChaveCli,ChavePri) % DH_P;
						printf("       |___*CHAVE: %lld\n", _Chave_);
						memcpy(chave, &_Chave_, sizeof(_Chave_));
						printf("       |___CHAVE TEXTO: %s\n", chave);
						//scanf("%d", &t);
					break;

					case 'p':

							memset(buffer_mensagem_in, 0x0, tM);
							recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0);
							
							send(socket_clienteRelay, &buffer_mensagem_in, sizeof(buffer_mensagem_in), 0);					
							
							memset(buffer_mensagem_in, 0x0, tM);
							recv(socket_clienteRelay, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0);
							printf("RECEBI: %s\n", buffer_mensagem_in);
							printf("VOU USAR: %s\n", chave_prox_convert);
							AES_Envia(AES_Encripta(chave, AES_Decripta(chave_prox_convert,buffer_mensagem_in)), socket_cliente);
						
							memset(buffer_mensagem_in, 0x0, tM);
							t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0);
							buffer_mensagem_in[t] = '\0';

							printf(":::::Cliente falou: %s\n", buffer_mensagem_in);
						
							memset(buffer_mensagem_out, 0x0, tM);
							strcpy(buffer_mensagem_out, AES_Decripta(chave,buffer_mensagem_in));							
							send(socket_clienteRelay,buffer_mensagem_out,strlen(buffer_mensagem_out),0);

					break;
				}
			}
			else
			{
					memset(buffer_mensagem_out, 0x0, tM);
					strcpy(buffer_mensagem_out, AES_Decripta(chave,comando));							
					if (buffer_mensagem_out[0] == '-' && buffer_mensagem_out[1] == ' ')
					{
							sscanf(buffer_mensagem_out,"- %[^;]s", conteudo);
							printf("ENDERECO: 10.0.2.15\n");
							if (exit_con)
							{
										exit_con = 0;
										ConectaProxRelay(8008, "10.0.2.15");
							}
							memset(buffer_mensagem_out, 0x0, tM);
							strcpy(buffer_mensagem_out, conteudo);	
					}					
						send(socket_clienteRelay,buffer_mensagem_out,strlen(buffer_mensagem_out),0);
			}
}

int main()
{

	printf("Selecione uma Porta: ");
	scanf("%d", &porta_ini);

	//printf("\nSegredo: ");
	//scanf("%d", &secreto);
	int max = 7;
	int min = 2;

	secreto = rand() % (max + 1 - min) + min;

	socket_clienteRelay = socket(AF_INET, SOCK_STREAM, 0);
//===============ServidorRELAY=======================
	printf("\n*Criando Socket Servidor RELAY\n");
	
	int socket_cliente;			

	int socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
	//*Dominio: 	AF_INET 	- Protocolos IPV4;
	//*Tipo: 	SOCK_STREAM 	- TCP(Protocolo de Controle de Transmissão);
	//*Protocolo: 	0 		- IP(Protocolo Internet).

	printf("|___Socket Servidor RELAY Criado com Arquivo de Descricao: %d\n", socket_servidor);
	
	addr_servidor.sin_family 	= AF_INET;
	addr_servidor.sin_port		= htons(porta_ini); //8080
	addr_servidor.sin_addr.s_addr	= htonl(INADDR_ANY);
	
	//Vinclua arquivo de descrição ao socket do servidor
	bind(socket_servidor, (struct sockaddr *) &addr_servidor, sizeof(addr_servidor));
	printf("|___Vinculando Arquivo de Descrião ao Socket\n");

	//Começa a ouvir conexões, neste caso no máximo 1 ao mesmo tempo 
	listen(socket_servidor, 1);
	printf("\n*Ouvindo na Porta: %d\n", ntohs(addr_servidor.sin_port));
	printf("|___Maximo de Conexões: 1\n\n");

	//Cria Socket do Cliente RELAY 
	int t_addr_cliente = sizeof(addr_cliente);
	socket_cliente = accept(socket_servidor, (struct sockaddr *)&addr_cliente, &t_addr_cliente);

	//Envia Mensagem ao Cliente
	strcpy(buffer_mensagem_out, "Olá Cliente! Sou Relay\0");
	if (send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0))
	{
		printf("*Conectado ao Cliente\n");

//========================================================
//===============ClienteRELAY=============================
		//printf("*Criando Socket Cliente RELAY\n");	

		//int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);	
	
		//printf("|___Socket Cliente RELAY Criado com Arquivo de Descricao: %d\n", socket_cliente);

		char enderecoServer[20];
		int portaServer;
		char TipoConexao;
		int t;

//========================================================

		while(flag)
		{
			memset(buffer_mensagem_in, 0x0, tM);
      memset(buffer_mensagem_out, 0x0, tM);

			printf("|___Aguardando Comando...\n");

			//Pega Mensagem do Cliente e Coloca na Buffer
			int t_mensagem;
			if ((t_mensagem = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0)
			{
				buffer_mensagem_in[t_mensagem] = '\0';
				printf("|___Cliente: %s\n", buffer_mensagem_in);
			}

			verifica(socket_cliente, buffer_mensagem_in);
		}
	}

		close(socket_cliente);
		return 0;

}















