/*
 * R245.cpp
 *
 *  Created on: 23.05.2011
 *      Author: bsv
 */

#include "R245.h"

R245::R245() {
}

R245::~R245() {
    delete lib;
}

bool R245::loadLibrary()
{
    lib = new QLibrary("lib/libr245.dll");

    if(lib->load())
    {
        qDebug("LOAD is OK");
    } else
    {
        qDebug("LOAD isn't ok");
        lib = NULL;
        return false;
    }

    R245_GetDevInfo = (GetDevInfo) lib->resolve("R245_GetDevInfo");
    R245_InitDev = (InitDev) lib->resolve("R245_InitDev");

    return true;
}
