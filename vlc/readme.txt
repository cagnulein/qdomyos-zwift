copy this file in:
(WINDOWS) %APPDATA%\vlc\lua\intf
(unix) ~/.local/share/vlc/lua/intf
(mac) /Users/%your_name%/Library/Application Support/org.videolan.vlc/lua/intf
Start vlc with this command line parameter
--extraintf=luaintf --lua-intf=cli --lua-config="cli={host='0.0.0.0:4212'}"