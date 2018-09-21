CFILES   := $(wildcard ../*.c)
CXXFILES := $(wildcard *.cpp)

OFILES   := $(patsubst ../%,%,$(CFILES:.c=.c.o))
OXXFILES := $(CXXFILES:.cpp=.cpp.o)

CPPFLAGS := -Wall -g -I../../../../include -O2 \
            -D"LightLock=pthread_mutex_t" \
            -D"LightLock_Init(x)=pthread_mutex_init(x, NULL)" \
            -D"LightLock_Lock(x)=pthread_mutex_lock(x)" \
            -D"LightLock_Unlock(x)=pthread_mutex_unlock(x)"

CFLAGS   := $(CPPFLAGS)
CXXFLAGS := $(CPPFLAGS) -std=c++11

all: rbtree_test

rbtree_test: $(OFILES) $(OXXFILES)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OFILES): %.c.o: ../%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(OXXFILES): %.cpp.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	$(RM) rbtree_test $(OFILES) $(OXXFILES)
