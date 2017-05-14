#!/bin/bash

# SIGKILL - 9
# SIGTERM - 15
# SIGSTOP - 19
# SIGUSR1 - 10

./record_gen.py infile

echo -e "RAPORT ZAD2\n\n" > zad2.raport
echo -e "INFORMACJE SYSTEMOWE:\n" >> zad2.raport
uname -a >> zad2.raport
echo -e "\n\n"

for Variant in {1..5}
do
    echo -e "VARIANT $Variant:\n" >> zad2.raport
    for Sig in 9 15 10
    do
        echo -e "Signal number $Sig:\n" >> zad2.raport
        ./reader.out 5 infile 5 a 3 $Variant $Sig >> zad2.raport
        echo -e "\n\n" >> zad2.raport
    done
done