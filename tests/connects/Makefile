CXXFLAGS =-g `wx-config --cflags` -I../../pcbnew -I../../include -I../../polygon -I../../ 
LDFLAGS = `wx-config --libs` _pcbnew.so
OBJS=connects.o


all:  connects

connects:	$(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)
    

%.o:	%.cpp
	g++ -c $^ $(CXXFLAGS)

clean:
	rm -f $(OBJS) connects