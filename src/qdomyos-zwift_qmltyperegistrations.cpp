/****************************************************************************
** Generated QML type registration code
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtQml/qqml.h>
#include <QtQml/qqmlmoduleregistration.h>

#include <inappproductqmltype.h>
#include <inappstoreqmltype.h>
#include <inapptransaction.h>

void qml_register_types_org_cagnulein_qdomyoszwift()
{
    qmlRegisterTypesAndRevisions<InAppProductQmlType>("org.cagnulein.qdomyoszwift", 1);
    qmlRegisterTypesAndRevisions<InAppStoreQmlType>("org.cagnulein.qdomyoszwift", 1);
    qmlRegisterTypesAndRevisions<InAppTransaction>("org.cagnulein.qdomyoszwift", 1);
    qmlRegisterModule("org.cagnulein.qdomyoszwift", 1, 0);
}

static const QQmlModuleRegistration registration("org.cagnulein.qdomyoszwift", 1, qml_register_types_org_cagnulein_qdomyoszwift);
