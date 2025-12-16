#include <iostream>

#include <stdlib.h>
#include <thread>
#include <glm/glm.hpp>
//#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <vector>
#include "definitions.h"

extern void shotReceived(enum entity_manager::ENDGAME_STATES state);

namespace network{
    
	constexpr auto BUFFER_SIZE = 1000;
	constexpr auto NUM_CONEXIONES = 1;
    constexpr auto DATA_BYTES = sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(float);
	constexpr auto TICKRATE = 1000.0 / 64.0;
    SOCKET socketCl;
    std::thread senderThread;
    std::thread receiverThread;

	struct messageInfo
	{
        DWORD messageType; //TODO: Separar por bits (flags) para saber qué información de evento nos mandan simultáneamente, siguiendo un orden codificado
        union {
            struct fields {
                glm::mat4 matrix;
                glm::vec3 vector;
                float angle;
            }dataFields;
			BYTE DATA[DATA_BYTES];
        }DATA;
	};

	constexpr auto BYTES_MESSAGETYPE = sizeof(DWORD);
    constexpr auto BYTES_MENSAJE = BYTES_MESSAGETYPE + DATA_BYTES;
    

	enum messageTypes
	{
		PLAYER_MOVED = 0x1,
		PLAYER_ROTATED = 0x2,
		PLAYER_SELECTED_WEAPON_AK = 0x4,
		PLAYER_SELECTED_WEAPON_KNIFE = 0x8,
		PLAYER_CROUCHED_SWITCHED = 0x10,
		PLAYER_SHOT = 0x20,
        ENEMY_HIT = 0x40
	};


    /* EVENTOS A ENVIAR:
    -Mover jugador (translate, rotate)  -> Posición X,Z
    -Rotar jugador (cambio de apuntado) -> 2 Euler angles (izquierda,derecha) + (arriba,abajo)
    -Cambiar postura (agacharse)
    -Cambiar arma
    -Muerte
    -Disparar
    */

	void sendGameEvent(DWORD mT, std::vector<entity*>& entList){

        messageInfo m = { mT };

        switch (mT){
        case messageTypes::PLAYER_SELECTED_WEAPON_AK:
        case messageTypes::PLAYER_SELECTED_WEAPON_KNIFE:
        case messageTypes::PLAYER_CROUCHED_SWITCHED:
        case messageTypes::PLAYER_SHOT:
        case messageTypes::PLAYER_SHOT | messageTypes::ENEMY_HIT:

			send(socketCl, (char*)&(m), BYTES_MESSAGETYPE, 0);
            break;
        }
        
		

    }
    
    void inline processMessage(messageInfo& info, std::vector<entity*>& entList)
    {
        //Mensajes periódicos:
        if (info.messageType & messageTypes::PLAYER_MOVED)
        {
            entity* foreign = entList.at(entity_manager::FOREIGN_PLAYER);
            memcpy(&(foreign->transformToInherit), &(info.DATA.dataFields.matrix), sizeof(glm::mat4));
            memcpy(&(foreign->position), &(info.DATA.dataFields.vector), sizeof(glm::vec3));
            memcpy(&(foreign->eulerAngles.beta), &(info.DATA.dataFields.angle), sizeof(float));
            return;
        }

        //Mensajes por eventos:
        if (info.messageType & messageTypes::PLAYER_SELECTED_WEAPON_AK)
        {
            entList.at(entity_manager::FOREIGN_PLAYER)->weapon = entity_manager::WEAPON_AK;
            return;
        }
		if (info.messageType & messageTypes::PLAYER_SELECTED_WEAPON_KNIFE)
		{
            entList.at(entity_manager::FOREIGN_PLAYER)->weapon = entity_manager::WEAPON_KNIFE;
			return;
		}
		if (info.messageType & messageTypes::PLAYER_CROUCHED_SWITCHED)
		{
            bool* crouch = &(entList.at(entity_manager::FOREIGN_PLAYER)->crouching);
            glm::mat4* previousTransform = &(entList.at(entity_manager::FOREIGN_PLAYER)->transformToInherit);

            if (*crouch)
               *previousTransform  = glm::translate(*previousTransform, glm::vec3(0, OFFSET_PLAYER_HEIGHT_DIFF_WHEN_CROUCHING, 0));
            else
                *previousTransform = glm::translate(*previousTransform, glm::vec3(0, -OFFSET_PLAYER_HEIGHT_DIFF_WHEN_CROUCHING, 0));

            *crouch = !(*crouch);
			return;
		}
		if (info.messageType & messageTypes::PLAYER_SHOT)
		{
            if (info.messageType & messageTypes::ENEMY_HIT){
                shotReceived(entity_manager::LOST);
                printf("RECEIVED HIT!\n");
            }
			return;
		}

    }


    int networkEventProcessor(SOCKET* sockCon, std::vector<entity*>& entList)
    {

        messageInfo info = {};
        while (recv(*sockCon, (char*)&info, BYTES_MENSAJE, 0) > 0)
        {
            processMessage(info, entList);
        }

        std::terminate();
    }

