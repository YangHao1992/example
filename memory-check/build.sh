clang++ -O1 -g -fsanitize=address -fno-omit-frame-pointer test_memory.cpp -o memory
clang++ -fsanitize=thread -g -O1 test_thread.cpp -o thread
./memory
./thread
