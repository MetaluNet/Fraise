##
## Made by fabien le mentec <texane@gmail.com>
## 
## Started on  Sun Sep 20 00:28:27 2009 texane
## Last update Sun Feb  7 11:35:53 2010 texane
##


PATH	:=	$(PATH):/usr/share/pk2


CC	:=	/home/texane/proj/present/pic/18f4550/tools/sdcc/bin/sdcc
CFLAGS	:=	-mpic16						\
		--model-large					\
		-p18f4550 -Dpic18f4550				\
		-DSDCC_PIC18					\
		-DCONFIG_ENABLE_SERIAL				\
		-DCONFIG_ENABLE_I2C				\
		-I../src					\
		 -Wa -DSDCC

AS	:=	gpasm
ASFLAGS	:=	-q -p18f4550 -Dpic18f4550 			\
		-I /usr/share/gputils/header 			\
		-DSDCC

LD	:=	gplink
LDFLAGS	:=	-w -r --map -s ../src/18f4550.lkr		\
		-I/usr/local/share/sdcc				\
		-I/usr/local/share/sdcc/lib/pic16
LIBS	:=	libdev18f4550.lib libsdcc.lib libc18f.lib

LIB	:=	gplib -c

NAME	:=	slave.hex

CSRCS	:=	../src/main.c			\
		../src/osc.c			\
		../src/int.c			\
		../src/serial.c			\
		../src/i2c.c
ASRCS	:=

OBJS	:=	$(CSRCS:.c=.o)			\
		$(ASRCS:.asm=.o)

GARBAGE	:=	$(NAME:.hex=.lst)		\
		$(NAME:.hex=.cod)		\
		$(CSRCS:.c=.lst)		\
		$(CSRCS:.c=.cod)		\
		$(CSRCS:.c=.asm)		\
		$(CSRCS:.c=.map)

# rules

all	:	$(NAME)

$(NAME)	:	$(OBJS)
		$(LD) $(LDFLAGS) -o $(NAME) $(OBJS) crt0i.o $(LIBS)

%.o	:	%.c
		$(CC) $(CFLAGS) -c -o $(@) $(<)

%.o	:	%.asm
		$(AS) $(ASFLAGS) -c -o $(@) $(<)

PHONY	:	clean fclean re write erase test

clean	:
		$(RM) $(OBJS)
		$(RM) $(GARBAGE)
		$(RM) slave.map

fclean	:	clean
		$(RM) $(NAME)

re	:	fclean all

write	:	$(NAME)
		pk2cmd -PPIC18f4550 -M -F$(NAME)
erase	:
		pk2cmd -PPIC18F4550 -E
verify	:
		pk2cmd -PPIC18f4550 -Y -F$(NAME)
test	:	write
		../linux/build/serial /dev/ttyUSB0 0
