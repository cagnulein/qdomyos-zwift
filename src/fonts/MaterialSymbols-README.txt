MaterialSymbolsQZ.ttf - icon font of the modern look (UiIcon.qml, names in UiIcons.js).

A subset of Material Symbols Rounded by Google (https://github.com/google/material-design-icons,
variablefont/), Apache License 2.0 (MaterialSymbols-LICENSE.txt). The variable font is fixed to
the instance FILL 0, GRAD 0, opsz 24, wght 400 and cut down to the glyphs listed in UiIcons.js,
so a new icon needs its line "name": 0x<code point> in UiIcons.js and the subset made again
with fontTools (instancer, then subset to those code points).
