#pragma once

typedef int GUID;

struct GuidDescriptor
{
	int entityType;
	void* data;
};

void Guid_Init(int maxEntities);
void Guid_CheckTable();
GUID Guid_AddToGUIDTable(int entityType, void* entityData);
void Guid_SwapGuidDescriptors(GUID guid1, GUID guid2);
GuidDescriptor Guid_GetDescriptor(GUID guid);