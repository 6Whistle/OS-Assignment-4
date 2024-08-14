EXEC = D_recompile
cc = gcc

default:
		$(cc) -o drecompile D_recompile.c -lrt
dynamic:
		$(cc) -Ddynamic -o drecompile D_recompile.c -lrt
clean:
		rm -rf D_recompile $(EXEC)