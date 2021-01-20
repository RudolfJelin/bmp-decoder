zip exam-impl-test.zip *.c *.h decodedBMP.bmp;
clang main.c -pedantic -Wall -Werror -g -o main;
#valgrind --leak-check=full ./main;
valgrind ./main in.bmp out.bmp;
echo Returned: $?;
