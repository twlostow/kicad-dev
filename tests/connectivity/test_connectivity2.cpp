#include <class_board.h>
#include <io_mgr.h>

#include "connectivity.h"

CN_CONNECTIVITY_ALGO *connectivity;

void removeItem(BOARD_ITEM *item)
{
    connectivity->Remove(item);
}

void addItem(BOARD_ITEM *item)
{
    connectivity->Add(item);
}

void case0()
{
connectivity = new CN_CONNECTIVITY_ALGO;
#include "case0.cpp"
//printf("Unconnected: %d\n", connectivity->GetUnconnectedCount());
assert( connectivity->GetUnconnectedCount() == 0 );
delete connectivity;
}

void case1()
{
connectivity = new CN_CONNECTIVITY_ALGO;
#include "case1.cpp"
//printf("Unconnected: %d\n", connectivity->GetUnconnectedCount());
assert( connectivity->GetUnconnectedCount() == 0 );
delete connectivity;
}



int main( int argc, char* argv[] )
{
  case0();
  case1();
  return 0;
}
