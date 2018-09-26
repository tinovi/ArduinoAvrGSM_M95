echo -n "" > config.h
echo "#ifndef CONFIG_H_" >>config.h
echo "#define CONFIG_H_">>config.h
echo >> config.h
echo "#define APITYPE  \"GSMMOD1\"" >>config.h
echo >>config.h

ttt=$(wget -qO- http://tinovi.io/api/v1/node/dsid/11796630-dad7-11e4-9261-c198fb8c6d97/GSMMOD1)

echo "#define APIKEY \"$ttt\"" >>config.h
echo >>config.h
echo "#define PIN \"1111\" ">>config.h
echo  >>config.h
echo "#define VERSION 0" >>config.h
echo >>config.h
echo "#endif /* CONFIG_H_ */" >>config.h

