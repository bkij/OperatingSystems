#!/bin/bash

echo -e "Generating files...\n"
./io_benchmark.out sys generate file_4 2000 4
./io_benchmark.out sys generate file_512 2000 512
./io_benchmark.out sys generate file_4096 2000 4096
./io_benchmark.out sys generate file_8192 2000 8192

echo -e "Sorting...\n"
cp file_4 file_4_lib
echo -e "\n4 bytes, sys:\n"
time ./io_benchmark.out sys sort file_4 2000 4
echo -e "\n4 bytes, lib:\n"
time ./io_benchmark.out lib sort file_4_lib 2000 4

cp file_512 file_512_lib
echo -e "\n512 bytes, sys:\n"
time ./io_benchmark.out sys sort file_512 2000 512
echo -e "\n512 bytes, lib:\n"
time ./io_benchmark.out lib sort file_512_lib 2000 512

cp file_4096 file_4096_lib
echo -e "\n4096 bytes, sys:\n"
time ./io_benchmark.out sys sort file_4096 2000 4096
echo -e "\n4096 bytes, lib:\n"
time ./io_benchmark.out lib sort file_4096_lib 2000 4096

cp file_8192 file_8192_lib
echo -e "\n8192 bytes, sys:\n"
time ./io_benchmark.out sys sort file_8192 2000 8192
echo -e "\n8192 bytes, lib:\n"
time ./io_benchmark.out lib sort file_8192_lib 2000 8192

echo -e "Permuting...\n"
echo -e "\n4 bytes, sys:\n"
time ./io_benchmark.out sys shuffle file_4 2000 4
echo -e "\n4 bytes, lib:\n"
time ./io_benchmark.out lib shuffle file_4 2000 4

echo -e "\n512 bytes, sys:\n"
time ./io_benchmark.out sys shuffle file_512 2000 512
echo -e "\n512 bytes, lib:\n"
time ./io_benchmark.out lib shuffle file_512 2000 512

echo -e "\n4096 bytes, sys:\n"
time ./io_benchmark.out sys shuffle file_4096 2000 4096
echo -e "\n4096 bytes, lib:\n"
time ./io_benchmark.out lib shuffle file_4096 2000 4096

echo -e "\n8192 bytes, sys:\n"
time ./io_benchmark.out sys shuffle file_8192 2000 8192
echo -e "\n8192 bytes, lib:\n"
time ./io_benchmark.out lib shuffle file_8192 2000 8192
