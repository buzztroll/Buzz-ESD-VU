

esd1: esd1.c
	gcc -o esd1 esd1.c -lesd

esd2: esd2.c
	gcc -o esd2 esd2.c -lesd

clean:
	rm -f esd1 esd2
