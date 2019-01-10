CC=gcc
OBJ=cpuid-check

cpuid.o: cpuid-check.c
	$(CC) -c cpuid-check.c -o $(OBJ)
	ar crU libCpuidCheck.a $(OBJ)
clean:
	rm -rf $(OBJ)
	rm libCpuidCheck.a
