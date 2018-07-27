dos2unix src/*.hex
dos2unix src/*.bin
dos2unix src/*.sym
mv src/*.hex exp/
mv src/*.obj exp/
mv src/*.bin exp/
mv src/*.sym exp/
rm -f src/*.lst
