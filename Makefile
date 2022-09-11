TARGET=example1 example2

all: $(TARGET)

%: %.c
	gcc $^ -o $@ -lreadline

clean:
	rm -f example1 example2