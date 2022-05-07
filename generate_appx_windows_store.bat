copy icons\iOS\iTunesArtwork@2x.png build-qdomyos-zwift-Qt_5_15_2_for_UWP_64bit_MSVC_2019-Release\release
del build-qdomyos-zwift-Qt_5_15_2_for_UWP_64bit_MSVC_2019-Release\release\qz.appx
cd build-qdomyos-zwift-Qt_5_15_2_for_UWP_64bit_MSVC_2019-Release\release
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\makeappx.exe" pack /d . /p qz
explorer build-qdomyos-zwift-Qt_5_15_2_for_UWP_64bit_MSVC_2019-Release\release
pause