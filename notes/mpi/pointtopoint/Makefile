
CC = mpicc -g -Wall -O3
RM = rm -f

%.o: %.c
	$(CC) -c -o $@ $<

%: %.o
	$(CC) -o $@ $<

pointtopoint: pointtopoint.o

pointtopoint-2: pointtopoint-2.o
pointtopoint-3: pointtopoint-3.o
pointtopoint-4: pointtopoint-4.o
pointtopoint-5: pointtopoint-5.o
pointtopoint-6: pointtopoint-6.o
pointtopoint-7: pointtopoint-7.o
pointtopoint-8: pointtopoint-8.o
pointtopoint-9: pointtopoint-9.o

clean:
	$(RM) *.o

.PHONY: clean
