# FULL NAME: Rabia Qureshi
# STUDENT ID: 1046427
# DATE: April 11, 2021
# ASSIGNMENT 4

# MAKEFILE GENERAL SYNTAX
# target: prerequisites
#	commands

# $^ is the names of all the prerequisites with spaces between them
# $< is the name of the first prerequisite
# $@ is the name of the target

CC = gcc
CFLAGS = -Wpedantic -std=gnu99 -g

FSReport: FSReport.o
	$(CC) $(CFLAGS) $^ -o $@

FSReport.o: FSReport.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o FSReport
