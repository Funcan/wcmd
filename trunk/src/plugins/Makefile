CC=gcc
CFLAGS=-Wall -O2 -g -DDEBUG -fPIC -shared
LDFLAGS=

OBJS=null.o

TARGET=null.so

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM -MT $*.o $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) -MM -MT $*.o $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC)  $^ -o $@ $(CPPFLAGS) $(LDFLAGS)  -shared
	strip -s $@

clean:
	-rm -f $(TARGET) $(OBJS) $(OBJS:.o=.d)

install:
	mkdir ~/.config/wcmd/plugins -p
	install -m 644 $(TARGET) ~/.config/wcmd/plugins/