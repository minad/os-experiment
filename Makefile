all: tags kernel

kernel:
	(cd src; make)

tags:
	ctags src/*.c include/*.h

clean:
	(cd src; make clean)
