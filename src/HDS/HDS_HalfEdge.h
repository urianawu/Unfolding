#pragma once
#include "HDS/hds_common.h"

class HDS_HalfEdge : public HDS_Common
{
private:
	static hdsid_t uid;
public:
	static void resetIndex() { uid = 0; }
	static void matchIndexToSize(size_t	size) { uid = size; }

	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_EDGE; }

	HDS_HalfEdge();
	~HDS_HalfEdge();
	
	// Get the explicit pointer to corresponding edges
	HDS_HalfEdge* prev() { return this + prev_offset; }
	HDS_HalfEdge* next() { return this + next_offset; }
	HDS_HalfEdge* flip() { return this + flip_offset; }
	HDS_HalfEdge* rotCW() { return flip()->next(); }
	HDS_HalfEdge* rotCCW() { return prev()->flip(); }
	HDS_HalfEdge* cutTwin() { return this + cutTwin_offset; }
	HDS_HalfEdge* bridgeTwin() { return this + brt_offset; }
	const HDS_HalfEdge* prev() const { return this + prev_offset; }
	const HDS_HalfEdge* next() const { return this + next_offset; }
	const HDS_HalfEdge* flip() const { return this + flip_offset; }
	const HDS_HalfEdge* rotCW() const { return flip()->next(); }
	const HDS_HalfEdge* rotCCW() const { return prev()->flip(); }
	const HDS_HalfEdge* cutTwin() const { return this + cutTwin_offset; }
	const HDS_HalfEdge* bridgeTwin() const { return this + brt_offset; }

	// Get the connected vertex/face id
	// Explicit pointer access is handled by HDS_Mesh
	hdsid_t faceID() const { return fid; }
	hdsid_t vertID() const { return vid; }

	void setPicked(bool v) { isPicked = v; flip()->isPicked = v; }
	void setCutEdge(bool v) { isCutEdge = v; flip()->isCutEdge = v; }
	void setFlip(HDS_HalfEdge* f_e) {
		flip_offset = f_e - this;
		f_e->flip_offset = -flip_offset;
	}
	void setBridgeTwin(HDS_HalfEdge* bt_e) {
		brt_offset = bt_e - this;
		bt_e->brt_offset = -brt_offset;
	}

	uint16_t getFlag() const { return flag; }
	// TODO: move to mesh factory
	void computeCurvature();
	QVector3D computeNormal();
public:
	hdsid_t index;
	hdsid_t refid;

	hdsid_t fid;
	hdsid_t vid;

	// Offset to index of previous/nex/flip edge
	// previous/nex/flip edge doesn't exist, when (previous/nex/flip == 0)
	int32_t prev_offset, next_offset, flip_offset;

	int32_t cutTwin_offset;// pointer to its twin halfedge created in a cut event
	int32_t brt_offset;// Bridge-Tween Edge Offset

	union
	{
		uint16_t flag;
		struct
		{
			bool : 1;
			bool isPicked : 1;
			bool isCutEdge : 1;
			bool isExtended : 1;//From hollower
			bool isNegCurve : 1;
			bool isJoint : 1;//From woven
		};
	};
	
	Float angle;
};
