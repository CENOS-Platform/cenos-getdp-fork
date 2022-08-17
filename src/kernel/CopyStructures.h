#include "ProData.h"
#include "GeoData.h"
#include "DofData.h"
#include "MallocUtils.h"

DefineSystem * CopyDefineSystem(DefineSystem* d);
//Tree_T * Tree_Copy(Tree_T* t);
GeoData * CopyGeoData(GeoData* g);
//Grid Grid_Copy(Grid grid);
DofData * CopyDofData(DofData * otherDofData);
//Element * CopyElement(Element * otherElement);
char * CharCopy(char * data);
CurrentData * CopyCurrentData(CurrentData* c);
