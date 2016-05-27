#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
class TangramWidget;

void processNetworkQueue();

void finishUrlRequests();

void registerWidget(TangramWidget *widget);

#endif // PLATFORM_QT_H

