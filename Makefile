all:
	@echo "make english - 英語版"
	@echo "make japan   - 日本語版"

english:
	gcc -o convert convert.c
	./convert > MODDING_english.csv 2> err

japan:
	gcc -o convert convert.c -DJAPANESE -DENGLISH
	./convert > MODDING.csv 2> err
	
release:	
	gcc -o convert convert.c -DJAPANESE
	./convert > MODDING.csv 2> err
	

gdb:
	gdb ./convert

clean:
	rm convert 

convert: convert.c
	gcc -g -o $@ $<




