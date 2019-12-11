#include<stdio.h> //teste novo 234
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

struct sockaddr_in addr_servidor;

char chave[16];
char buffer_mensagem_out[312];
char buffer_mensagem_in[312];

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

				unsigned long long int ChavePri = 2;//rand() % 10;
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
	printf("		    |___Enviando...\n");
	char texto_temp[tM];
	memset(texto_temp, 0x0, tM);

	strcpy(texto_temp, msg);
						
	memset(buffer_mensagem_out, 0x0, tM);
	strcpy(buffer_mensagem_out, "-e ");
	strcat(buffer_mensagem_out, texto_temp);
	strcat(buffer_mensagem_out, ">");

	send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);

}


int main()
{

	printf("*Criando Socket Cliente\n");	

	int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);	
	
	printf("|___Socket Cliente Criado com Arquivo de Descricao: %d\n", socket_cliente);

	char enderecoServer[20];
	int portaServer;
	char TipoConexao;

	//Dados Servidor defaul------------------
	strcpy(enderecoServer, "10.0.2.15");
	portaServer = 8008;
	TipoConexao = 'n';
	//---------------------------------------

	/*printf("|___Dados do Servidor:\n");
	printf("	|___IP: ");
	scanf("%s", enderecoServer);
	printf("\n	|___Porta: ");
	scanf("%d", &portaServer);*/
	printf("\n	|___Tipo de Conexao: ");
	scanf("%c", &TipoConexao);

	int t;

	if (TipoConexao == 'n')
	{

			addr_servidor.sin_family 	= AF_INET;
			addr_servidor.sin_port		= htons(portaServer);
			addr_servidor.sin_addr.s_addr	= inet_addr(enderecoServer);

			printf("|___Conectando ao Servidor...\n");

			if (connect(socket_cliente, (struct sockaddr*) &addr_servidor, sizeof(addr_servidor)) == -1) 
			{
					printf("\033[0;31m");
        	printf("	|___Falha ao Tentar se Conectar ao Servidor: %s\n", enderecoServer);
					printf("\033[0m");

					return 1;
	  	}
			else if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0) 
			{
					printf("\033[0;32m");
	        printf("|___Servidor Conectado!\n");
					printf("\033[0m");		
					printf("	|___Servidor Disse: %s\n", buffer_mensagem_in);
	  	}

		char comando[123];
		char texto_claro[tM];
		int flag = 1;

		while(flag)
		{
			memset(buffer_mensagem_in, 0x0, tM);
  	  memset(buffer_mensagem_out, 0x0, tM);
			memset(texto_claro, 0x0, tM);		

			printf("	|___Enviar Comando: ");
			scanf(" %[^\n]s", buffer_mensagem_out);
			if (buffer_mensagem_out[0] == '-')
			{
				char opcao = buffer_mensagem_out[1];
				switch (opcao)
				{

					case '?':

						printf("-c (Enviar Comando)\n-s (Fechar Conexao)\n-e (Encriptar)\n");

					break;

					case 'e':

						sscanf(buffer_mensagem_out,"-e %[^\n]s", texto_claro);

						AES_Envia(AES_Encripta(DH_GeraChave(socket_cliente), texto_claro), socket_cliente);	

						if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0)
							printf("	|___Retorno Servidor: %s\n", buffer_mensagem_in);	
					
					break;

					case 'd':

						sscanf(buffer_mensagem_out,"-d%[^\n]s", buffer_mensagem_out);
						
						send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);

					break;

					case 'c':
						
						printf("		|___Enviando Comando Terminal\n");
						send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);
					
					break;

					case 't':

						sscanf(buffer_mensagem_out,"-t %[^\n]s", buffer_mensagem_out);
						//TOR_EnviaMsg(TOR_MontaCirc(TOR_EnderecoRelays()), buffer_mensagem_out);
						//TOR_MontaCirc();

					break;

					case 's':
						
						printf("		|___Fechando Conexao\n");
						send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);
						flag = 0;
	
					break;

				}
			}
			else
			{
				send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0);
	
				if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0)
					printf("	|___Retorno Servidor: %s\n", buffer_mensagem_in);
			}

		}

		close(socket_cliente);
		return 0;

	}
	else if (TipoConexao == 't')
	{

		printf("|___Montando Circuito TOR...\n");
		char Relays[3][16] = {"10.0.2.15:8080\0","10.0.2.15:7777\0","10.0.2.15:8118\0"};

		printf("		|___Insira os Enderecos de Relays:\n");
		printf("		    |___Guard:	%s\n", Relays[0]);
		//scanf("%s", Relays[0]);
		printf("		    |___Middle:	%s\n", Relays[1]);
		//scanf("%s", Relays[1]);
		printf("		    |___Exit:	%s\n", Relays[2]);
		//scanf("%s", Relays[2]);

		struct sockaddr_in addr_Relay_Guard;

		int porta = 8080;
		char end[16];
		sscanf(Relays[0], "%[^:]s", end);

		addr_Relay_Guard.sin_family 			= AF_INET;
		addr_Relay_Guard.sin_port					= htons(porta);
		addr_Relay_Guard.sin_addr.s_addr	= inet_addr(end);
		
		memset(buffer_mensagem_in, 0x0, tM);
		memset(buffer_mensagem_out, 0x0, tM);


		if (connect(socket_cliente, (struct sockaddr*) &addr_Relay_Guard, sizeof(addr_Relay_Guard)) == -1) 
		{
					printf("\033[0;31m");
        	printf("	|___Falha ao Tentar se Conectar ao Relay Guard: %s\n", Relays[0]);
					printf("\033[0m");

					return 1;
	  }
		else if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0) 
		{
					printf("\033[0;32m");
	        printf("|___Guard Conectado!\n");
					printf("\033[0m");		
					printf("	|___Guard Disse: %s\n", buffer_mensagem_in);
	  }

		//--------------Pega Chaves-----------------------

		char Chaves_Relays[3][16];

		memset(Chaves_Relays[0], 0x0, 16);
		memset(Chaves_Relays[1], 0x0, 16);
		memset(Chaves_Relays[2], 0x0, 16);

		strcpy(Chaves_Relays[0],DH_GeraChave(socket_cliente));

		char texto_claro[tM];
		memset(texto_claro, 0x0, tM);
		strcpy(texto_claro, "-i");// i -> pega senha
		strcat(texto_claro, Relays[1]);

		AES_Envia(AES_Encripta(Chaves_Relays[0], texto_claro), socket_cliente);

		if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0)
		{
					printf("	|___Retorno: %s\n", buffer_mensagem_in);
					strcpy(Chaves_Relays[1], AES_Decripta(Chaves_Relays[0], buffer_mensagem_in));
		}


		memset(texto_claro, 0x0, tM);
		strcpy(texto_claro, "-p ");
		send(socket_cliente, texto_claro, strlen(texto_claro), 0);


		memset(texto_claro, 0x0, tM);
		strcpy(texto_claro, "-i");// i -> pega senha
		strcat(texto_claro, Relays[2]);
		AES_Envia(AES_Encripta(Chaves_Relays[1], texto_claro), socket_cliente);
		char temp[tM];
		
		if ((t = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0)) > 0)
		{				
					memset(temp, 0x0, tM);
					printf("	|___Retorno: %s\n", buffer_mensagem_in);
					sscanf(buffer_mensagem_in,"-e %[^>]s", temp);
					strcpy(Chaves_Relays[2], AES_Decripta(Chaves_Relays[0], temp));
		}

		printf("	|___*CHAVES RELAYS: \n");
		printf("	    |___Guard:	%s\n", Chaves_Relays[0]);
		printf("	    |___Middle:	%s\n", Chaves_Relays[1]);
		printf("	    |___Exit:	%s\n", Chaves_Relays[2]);

		int flag2 = 1;
		while (flag2)
		{
			char com[16];
			char c0[16];
			char c1[16];
			char c2[16];

			memset(c0, 0x0, 16);
			memset(c1, 0x0, 16);
			memset(c2, 0x0, 16);
		
			printf("	|___Enviar Comando via Tor: ");
			memset(com, 0x0, 16);
			scanf(" %[^\n]s", com);
			printf("\n");

			strcpy(c2, AES_Encripta(Chaves_Relays[2], com));
			strcpy(c1, AES_Encripta(Chaves_Relays[1], c2));
			strcpy(c0, AES_Encripta(Chaves_Relays[0], c1));

			printf("	|____INFORMACAO: %s\n", com);
			printf("	|____Camada 1: %s\n", c2);
			printf("	|____Camada 2: %s\n", c1);
			printf("	|____Camada 3: %s\n", c0);

			send(socket_cliente, c0, strlen(c0), 0);
			//close(socket_cliente);
		}		
	}
	

	return 0;
}















