#include "Entities.h"

#include "MenuSZK.h"
#include "Debug.h"

std::unordered_set<int> knownPeds;
std::unordered_set<int> knownVehicles;

void Entities::Update()
{
    FindNewEntities();
}

std::unordered_set<int> Entities::GetVehiclesRefs()
{
    return knownVehicles;
}

void Entities::FindNewEntities()
{
    FindNewPeds();
    RemoveNullPeds();

    FindNewVehicles();
    RemoveNullVehicles();
}

void Entities::FindNewPeds()
{
    // Guard: pool may be null/transient during game startup/loading
    // (Start New Game reallocates pools). Never touch without validation.
    if (!pPedPool || !*pPedPool) return;
    auto objects = *(GTAPedSA**)(*pPedPool + 0);  // 64-bit CPool: +0 m_pObjects(8)
    tByteFlag* flags = *(tByteFlag**)(*pPedPool + 8);  // 64-bit CPool: +8 m_pFlags(8)
    int size = *(int*)(*pPedPool + 16);  // 64-bit CPool: +16 m_nSize(4)
    if (!objects || !flags || size <= 0 || size > 100000) return;

    for(int i = 0; i < size; ++i)
    {
        if(flags[i].bEmpty) continue;
        auto& ent = objects[i];

        auto ref = GetPedRef(&ent);

        // 🔹 só processa se ainda não vimos esse ped
        if (knownPeds.insert(ref).second)  
        {
            //menuDebug->AddLine("new ped " + std::to_string(ref));

            // aqui você pode chamar Peds::TryCreatePed(ref) ou similar
            menuSZK->onPedAdded->Emit(ref);
        }
    }
}

void Entities::RemoveNullPeds()
{
    if (!pPedPool || !*pPedPool) return;

    auto objects = *(GTAPedSA**)(*pPedPool + 0);  // 64-bit CPool: +0 m_pObjects(8)
    tByteFlag* flags = *(tByteFlag**)(*pPedPool + 8);  // 64-bit CPool: +8 m_pFlags(8)
    int size = *(int*)(*pPedPool + 16);  // 64-bit CPool: +16 m_nSize(4)

    if (!objects || !flags || size <= 0 || size > 100000) return;

    // 🔹 montar set temporário com refs atuais
    std::unordered_set<int> currentRefs;

    for (int i = 0; i < size; ++i)
    {
        if (flags[i].bEmpty) continue;

        GTAPedSA& ped = objects[i];
        int ref = GetPedRef(&ped);
        currentRefs.insert(ref);
    }

    // 🔹 remover do knownPeds qualquer ref que não esteja mais na pool
    for (auto it = knownPeds.begin(); it != knownPeds.end(); )
    {
        if (currentRefs.find(*it) == currentRefs.end())
        {
            auto ref = *it;

            //menuDebug->AddLine("removed ped " + std::to_string(ref));

            it = knownPeds.erase(it);  // remove e retorna próximo iterador

            menuSZK->onPedRemoved->Emit(ref);
        }
        else
            ++it;
    }
}

void Entities::FindNewVehicles()
{
    // Guard: pool may be null/transient during game startup/loading
    // (Start New Game reallocates pools). Never touch without validation.
    if (!pVehiclePool || !*pVehiclePool) return;
    auto objects = *(GTAVehicleSA**)(*pVehiclePool + 0);
    tByteFlag* flags = *(tByteFlag**)(*pVehiclePool + 8);
    int size = *(int*)(*pVehiclePool + 16);
    if (!objects || !flags || size <= 0 || size > 100000) return;

    for(int i = 0; i < size; ++i)
    {
        if(flags[i].bEmpty) continue;
        auto& ent = objects[i];

        auto ref = GetVehicleRef(&ent);

        // 🔹 só processa se ainda não vimos esse ped
        if (knownVehicles.insert(ref).second)  
        {
            //menuDebug->AddLine("new vehicle " + std::to_string(ref));

            menuSZK->onVehicleAdded->Emit(ref);
        }
    }
}

void Entities::RemoveNullVehicles()
{
    if (!pVehiclePool || !*pVehiclePool) return;

    auto objects = *(GTAVehicleSA**)(*pVehiclePool + 0);
    tByteFlag* flags = *(tByteFlag**)(*pVehiclePool + 8);
    int size = *(int*)(*pVehiclePool + 16);

    if (!objects || !flags || size <= 0 || size > 100000) return;

    // 🔹 montar set temporário com refs atuais
    std::unordered_set<int> currentRefs;

    for (int i = 0; i < size; ++i)
    {
        if (flags[i].bEmpty) continue;

        GTAVehicleSA& veh = objects[i];
        int ref = GetVehicleRef(&veh);
        currentRefs.insert(ref);
    }

    // 🔹 remover do knownPeds qualquer ref que não esteja mais na pool
    for (auto it = knownVehicles.begin(); it != knownVehicles.end(); )
    {
        if (currentRefs.find(*it) == currentRefs.end())
        {
            auto ref = *it;

            //menuDebug->AddLine("removed vehicle " + std::to_string(ref));

            it = knownVehicles.erase(it);  // remove e retorna próximo iterador

            menuSZK->onVehicleRemoved->Emit(ref);
        }
        else
            ++it;
    }
}