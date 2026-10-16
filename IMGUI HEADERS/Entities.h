#pragma once

#include "pch.h"

class Entities {
public:
    static void Update();

    static std::unordered_set<int> GetVehiclesRefs();
private:
    static void FindNewEntities();

    static void FindNewPeds();
    static void RemoveNullPeds();

    static void FindNewVehicles();
    static void RemoveNullVehicles();
};