gcc -o memleaktest memleaktest.c -ldl -g \
./memleaktest \
ls ./mem \
cat ./mem/* 
