/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -i ../revocation/revocationmanager.h -c RevocationMgrAdaptor -l RevocationManager -a ../dbusservice/dbusadaptor/revocationmgr_adaptor revocation.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "../dbusservice/dbusadaptor/revocationmgr_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class RevocationMgrAdaptor
 */

RevocationMgrAdaptor::RevocationMgrAdaptor(RevocationManager *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

RevocationMgrAdaptor::~RevocationMgrAdaptor()
{
    // destructor
}

int RevocationMgrAdaptor::popEvent()
{
    // handle method call com.deepin.filemanager.daemon.RevocationManager.popEvent
    return parent()->popEvent();
}

void RevocationMgrAdaptor::pushEvent(int event)
{
    // handle method call com.deepin.filemanager.daemon.RevocationManager.pushEvent
    parent()->pushEvent(event);
}

