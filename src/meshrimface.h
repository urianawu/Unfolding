#ifndef MESHRIMFACE_H
#define MESHRIMFACE_H

#include "common.h"
#include "MeshExtender.h"

class MeshExtender;

class MeshRimFace : public MeshExtender
{
public:
	MeshRimFace();
	static void rimMesh3D(HDS_Mesh *mesh);
	~MeshRimFace();

};

#endif // MESHRIMFACE_H
