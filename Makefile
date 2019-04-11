
all: ir-test

ir-test: ir-test.cpp ir.hpp
	g++ -std=gnu++11 ir-test.cpp -o ir-test

clean:
	rm -f ir-test
