CC=gcc
CXX=g++
CFLAGS=-g -Wall -O2 -I..
CXXFLAGS=$(CFLAGS)
PROGS=test-task-queue test-ep test-select test-timer test-evloop test-skiplist test-tcpserver test-udpserver test-unixserver test-singly-list

all: $(PROGS)

clean:
	rm -rf $(PROGS) *.dSYM a.out *.o

.PHONY: clean

test-task-queue: test_task_queue.c ../ep_task_queue.h
	$(CC) $(CFLAGS) -o $@ test_task_queue.c

test-ep: test_ep.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_ep.c

test-select: test_ep.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_ep.c -DUSE_SELECT

test-timer: test_timer.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_timer.c

test-evloop: test_evloop.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_evloop.c

test-evloop-foreach: test_evloop_foreach.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_evloop_foreach.c

test-skiplist: test_skiplist.c ../skiplist.h
	$(CC) $(CFLAGS) -o $@ test_skiplist.c

test-tcpserver: test_tcpserver.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_tcpserver.c

test-udpserver: test_udpserver.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_udpserver.c

test-unixserver: test_unixserver.c ../*.h
	$(CC) $(CFLAGS) -o $@ test_unixserver.c

test-singly-list: test_singly_list.c ../singly_list.h
	$(CC) $(CFLAGS) -o $@ test_singly_list.c
