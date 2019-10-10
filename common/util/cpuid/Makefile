CC=gcc
OBJ=cpuid-check
SECURE_CFLAGS=-fstack-protector-strong -fPIE -fPIC -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security
SECURE_LDFLAGS=-z noexecstack -z relro -z now -pie

cpuid.o: cpuid-check.c
	$(CC) $(SECURE_CFLAGS) $(SECURE_LDFLAGS) -c cpuid-check.c -L. -lsafestring -o $(OBJ)
	ar crU libCpuidCheck.a $(OBJ)
clean:
	rm -rf $(OBJ)
	rm libCpuidCheck.a
