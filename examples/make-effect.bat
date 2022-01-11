g++ -c GainPlugin.cpp ../src/FstAudioEffect.cpp -I"../src" -w

dllwrap  --output-def libPlugin.def  --driver-name c++ ^
GainPlugin.o FstAudioEffect.o ^
-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols --def Plugin.def ^
-o Gain.dll -Ofast

rem Clean up
del *.o