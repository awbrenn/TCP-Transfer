PROG = exam_program

SOURCE = awbrennExam2.c
OBJS = $(SOURCE:.c=.o)

all: $(PROG)

exam_program: $(SOURCE)
	cc -o exam_program awbrennExam2.c

#test:
#	./dnsq -t 5 -r 5 @192.168.1.254 www.clemson.edu

backup:
	rm -f awbrenn-hw3.tar *.o *.out client *~
	tar -cvzf awbrenn-hw3.tar.gz *.cpp *.h readme.txt Makefile

server:
	./exam_program 1 8302 file

client:
	./exam_program 0 127.0.0.1 8302 high_res_image.jpg

clean:
	rm -f $(PROG) *.o *.out exam_program *~
