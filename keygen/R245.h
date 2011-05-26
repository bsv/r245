/*
 * R245.h
 *
 *  Created on: 23.05.2011
 *      Author: bsv
 */

#ifndef R245_H_
#define R245_H_

#include <QLibrary>
#include "r245_types.h"

class R245 {
public:
    R245();
    virtual ~R245();

    bool loadLibrary();

    GetDevInfo R245_GetDevInfo;
    InitDev R245_InitDev;

private:
    QLibrary *lib;
};

#endif /* R245_H_ */
