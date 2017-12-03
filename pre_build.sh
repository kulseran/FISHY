mkdir .build
cd .build

unameOut="$(uname -s)"
case "$unameOut}" in
	MINGW*) cmake ../ -G "Visual Studio 15 2017 Win64";;
	*) cmake ../;;
esac
