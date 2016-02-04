CXXFLAGS =-g `wx-config --cflags` -I../../pcbnew -I../../include -I../../polygon -I../../ 
LDFLAGS = `wx-config --libs` ../../pcbnew/_pcbnew.so
CXX=clang++

all:  $(TEST)

connects:	$(OBJS)
	${CXX} -o $(TEST) $(OBJS) $(LDFLAGS)
    

%.o:	%.cpp
	${CXX} -c $^ $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(TEST)