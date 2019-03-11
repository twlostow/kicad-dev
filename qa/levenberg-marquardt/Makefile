CC=g++
OBJS=test_lm.o \
     levenberg_marquardt.o \
     ../../common/geometry/shape.o \
    ../../common/geometry/shape_arc.o \
    ../../common/geometry/shape_line_chain.o \
    ../../common/geometry/shape_collisions.o \
    ../../common/geometry/seg.o \
    ../../common/geometry/geometry_utils.o \
    ../../common/math/math_util.o \
    ../../polygon/clipper.o

CFLAGS=-I../  -I../../polygon/include -Ilevmar-2.6 -std=c++11 -g -I../../include/ -I../../include/geometry `wx-config --cflags` -DWX_COMPATIBILITY
LDFLAGS=-Llevmar-2.6 -llevmar -lm -g `wx-config --libs`

%.o:    %.cpp
	${CC} -c $^ -o $@ $(CFLAGS)

all: $(OBJS)
	${CC} -o test_lm $(OBJS) $(LDFLAGS) 

clean:
	rm -f $(OBJS) test_lm