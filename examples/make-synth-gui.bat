g++ -c ../VSTGui/*.cpp -I"../src/" -DWIN32 -trigraphs -DWINDOWS
g++ -c SawSynth.cpp ../src/FstAudioEffect.cpp -I"../VSTGui/" -I"../src/" -DWINDOWS_GUI -DUNICODE

dllwrap --output-def libPlugin.def --driver-name c++ ^
SawSynth.o FstAudioEffect.o aeffguieditor.o vstgui.o vstcontrols.o ^
-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols --def Plugin.def ^
-o SawSynthWithGui.dll -Ofast

rem Clean up
del *.o