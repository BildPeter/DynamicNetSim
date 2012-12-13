run_traversal: sets.o traversal.o read.o run_traversal.o
	g++ -o run_traversal run_traversal.o sets.o traversal.o read.o

run_traversal.o: run_traversal.cpp
	g++ -c run_traversal.cpp

traversal.o: traversal.cpp
	g++ -c traversal.cpp

sets.o: sets.cpp
	g++ -c sets.cpp
  
read.o: read.cpp
	g++ -c read.cpp

clean:
	rm *.o *~

