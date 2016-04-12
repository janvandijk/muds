CC      = gcc
PROF    =
C_FLAGS = -O -Wall $(PROF)
L_FLAGS = -O $(PROF) -lcrypt

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o magic.o save.o special.o update.o

merc: $(O_FILES)
	rm -f merc
	$(CC) -o merc $(O_FILES) $(L_FLAGS)

clean:
	rm *.o merc

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<
