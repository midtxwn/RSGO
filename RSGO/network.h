// NidhoggClientReal.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.


#include <Windows.h>

#include <thread>
#include <vector>
#include "definitions.h"

typedef void* SOCKET;
namespace network
{

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
    S-Cambiar postura (agacharse)
    S-Cambiar arma
    S-Muerte
    S-Disparar
    */
    

      int networkEventProcessor(SOCKET& sockCon, std::vector<entity*>& entList);

      int sendGameStatusAuto(SOCKET& sockCon, std::vector<entity*>& entList);
   
      std::pair<std::thread*, std::thread*> setupMultiplayerConnection(char* IP_textp, uint16_t port, std::vector<entity*>& entList, int esServidor);

      void sendGameEvent(DWORD mT, std::vector<entity*>& entList);

}
