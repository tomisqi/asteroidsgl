#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "guid.h"

struct GuidData
{
	GUID gguid; // global
	GuidDescriptor* guidTable;
	int maxEntities;
};

static GuidData guidData;

static GUID GetNextGUID()
{
	return guidData.gguid++;
}

void Guid_Init(int maxEntities)
{
	guidData.guidTable = (GuidDescriptor*)malloc(sizeof(GuidDescriptor) * maxEntities);
	memset(guidData.guidTable, 0, sizeof(GuidDescriptor) * maxEntities);
	guidData.maxEntities = maxEntities;

	// first guid in the table is a dummy
	GuidDescriptor dummyDesc = { -1, NULL };
	guidData.guidTable[0] = dummyDesc;

	guidData.gguid = 1;
}

void Guid_CheckTable()
{
	// sanity check for the guidTable
	for (int i = 1; i < guidData.maxEntities; i++)
	{
		void* data = guidData.guidTable[i].data;
		assert(data != NULL);						// should not be null
		for (int j = i + 1; j < guidData.maxEntities; j++)
		{
			assert(data != guidData.guidTable[j].data); // ... and should be different from others 
		}
	}
}

GUID Guid_AddToGUIDTable(int entityType, void* entityData)
{
	GuidDescriptor desc = { entityType, entityData };
	GUID guid = GetNextGUID();
	assert(guid < guidData.maxEntities);
	assert(guidData.guidTable[guid].data == NULL);//making sure we are not overwriting
	guidData.guidTable[guid] = desc;
	return guid;
}

void Guid_SwapGuidDescriptors(GUID guid1, GUID guid2)
{
	GuidDescriptor tmp = guidData.guidTable[guid1];
	guidData.guidTable[guid1] = guidData.guidTable[guid2];
	guidData.guidTable[guid2] = tmp;
}

GuidDescriptor Guid_GetDescriptor(GUID guid)
{
	return guidData.guidTable[guid];
}