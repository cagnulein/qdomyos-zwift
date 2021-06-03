#ifndef MATERIAL_H
#define MATERIAL_H

#include <QObject>
#include <QtGui/qcolor.h>

class materialHelper : public QObject {
    Q_OBJECT

  public:
    enum Theme { Light, Dark };

    enum Color {
        Red,
        Pink,
        Purple,
        DeepPurple,
        Indigo,
        Blue,
        LightBlue,
        Cyan,
        Teal,
        Green,
        LightGreen,
        Lime,
        Yellow,
        Amber,
        Orange,
        DeepOrange,
        Brown,
        Grey,
        BlueGrey
    };

    enum Shade {
        Shade50,
        Shade100,
        Shade200,
        Shade300,
        Shade400,
        Shade500,
        Shade600,
        Shade700,
        Shade800,
        Shade900,
        ShadeA100,
        ShadeA200,
        ShadeA400,
        ShadeA700,
    };

    Q_ENUM(Theme)
    Q_ENUM(Color)
    Q_ENUM(Shade)

    static QColor color(Color color, Shade shade = Shade500);
    static QColor shade(const QColor &color, Shade shade);
};

#endif // MATERIAL_H
