# moc main.cpp -o moc_main.cpp
sh compileLib.sh
sh compileRessources.sh
g++ main.cpp qrc_resources.cpp -fPIC -L./lib -lfolders -o main.exe `pkg-config --cflags --libs Qt5Widgets Qt5Gui Qt5Core  opencv4` -lexiv2
# unset GTK_PATH