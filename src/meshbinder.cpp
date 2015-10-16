#include "meshbinder.h"
#include "MeshExtender.h"

void MeshBinder::bindingMesh(HDS_Mesh* thismesh)
{
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	//keep track of original edges and faces
	vector<face_t*> old_faces;
	unordered_set<he_t*> old_edges;
	for (auto he: thismesh->heSet) {
		if (old_edges.find(he->flip) == old_edges.end())
		old_edges.insert(he);
	}

	for (auto f: thismesh->faceSet) {
		f->setScaleFactor(HDS_Connector::getScale());
		old_faces.push_back(f);
	}
	for (auto f: old_faces) {
		thismesh->deleteFace(f);
	}
	//clear old mesh
	thismesh->vertSet.clear();
	thismesh->vertMap.clear();
	for (auto he: old_edges) {
		thismesh->deleteHalfEdge(he);
		thismesh->deleteHalfEdge(he->flip);
	}


	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();

	//for each edge
	//get connector
	vector<vert_t*> vertices_new;
	vector<he_t*> hes_new;
	//set new connector on each edge
	for (auto he: old_edges) {
		cout<<"connector based on original edge "<<he->index<<endl;
		//get edge vertex, calculate scaled vertex
		vert_t* he_v = he->v;
		vert_t* he_flip_v = he->flip->v;
		face_t* he_f = he->f;
		face_t* he_flip_f = he->flip->f;

		vert_t* he1_v1 = new vert_t(he_f->scaleCorner(he_v));
		vert_t* he1_v2 = new vert_t(he_f->scaleCorner(he_flip_v));
		vert_t* he2_v1 = new vert_t(he_flip_f->scaleCorner(he_v));
		vert_t* he2_v2 = new vert_t(he_flip_f->scaleCorner(he_flip_v));

		vertices_new.push_back(he1_v1);
		vertices_new.push_back(he1_v2);
		vertices_new.push_back(he2_v1);
		vertices_new.push_back(he2_v2);

		//new edge pair based on new vertex position
		he_t* he1 = thismesh->insertEdge(he1_v1, he1_v2);
		he_t* he2 = thismesh->insertEdge(he2_v1, he2_v2);
		hes_new.push_back(he1);
		hes_new.push_back(he1->flip);
		hes_new.push_back(he2);
		hes_new.push_back(he2->flip);

		//set edge cut face
		face_t * cutFace = new face_t;
		cutFace->index = HDS_Face::assignIndex();
		cutFace->isCutFace = true;
		cutFace->he = he1;
		he1->setCutEdge(true);
		he2->flip->f = cutFace;
		he2->setCutEdge(true);
		thismesh->addFace(cutFace);

		//add connector
		he1->f = he_f;//pass original face to addConnector function

		vector<vert_t*> verts = MeshExtender::addConnector(thismesh, he1->flip, he2, cutFace);
		vertices_new.insert( vertices_new.end(), verts.begin(), verts.end() );
		he1->f = cutFace;

		//build he->f and he->flip->f scaled faces from he and he->flip


	}



	//add new vertices and edges
	for (auto v: vertices_new) {
		v->index = HDS_Vertex::assignIndex();
		thismesh->addVertex(v);
	}
	for (auto he: hes_new) {
		he->index = HDS_HalfEdge::assignIndex();
		thismesh->addHalfEdge(he);
	}

	/// update the curvature of each vertex
	for (auto &v : thismesh->vertSet) {
		v->computeNormal();
		v->computeCurvature();
		//cout << v->index << ": " << (*v) << endl;
	}
}
