
run_ref: clean ref
	./ref

ref: ref.c
	gcc -g  -fno-omit-frame-pointer -rdynamic -o $@ $^ 
#-lunwind


clean: 
	rm -f ref