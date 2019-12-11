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

struct sockaddr_in addr_servidor, addr_cliente;

unsigned long long int ChaveCli, ChavePub, ChavePri;
long long int _Chave_;
char chave[16];

int main()
{

	AES_KEY chave_aes;
				
	int tM = 321;
	char buffer_mensagem_in[tM];
	char buffer_mensagem_out[tM];
	char texto_claro[tM];

	printf("*Criando Socket Servidor\n");
	
	int socket_cliente;	
	int socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
	//*Dominio: 	AF_INET 	- Protocolos IPV4;
	//*Tipo: 	SOCK_STREAM 	- TCP(Protocolo de Controle de Transmissão);
	//*Protocolo: 	0 		- IP(Protocolo Internet).

	printf("|___Socket Servidor Criado com Arquivo de Descricao: %d\n", socket_servidor);
	
	addr_servidor.sin_family 	= AF_INET;
	addr_servidor.sin_port		= htons(8008);
	addr_servidor.sin_addr.s_addr	= htonl(INADDR_ANY);
	
	//Vinclua arquivo de descrição ao socket do servidor
	bind(socket_servidor, (struct sockaddr *) &addr_servidor, sizeof(addr_servidor));
	printf("|___Vinculando Arquivo de Descrião ao Socket\n");

	//Começa a ouvir conexões, neste caso no máximo 5 ao mesmo tempo 
	listen(socket_servidor, 5);
	printf("\n*Ouvindo na Porta: %d\n", ntohs(addr_servidor.sin_port));
	printf("|___Maximo de Conexões: 5\n\n");

	//Cria Socket do Cliente 
	int t_addr_cliente = sizeof(addr_cliente);
	socket_cliente = accept(socket_servidor, (struct sockaddr *)&addr_cliente, &t_addr_cliente);

	//Envia Mensagem ao Cliente
	strcpy(buffer_mensagem_out, "Olá Cliente!\0");
	if (send(socket_cliente, buffer_mensagem_out, strlen(buffer_mensagem_out), 0))
	{
		printf("*Conectado ao Cliente\n");
		
		int flag = 1;

		while(flag)
		{
			memset(buffer_mensagem_in, 0x0, tM);
      memset(buffer_mensagem_out, 0x0, tM);
			memset(texto_claro, 0x0, tM);

			printf("|___Aguardando Comando...\n");

			//Pega Mensagem do Cliente e Coloca na Buffer
			int t_mensagem = recv(socket_cliente, buffer_mensagem_in, sizeof(buffer_mensagem_in), 0);
			buffer_mensagem_in[t_mensagem] = '\0';
			printf("|___Cliente: %s\n", buffer_mensagem_in);

			if (buffer_mensagem_in[0] == '-')
			{
				char opcao = buffer_mensagem_in[1];
				char conteudo[tM];

				switch (opcao)
				{
					case 'c':
						sscanf(buffer_mensagem_in,"-c %[^\n]s", conteudo);
						printf("   |___Executando Comando: %s\n", conteudo);
						
						strcpy(buffer_mensagem_out, "Executando Comando...\0");
						system(conteudo);
					break;
					
					case 'e':

						sscanf(buffer_mensagem_in,"-e %[^..]s", conteudo);
						printf("   |___Mensagem Encriptada: %s\n", conteudo);
						//printf("CHAVE_TEXTO: %s\n", chave);
						AES_set_decrypt_key(chave, 128, &chave_aes);
						AES_decrypt(conteudo, texto_claro, &chave_aes);
						printf("	|___Texto Claro: %s\n", texto_claro);

						memset(buffer_mensagem_out, 0x0, tM);
					break;

					case 'd':
	
						strcpy(buffer_mensagem_out, "Mensagem Descriptografada...\0");
						send(socket_cliente,buffer_mensagem_out,strlen(buffer_mensagem_out),0);
						printf("   |___Gerando Chave (Protocolo DH)\n");

						recv(socket_cliente, &ChaveCli, sizeof(ChaveCli), 0);

						printf("       |___Chave Temp Publica Cliente: %lld\n", ChaveCli);
						
						ChavePri = 3;//(rand() % 10) - (rand() % 5);						
						ChavePub = (unsigned long long int)pow(DH_G, ChavePri) % DH_P;

						printf("       |___Chave Temp Privada: %lld\n", ChavePri);
						printf("       |___Chave Temp Publica: %lld\n", ChavePub);
						printf("       |___Enviando Chave ao Cliente\n");

						send(socket_cliente, &ChavePub, sizeof(ChavePub), 0);	
						
						_Chave_ = (long long int)pow(ChaveCli,ChavePri) % DH_P;
						printf("       |___*CHAVE: %lld\n", _Chave_);
						memcpy(chave, &_Chave_, sizeof(_Chave_));

					break;

					case 's':
						printf("   |___Fechando Conexão\n");
						strcpy(buffer_mensagem_out, "Conexao Encerada\0");
						flag = 0;
					break;
				}
			}
			else
				strcpy(buffer_mensagem_out, "Menssagem Recebida\0");

			send(socket_cliente,buffer_mensagem_out,strlen(buffer_mensagem_out),0);
		}

		printf("\n*Conexão Encerada\n");
		close(socket_cliente);
		close(socket_servidor);	
	}	

	return 0;
}
