    int sendGameStatusAuto(SOCKET* sockCon, std::vector<entity*>& entList)
    {
        entity* localPlayer = entList.at(entity_manager::LOCAL_PLAYER);

        messageInfo m = { 
            messageTypes::PLAYER_MOVED | messageTypes::PLAYER_ROTATED, 
            localPlayer->transformToInherit, 
            localPlayer->position,
            localPlayer->eulerAngles.beta
        };


		while (send(*sockCon, (char*)&(m), BYTES_MENSAJE, 0) > 0)
        {
            Sleep(TICKRATE);
            m.DATA.dataFields = { localPlayer->transformToInherit, localPlayer->position, localPlayer->eulerAngles.beta };
        }

        std::terminate();
    }

    int server(uint16_t puertoPropio, std::vector<entity*>& entList)
    {

        SOCKET socketS;
        int rec;
        socklen_t tamano = sizeof(struct sockaddr_in);
        uint16_t puertoCliente;
        char IP[INET_ADDRSTRLEN];

        struct sockaddr_in saddrS, saddrC;


        WSADATA wsaData;
        WORD nVersionRequired = MAKEWORD(2, 2);
        WSAStartup(nVersionRequired, &wsaData);

        printf("Creando servidor...\n");
        //Crear el SOCKET
        if ((socketS = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("No se puedo crear el socket\n");
            printf("%d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }


        //Asignar valores al STRUCT del SOCKET
        saddrS.sin_family = AF_INET;
        saddrS.sin_addr.s_addr = htonl(INADDR_ANY);                 //OJO: Si se nos olvida darles la vuelta puede dar problemas (está mal)
        //sscanf(argv[1], "%hu", &puerto);
        saddrS.sin_port = htons(puertoPropio);                      //OJO:


        //Hacemos BIND()
        if (bind(socketS, (struct sockaddr*)&saddrS, tamano) == -1) {
            closesocket(socketS);
            perror("No se pudo asignar dirección\n");
            printf("%d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }


        //Hacemos LISTEN()
        if (listen(socketS, NUM_CONEXIONES) == -1) {
            closesocket(socketS);
            perror("No se pudo marcar como pasivo el socket\n");
            printf("%d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }



        //Hacemos el ACCEPT() de un cliente

        if ((socketCl = accept(socketS, (struct sockaddr*)&saddrC, &tamano)) == -1) {
            closesocket(socketCl);
            closesocket(socketS);
            perror("No se pudo aceptar la conexión\n");
            printf("%d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        printf("Conexion aceptada!\n");
        //saddrC.sin_addr.s_addr = ntohl(saddrC.sin_addr.s_addr);    Nota: No hace falta porque el inet_ntop asume que esta en orden de red


        //Obetenemos el PUERTO E IP del cliente para imprimir

        puertoCliente = ntohs(saddrC.sin_port);
        if ((inet_ntop(AF_INET, (const void*)&saddrC.sin_addr, IP, INET_ADDRSTRLEN) == NULL)) {
            closesocket(socketCl);
            closesocket(socketS);
            perror("ERROR de conversión a string de IP\n");
            printf("%d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        printf("IP conectada: %s\nPuerto conectado: %hu\n", IP, puertoCliente);



        receiverThread = std::thread(networkEventProcessor, &socketCl, std::ref(entList));
        senderThread = std::thread(sendGameStatusAuto, &socketCl, std::ref(entList));
        //closesocket(socketC);

        //closesocket(socketS);
    }

    void client(char* IP_textp, uint16_t port, std::vector<entity*>& entList)
    {

        WSAData wsaData;
        WORD wVersionRequired = MAKEWORD(2, 2);
        WSAStartup(wVersionRequired, &wsaData);
        socketCl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in addr_serv;

        addr_serv.sin_port = htons(port);
        if (inet_pton(AF_INET, IP_textp, &(addr_serv.sin_addr)) < 1)
        {
            perror("Error codificando IP a addr_serv");
            std::cout << WSAGetLastError();
            exit(EXIT_FAILURE);
        }
        addr_serv.sin_family = AF_INET;

        printf("Intentando conectar...\n");
        if ((connect(socketCl, (struct sockaddr*)&addr_serv, sizeof(addr_serv))) == SOCKET_ERROR)
        {
            perror("Error creando conexión con el servidor");
            std::cout << WSAGetLastError();
            exit(EXIT_FAILURE);
        }
        printf("Exito!\n");
         receiverThread = std::thread(networkEventProcessor, &socketCl, std::ref(entList));
         senderThread = std::thread(sendGameStatusAuto, &socketCl, std::ref(entList));




        //closesocket(socketCl);
    }

	std::pair<std::thread*, std::thread*> setupMultiplayerConnection(char* IP_textp, uint16_t port, std::vector<entity*>& entList, int esServidor)
	{
		if (esServidor)
			server(port, entList);
		else client(IP_textp, port, entList);

		std::pair<std::thread*, std::thread*> threads = std::make_pair(&senderThread, &receiverThread);

		return threads;
	}
}
