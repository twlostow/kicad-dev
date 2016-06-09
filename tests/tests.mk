CXXFLAGS =-g `wx-config --cflags` -I../../pcbnew -I../../include -I../../polygon -I../../ \
-DHAVE_STDINT_H -DKICAD_KEEPCASE -DUSE_OPENMP -DWXUSINGDLL -DWX_COMPATIBILITY -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -DPCBNEW -fvisibility=default -std=c++11 \
-Wno-unused-local-typedefs -Wno-strict-aliasing -fopenmp -pthread -g3 -ggdb3 -DDEBUG -Wno-deprecated-declarations -fPIC

LDFLAGS = `wx-config --libs` ../../pcbnew/_pcbnew.kiface
CXX=g++


all:  $(TEST)

connects:	$(OBJS)
	${CXX} -o $(TEST) $(OBJS) $(LDFLAGS)
    

%.o:	%.cpp
	${CXX} -c $^ $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(TEST)