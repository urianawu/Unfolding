#define NOMINMAX

#include "meshmanager.h"
#include <iostream>
//#include "glutils.hpp"
#include "mathutils.hpp"
#include "utils.hpp"
#include "MeshViewerLegacy.h"
#include "morsesmalecomplex.h"

#include <QMouseEvent>

using namespace std;

MeshViewerLegacy::MeshViewerLegacy(QWidget *parent) :
	QGLWidget(qglformat_3d, parent)
{
	interactionState = SelectVertex;
	selectionMode = SingleSelect;
	view_cam.updateModelView();
	heMesh = nullptr;
	colormap = ColorMap::getDefaultColorMap();

	lightingState = Wireframe;

	showReebPoints = false;

	showCut = false;
	showMultCut = false;

	showText = false;
	showVIndex = true;
	showCLDistance = false;
	showCPDistance = false;

	lastSelectedIndex = 0;

	cmode =Random;//PointNormal;//Geodesics;
	cp_smoothing_times = 0;
	cp_smoothing_type = 0;
	scale = 1.0f;
	nn=0;
	nm=0;
	mm=0;


	setStatusTip(tr("Hold Alt to rotate camera, Shift to move camera, Ctrl to zoom"));

}

MeshViewerLegacy::~MeshViewerLegacy()
{

}

void MeshViewerLegacy::bindHalfEdgeMesh(HDS_Mesh *mesh)
{
	heMesh = mesh;
	//findReebPoints();
	updateGL();
}

void MeshViewerLegacy::setCurvatureColormap(ColorMap cmap)
{
	colormap = cmap;
	updateGL();
}

bool MeshViewerLegacy::QtUnProject(const QVector3D& pos_screen, QVector3D& pos_world)
{
	bool isInvertible;
	QMatrix4x4 proj_modelview_inv = view_cam.projectionModelView().inverted(&isInvertible);//not understood
	if (isInvertible)
	{
		QVector3D pos_camera;
		pos_camera.setX((pos_screen.x() - (float)view_cam.viewport.x) / (float)view_cam.viewport.w*2.0 - 1.0);
		pos_camera.setY((pos_screen.y() - (float)view_cam.viewport.y) / (float)view_cam.viewport.h*2.0 - 1.0);
		pos_camera.setZ(2.0*pos_camera.z() - 1.0);
		pos_world = (proj_modelview_inv*QVector4D(pos_camera, 1.0f)).toVector3DAffine();

	}

	return isInvertible;
}
void MeshViewerLegacy::selectAll()
{
	switch (interactionState) {

	case SEL_FACE:
		for (auto f : heMesh->faces())
			f->setPicked(true);
		break;
	case SelectEdge:
		for (auto e : heMesh->halfedges())
			e->setPicked(true);
		break;
	case SelectVertex:
		for (auto v : heMesh->verts())
			v->setPicked(true);
		break;
	default:
		break;
	}
	updateGL();
}

void MeshViewerLegacy::selectInverse()
{
	switch (interactionState) {
	case SEL_FACE: {
		for (auto f : heMesh->faces())
			heMesh->selectFace(f->index);
		break;
	}
	case SelectEdge: {

		unordered_set<HDS_HalfEdge*> selected = heMesh->getSelectedHalfEdges();


		for (auto e : heMesh->halfedges()) {
			if (selected.find(e) != selected.end())
				e->setPicked(false);
			else
				e->setPicked(true);
		}
		break;
	}
	case SelectVertex: {
		for (auto v : heMesh->verts())
			heMesh->selectVertex(v->index);
		break;
	}
	default:
		break;
	}
	updateGL();

}

void MeshViewerLegacy::selectCC()
{
	switch (interactionState) {

	case SEL_FACE:

		break;
	case SelectEdge:

		break;
	case SelectVertex:

		break;
	default:
		break;
	}
	updateGL();

}

void MeshViewerLegacy::selectTwinPair()
{
	switch (interactionState) {
	case SelectEdge:
		if (!heMesh->getSelectedHalfEdges().empty()) {
			for (auto he :heMesh->getSelectedHalfEdges()) {
				if (he->cutTwin != nullptr){
					he->cutTwin->setPicked(true);
				} else if (he->flip->cutTwin != nullptr){
					he->flip->cutTwin->setPicked(true);

				}
			}
		}
		break;

	default:
		break;
	}
	updateGL();
}

void MeshViewerLegacy::selectNextEdge()
{
	switch (interactionState) {
	case SelectEdge:
		if (!heMesh->getSelectedHalfEdges().empty()) {
			for (auto he :heMesh->getSelectedHalfEdges()) {
				if (he->next != nullptr)
					he->next->setPicked(true);
			}
		}
		break;
	default:
		break;
	}
	updateGL();
}

void MeshViewerLegacy::selectCP()
{
	for (auto p : reebPoints) {
		p->setPicked(true);
	}
	reebPoints.clear();
	updateGL();
}



void MeshViewerLegacy::selectMSTEdges()
{
	selectClear();

	//find MST(with no weight), face as "graph node", edge between faces as "graph edge"
	unordered_set<HDS_Face*> MSTnodes;
	PQ_MST_Face faces;

	HDS_Face* tmp = *(heMesh->faces().begin());
	Face init(tmp,0);
	faces.push(init);
	MSTnodes.emplace(tmp);

	int depth = 0;
	unordered_set<HDS_HalfEdge*> MSTedges;
	while (!faces.empty()) {
		Face currentFace = faces.top();
		faces.pop();
		if (currentFace.parent != nullptr)
			MSTedges.emplace(heMesh->incidentEdge(currentFace.parent,currentFace.f));

		for(auto adjacentFace: heMesh->incidentFaces(currentFace.f)) {
			if (MSTnodes.find(adjacentFace) == MSTnodes.end()) {
				MSTnodes.emplace(adjacentFace);
				Face adj(adjacentFace,++depth);
				adj.setParent(currentFace.f);
				faces.push(adj);
			}
		}
	}

	cout<<"mst edges #:::"<<MSTedges.size()<<endl;
	for (auto he: heMesh->halfedges()) {
		if (MSTedges.find(he) == MSTedges.end() && MSTedges.find(he->flip) == MSTedges.end())
			he->setPicked(true);
	}
}

void MeshViewerLegacy::selectGrow()
{
	//get all neighbours
	switch (interactionState) {

	case SEL_FACE:
		for (auto f : heMesh->getSelectedFaces()) {
			for (auto face : heMesh->incidentFaces(f)) {
				face->setPicked(true);
			}
		}
		break;
	case SelectEdge:
		for (auto e : heMesh->getSelectedHalfEdges()) {
			for (auto edge : heMesh->incidentEdges(e->v)) {
				edge->setPicked(true);

			}
		}
		break;
	case SelectVertex:
		for (auto vert : heMesh->getSelectedVertices()) {
			for (auto v : vert->neighbors()) {
				v->setPicked(true);
			}
		}
		break;
	default:
		break;
	}
	updateGL();

}

void MeshViewerLegacy::selectShrink()
{
	//BFS to get all neighbours that are selected
	switch (interactionState) {
	case SEL_FACE:
		for (auto f : heMesh->getSelectedFaces()) {

		}
		break;
	case SelectEdge:
		for (auto e : heMesh->getSelectedHalfEdges()) {
			for (auto edge : heMesh->incidentEdges(e->v)) {
				edge->setPicked(true);

			}
		}
		break;
	case SelectVertex:
		for (auto vert : heMesh->getSelectedVertices()) {
			for (auto v : vert->neighbors()) {
				v->setPicked(true);
			}
		}
		break;
	default:
		break;
	}
	updateGL();
}

void MeshViewerLegacy::selectClear()
{
	resetEdges();
	resetFaces();
	resetVertices();
	updateGL();

}

void MeshViewerLegacy::resetEdges()
{
	//reset all edges
	for (auto he: heMesh->halfedges())
		he->setPicked(false);
}

void MeshViewerLegacy::resetVertices()
{
	//reset all vertices
	for (auto v: heMesh->verts())
		v->setPicked(false);

}

void MeshViewerLegacy::resetFaces()
{
	//reset all faces
	for (auto f: heMesh->faces())
		f->setPicked(false);
}


void MeshViewerLegacy::disablecpp()
{
	isCriticalPointModeSet = false;
	showVIndex = true;
	showCPDistance = false;
	updateGL();
}

void MeshViewerLegacy::disableclp()
{
	isCutLocusModeset = false;
	showVIndex = true;
	showCLDistance = false;
	showCut = false;

	updateGL();
}

int MeshViewerLegacy::getSelectedElementIndex(const QPoint &p)
{
	int winX = p.x(), winY = height() - p.y();

	auto max = [](int a, int b) { return a > b ? a : b; };
	auto min = [](int a, int b) { return a < b ? a : b; };

	// search for pixels within a small window
	const int radius = 5;
	map<int, int> counter;
	int maxIdx = -1, maxCount = 0;
	for (int y = max(winY - radius, 0); y < min(winY + radius, height()); ++y) {
		int dy = y - winY;
		for (int x = max(winX - radius, 0); x < min(winX + radius, width()); ++x) {
			int dx = x - winX;
			if (dx*dx + dy*dy <= radius*radius) {
				int offset = (y * width() + x) * 4;
				unsigned char r, g, b, a;
				r = selectionBuffer[offset + 0];
				g = selectionBuffer[offset + 1];
				b = selectionBuffer[offset + 2];
				a = selectionBuffer[offset + 3];

				if (a == 0) continue;
				else {
					int idx = decodeIndex(r, g, b, 1.0);
					auto it = counter.find(idx);
					if (it == counter.end()) {
						counter.insert(make_pair(idx, 1));
						if (maxCount == 0) {
							maxCount = 1;
							maxIdx = idx;
						}
					}
					else {
						++it->second;
						if (it->second > maxCount) {
							maxCount = it->second;
							maxIdx = idx;
						}
					}
				}
			}
		}
	}

	lastSelectedIndex = maxIdx;

	return maxIdx;
}
/*
void MeshViewer::computeGlobalSelectionBox()
{
	/// get GL state
	BOOL DD=0,ED=0,DF=0,EF=0;
	GLint m_GLviewport[4];
	GLdouble m_GLmodelview[16];
	GLdouble m_GLprojection[16];
	glGetIntegerv(GL_VIEWPORT, m_GLviewport);           // Retrieves The Viewport Values (X, Y, Width, Height)
	glGetDoublev(GL_MODELVIEW_MATRIX, m_GLmodelview);       // Retrieve The Modelview Matrix
	glGetDoublev(GL_PROJECTION_MATRIX, m_GLprojection);     // Retrieve The Projection Matrix

	//Not know why, but it solves the problem, maybe some issue with QT
	if (width() < height())
		m_GLviewport[1] = -m_GLviewport[1];

	GLdouble winX = sbox.corner_win[0];
	GLdouble winY = sbox.corner_win[1];
	QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[0]);//return isVisible
	//qDebug()<<sbox.gcorners[0];
	DD = gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global, sbox.corner_global + 1, sbox.corner_global + 2);//The new position of the mouse
	//qDebug()<<sbox.corner_global[0]<<sbox.corner_global[1]<<sbox.corner_global[2];
	
	//cout<<"DD"<<endl<<DD<<endl;
	//cout<<&sbox.corner_global<<endl;
	//cout<<&sbox.corner_global+1<<endl;
	//cout<<&sbox.corner_global+2<<endl;
	
	winX = sbox.corner_win[0];
	winY = sbox.corner_win[3];
	QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[1]);
	//qDebug()<<sbox.gcorners[1];
	ED = gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global + 3, sbox.corner_global + 4, sbox.corner_global + 5);//The new position of the mouse
	//qDebug()<<sbox.corner_global[3]<<sbox.corner_global[4]<<sbox.corner_global[5];
	//cout<<"ED"<<endl<<ED<<endl;
	
	winX = sbox.corner_win[2];
	winY = sbox.corner_win[3];
	QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[2]);
	//qDebug()<<sbox.gcorners[2];
	DF = gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global + 6, sbox.corner_global + 7, sbox.corner_global + 8);//The new position of the mouse
	//qDebug() << sbox.corner_global[6] << sbox.corner_global[7]<< sbox.corner_global[8];
	cout<<"DF"<<endl<<DF<<endl;
	winX = sbox.corner_win[2];
	winY = sbox.corner_win[1];
	QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[3]);
	//qDebug()<<sbox.gcorners[3];
	EF = gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global + 9, sbox.corner_global + 10, sbox.corner_global + 11);//The new position of the mouse
	//qDebug() << sbox.corner_global[9] << sbox.corner_global[10]<< sbox.corner_global[11];
	// cout<<"EF"<<endl<<EF<<endl;
}
*/
void MeshViewerLegacy::mousePressEvent(QMouseEvent *e)
{
	mouseState.isPressed = true;

	/// set interaction mode as camera rotation if alt key is hold
	if (e->modifiers() & Qt::AltModifier) {
		interactionStateStack.push(interactionState);
		interactionState = Camera;

		/// set interaction mode as camera translation if shift key is hold
	} else if (e->modifiers() & Qt::ShiftModifier) {
		interactionStateStack.push(interactionState);
		interactionState = Camera_Translation;

		/// set interaction mode as camera zoom if ctrl key is hold
	} else if (e->modifiers() & Qt::ControlModifier) {
		interactionStateStack.push(interactionState);
		interactionState = Camera_Zoom;
	}

	switch (interactionState) {
	case Camera:
	case Camera_Translation:
	case Camera_Zoom:
		mouseState.prev_pos = QVector2D(e->pos());
		break;
	case SEL_FACE:
	case SelectEdge:
	case SelectVertex: {
		sbox.corner_win[0] = e->x();
		sbox.corner_win[1] = view_cam.viewport.h - e->y();
		/*
	cout<<"e->x(win[0])"<<endl<<e->x()<<endl;
	cout<<"e->y(win[1])"<<endl<<e->y()<<endl;
	view_cam.print();
	*/
		break;
	}
	}
}

void MeshViewerLegacy::mouseMoveEvent(QMouseEvent *e)
{
	switch (interactionState) {
	case Camera: {
		if (e->buttons() & Qt::LeftButton) {
			QVector2D diff = QVector2D(e->pos()) - mouseState.prev_pos;

			if ((e->modifiers() & Qt::ShiftModifier)) {      //press "shift" key
				view_cam.translation += QVector3D(diff.x() / 100.0, -diff.y() / 100.0, 0.0);
			}
			else if (e->modifiers() & Qt::ControlModifier)   //press "crl" key
			{
				view_cam.translation += QVector3D(0.0, 0.0, diff.x() / 100.0 - diff.y() / 100.0);
			}

			else{
				// Rotation axis is perpendicular to the mouse position difference
				// vector
				QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
				// Accelerate angular speed relative to the length of the mouse sweep
				qreal acc = diff.length() / 4.0;
				// Calculate new rotation axis as weighted sum
				view_cam.rotationAxis = (view_cam.rotationAxis * view_cam.angularChange + n * acc).normalized();
				// Change rotation angle
				view_cam.angularChange = acc;

				view_cam.rotation = QQuaternion::fromAxisAndAngle(view_cam.rotationAxis, view_cam.angularChange) * view_cam.rotation;
			}
			view_cam.updateModelView();
			mouseState.prev_pos = QVector2D(e->pos());

		}
		updateGL();
		break;
	}

	case Camera_Translation:
		if (e->buttons() & Qt::LeftButton) {
			QVector2D diff = QVector2D(e->pos()) - mouseState.prev_pos;
			view_cam.translation += QVector3D(diff.x() / 100.0, -diff.y() / 100.0, 0.0);
			view_cam.updateModelView();
			mouseState.prev_pos = QVector2D(e->pos());
		}
		updateGL();
		break;
	case Camera_Zoom:
		if (e->buttons() & Qt::LeftButton) {
			QVector2D diff = QVector2D(e->pos()) - mouseState.prev_pos;
			view_cam.translation += QVector3D(0.0, 0.0, diff.x() / 100.0 - diff.y() / 100.0);
			view_cam.updateModelView();
			mouseState.prev_pos = QVector2D(e->pos());
		}
		updateGL();
		break;
		//selection box
	case SEL_FACE:
	case SelectEdge:
	case SelectVertex: {
		if (mouseState.isPressed) {
			isSelecting = true;
			sbox.corner_win[2] = e->x();
			sbox.corner_win[3] = view_cam.viewport.h - e->y();

			//kkkkkkkkkkkkkkkk
			//computeGlobalSelectionBox();
		}
		else {
			isSelecting = false;
			mouseState.isPressed = false;//later added
		}
		//  cout<<"moving mousestate"<<mouseState.isPressed<<endl;
		updateGL();
		break;
	}
	}
}

void MeshViewerLegacy::mouseReleaseEvent(QMouseEvent *e)
{
	switch (interactionState) {
	case Camera:
	case Camera_Translation:
	case Camera_Zoom:

		mouseState.prev_pos = QVector2D(e->pos());
		break;

		//selection box
	case SEL_FACE:
	case SelectEdge:
	case SelectVertex: {
		sbox.corner_win[2] = e->x();
		sbox.corner_win[3] = view_cam.viewport.h - e->y();
		
		//kkkkkkkkkkkkkkk
		//computeGlobalSelectionBox();
		isSelecting = false;
		mouseState.isPressed = false;   //later added
		cout<<"releasing mousestate"<<mouseState.isPressed<<endl;
		int selectedElementIdx = getSelectedElementIndex(e->pos());//mouse positon;
		cout << "selected element " << selectedElementIdx << endl;
		if (selectedElementIdx >= 0) {
			//for testing
			if (interactionState == SelectEdge){
			cout<< "select edge's face index: "<<heMesh->heMap[selectedElementIdx]->f->index<<endl;
			cout<< "select edge flip's face index: "<<heMesh->heMap[selectedElementIdx]->flip->f->index<<endl;
			}
			if (interactionState == SEL_FACE) {
				cout<<"select face is cut face? "<<heMesh->faceMap[selectedElementIdx]->isCutFace<<endl;
				cout<<"select face is bridge? "<<heMesh->faceMap[selectedElementIdx]->isBridger<<endl;

			}
			selectedElementsIdxQueue.push(selectedElementIdx);

			switch (selectionMode) {
			case SingleSelect:
				if (selectedElementsIdxQueue.size() > 1) {
					if (selectedElementsIdxQueue.front() != selectedElementsIdxQueue.back()) {
						if (interactionState == SelectEdge) {
							heMesh->heMap[selectedElementsIdxQueue.front()]->setPicked(false);//deselect
						} else if (interactionState == SEL_FACE) {
							heMesh->faceMap[selectedElementsIdxQueue.front()]->setPicked(false);//deselect
						} else if (interactionState == SelectVertex){
							heMesh->vertMap[selectedElementsIdxQueue.front()]->setPicked(false);//deselect
						}
					}
					selectedElementsIdxQueue.pop();

				}

			case MultiSelect:
				if (interactionState == SelectEdge) {
					heMesh->selectEdge(selectedElementIdx);
				}
				else if (interactionState == SEL_FACE) {
					heMesh->selectFace(selectedElementIdx);
				}
				else if (interactionState == SelectVertex){
					heMesh->selectVertex(selectedElementIdx);
					if (isCriticalPointModeSet)
						findReebPoints();
					if  (isCutLocusModeset)
						findCutLocusPoints();
				}
				break;

			}
		}
		else
			return;


		break;
	}
	}
	mouseState.isPressed = false;
	//cout<<"releasing mousestate"<<mouseState.isPressed<<endl; //later added;
	/// reset interaction mode if in camera mode triggered by holding alt
	if (e->modifiers() & Qt::AltModifier) {
		interactionState = interactionStateStack.top();
		interactionStateStack.pop();
	}

	updateGL();
}

void MeshViewerLegacy::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
	case Qt::Key_C:
	{
		int lev0 = cp_smoothing_times / 10;
		int lev1 = lev0 + 1;
		const int maxLev = 10;
		if (lev1 > 10) return;

		double ratio = (cp_smoothing_times - lev0) / (double)(lev1 - lev0);
		//emit updateMeshColorByGeoDistance(lastSelectedIndex, lev0, lev1, ratio);
		emit updateMeshColorByGeoDistance(lastSelectedIndex); //changed to non-smoothing method due to bug in smoothed method

		break;
	}
		//    case Qt::Key_E:
		//    {
		//        if (heMesh) {
		//            heMesh->flipShowEdges();
		//        }
		//        break;
		//    }
		//    case Qt::Key_V:
		//    {
		//        if (heMesh) {
		//            heMesh->flipShowVertices();
		//        }
		//        break;
		//    }
		//    case Qt::Key_F:
		//    {
		//        if (heMesh) {
		//            heMesh->flipShowFaces();
		//        }
		//        break;
		//    }
		//    case Qt::Key_L:
		//    {
		//        enableLighting = !enableLighting;
		//        break;
		//    }
		//    case Qt::Key_R:
		//    {
		//        toggleCriticalPoints();
		//        break;
		//    }
	case Qt::Key_M:
	{
		//loop through all critical point modes
		cmode = CriticalPointMode((cmode + 1) % NCModes);
		findReebPoints();
		break;
	}
	case Qt::Key_Down:
	{
		double numSteps = .20f;
		view_cam.translation.setY(view_cam.translation.y() - numSteps);
		view_cam.updateModelView();
		break;
	}
	case  Qt::Key_Up:
	{
		double numSteps =  .20f;
		view_cam.translation.setY(view_cam.translation.y() + numSteps);
		view_cam.updateModelView();
		break;
	}
	case Qt::Key_Left:
	{
		double numSteps = .20f;
		view_cam.translation.setX(view_cam.translation.x() - numSteps);
		view_cam.updateModelView();
		break;
	}
	case Qt::Key_Right:
	{
		double numSteps = .20f;
		view_cam.translation.setX(view_cam.translation.x() + numSteps);
		view_cam.updateModelView();
		break;
	}

	}
	updateGL();
}

void MeshViewerLegacy::keyReleaseEvent(QKeyEvent *e)
{

}

void MeshViewerLegacy::wheelEvent(QWheelEvent *e)
{

	switch (interactionState) {
	case Camera:{
		double numSteps = e->delta() / 200.0f;
		view_cam.translation.setZ(view_cam.translation.z() + numSteps);
		view_cam.updateModelView();
		break;
	}
	default:
		break;
	}
	updateGL();
	/*
	int numDegrees = e->delta();
		int num = numDegrees / 120;
		if(num < 0)
		{
			num = 0 - num;
		}

		if (e->orientation() == Qt::Horizontal) {

			//scrollHorizontally(numSteps);
			scale *= 1.25;
//          resize(this->scale *this->size());
		} else {
			//scrollVertically(numSteps);
			if(numDegrees > 0)
			{
				//scale *= 0.75;
			resize(num * 0.95 *this->size());
			}
			else if(numDegrees < 0)
			{
				//scale *= 1.25;
	//          move(100,200);
			}
		}*/
	e->accept();

}

void MeshViewerLegacy::enterEvent(QEvent *e)
{
	QGLWidget::enterEvent(e);
	grabKeyboard();
	setFocus();
}

void MeshViewerLegacy::leaveEvent(QEvent *e)
{
	QGLWidget::leaveEvent(e);
	releaseKeyboard();
}

void MeshViewerLegacy::initializeGL()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	//glEnable (GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);

	setMouseTracking(true);

	initializeFBO();
}

void MeshViewerLegacy::initializeFBO() {
	fbo.reset(new QGLFramebufferObject(width(), height(), QGLFramebufferObject::Depth));
	selectionBuffer.resize(width()*height() * 4);
}

void MeshViewerLegacy::resizeGL(int w, int h)
{
	initializeFBO();

	view_cam.updateViewport(w, h);
	view_cam.updateProjection();

	glViewport(view_cam.viewport.x, view_cam.viewport.y, view_cam.viewport.w, view_cam.viewport.h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(view_cam.projection.constData());
}

void MeshViewerLegacy::paintGL()
{

	glClearColor(1., 1., 1., 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// the model view matrix is updated somewhere else
	glMultMatrixf(view_cam.modelview.constData());

	if (lightingState == Smooth) {
		glEnable(GL_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);

		glShadeModel(GL_SMOOTH);
	}

	if (lightingState != Wireframe) {
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
	}else
		glDisable(GL_DEPTH_TEST);

	if (lightingState == Wireframe) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}else
		glDisable(GL_BLEND);

	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	if (heMesh == nullptr) {
		glLineWidth(2.0);
		GLUtils::drawQuad(QVector3D(-1, -1, 0),
						  QVector3D(1, -1, 0),
						  QVector3D(1, 1, 0),
						  QVector3D(-1, 1, 0));
	}
	else {
		if (lightingState != Wireframe)
			enableLights();
		heMesh->draw(colormap);
		if (lightingState != Wireframe)
			disableLights();

		switch (interactionState) {
		case Camera:
			break;
		default:
			if (mouseState.isPressed)
			{
				drawSelectionBox();
			}
			drawMeshToFBO();
		}

		if (showReebPoints) {
			drawReebPoints();
		}

		if (showCut) {
			selectCutLocusEdges();
		}
		glColor4f(0.0,0.0,0.0,0.5);
		QFont fnt;
		fnt.setPointSize(8);
		for(auto vit=heMesh->vertSet.begin();vit!=heMesh->vertSet.end();vit++)
		{
			HDS_Vertex * v = (*vit);
			QString *name;
			if (showText)
			{
				if (showVIndex)
				{
					this->renderText(v->x(),v->y(),v->z(),name->number(v->index),fnt);
				}
				else if (showCLDistance)
				{
					if (v->index >= CLdistances.size())
					{
						cout << "v index:" << v->index << " while CLdistances len: " << CLdistances.size() << endl;
						continue;
					}
					this->renderText(v->x(),v->y(),v->z(),name->number(CLdistances[v->index]),fnt);
				}
				else if (showCPDistance)
				{
					if (v->index >= CLdistances.size())
					{
						cout << "v index:" << v->index << " while CLdistances len: " << CPdistances.size() << endl;
						continue;
					}
					this->renderText(v->x(),v->y(),v->z(),name->number(CPdistances[v->index]),fnt);
				}
			}
		}
	}
}

static QImage toQImage(const unsigned char* data, int w, int h) {
	QImage qimg(w, h, QImage::Format_ARGB32);
	for (int i = 0, idx = 0; i < h; i++) {
		for (int j = 0; j < w; j++, idx += 4)
		{
			unsigned char r = data[idx + 2];
			unsigned char g = data[idx + 1];
			unsigned char b = data[idx];
			unsigned char a = 255;
			QRgb qp = qRgba(r, g, b, a);
			qimg.setPixel(j, i, qp);
		}
	}
	return qimg;
}

void MeshViewerLegacy::drawMeshToFBO() {
	fbo->bind();

#if 0
	cout << (fbo->isBound()?"bounded.":"not bounded.") << endl;
	cout << (fbo->isValid()?"valid.":"invalid.") << endl;
#endif

	glPushMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// the model view matrix is updated somewhere else
	glMultMatrixf(view_cam.modelview.constData());

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDepthMask(GL_TRUE);

	/// must set alpha to zero
	glClearColor(11, 22, 33, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glShadeModel(GL_FLAT);
	glDisable(GL_BLEND);

	switch (interactionState) {
	case SEL_FACE:
		heMesh->drawFaceIndices();
		break;
	case SelectEdge:
		heMesh->drawEdgeIndices();
		break;
	case SelectVertex:
		heMesh->drawVertexIndices();
		break;
	}

	glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, &(selectionBuffer[0]));
#if 0
	GLenum errcode = glGetError();
	if (errcode != GL_NO_ERROR) {
		const GLubyte *errString = gluErrorString(errcode);
		fprintf (stderr, "OpenGL Error: %s\n", errString);
	}
#endif

	glPopMatrix();

	fbo->release();

	//QImage img = toQImage(&(selectionBuffer[0]), width(), height());
	//img.save("fbo.png");
}

void MeshViewerLegacy::drawSelectionBox() {
	if (!isSelecting) return;

	cout<<"drawingSelectionBox mousestate"<<mouseState.isPressed<<endl;
#if 1
	//draw selection box
	glColor4f(0.0, 1.0, 19.0 / 255, 0.2);
	glBegin(GL_QUADS);
	glVertex3dv(sbox.corner_global);
	glVertex3dv(sbox.corner_global + 3);
	glVertex3dv(sbox.corner_global + 6);
	glVertex3dv(sbox.corner_global + 9);
	glEnd();

	//draw selection box

	glLineWidth(3.0);
	glColor4f(0.0, 1.0, 19.0 / 255, 0.5);
	glBegin(GL_LINE_LOOP);
	glVertex3dv(sbox.corner_global);
	glVertex3dv(sbox.corner_global + 3);
	glVertex3dv(sbox.corner_global + 6);
	glVertex3dv(sbox.corner_global + 9);
	glEnd();


#else
	glPushMatrix();
	glTranslatef(0, 0, 0.5);
	//draw selection box
	GLUtils::fillQuad(sbox.gcorners, QColor(0.0, 1.0, 19.0/255.0, 0.2));

	//draw selection box
	glLineWidth(3.0);
	GLUtils::drawQuad(sbox.gcorners, QColor(0.0, 1.0, 19.0/255.0, 0.5));
	glPopMatrix();
#endif
}

void MeshViewerLegacy::enableLights()
{
	GLfloat light_position[] = { 100.0, 40.0, 100.0, 1.0 };
	GLfloat mat_specular[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat mat_diffuse[] = { 0.375, 0.375, 0.375, 1.0 };
	GLfloat mat_shininess[] = { 25.0 };
	GLfloat light_ambient[] = { 0.9, 0.9, 0.9, 1.0 };
	GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	light_position[0] = -10.0;
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}

void MeshViewerLegacy::disableLights()
{
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHTING);
}

void MeshViewerLegacy::drawReebPoints()
{
	//cout << "nReebPoints = " << reebPoints.size() << endl;
	glPointSize(10.0);
	glBegin(GL_POINTS);
	for (auto p : reebPoints) {
		GLfloat max_mat_diffuse[4] = {0, 1, 0, 1};
		GLfloat min_mat_diffuse[4] = {1, 0, 0.5, 1};
		GLfloat saddle_mat_diffuse[4] = {0, 0, 1, 1};

		switch (p->rtype) {
		case HDS_Vertex::Maximum://green
			glColor4f(0,1.0, 0, 1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, max_mat_diffuse);

			mm+=1;
			break;
		case HDS_Vertex::Minimum: //pink
			glColor4f(1, 0, 0.5, 1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, min_mat_diffuse);

			nm+=1;
			break;
		case HDS_Vertex::Saddle: //blue
			glColor4f(0, 0, 1, 1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, saddle_mat_diffuse);

			nn+=1;
			break;
		}

		GLUtils::useVertex(p->pos);
	}
	glEnd();
	// cout<<"Maximum P : " <<mm<<endl;
	// cout<<"Minimum P : " <<nm<<endl;
	// cout<<"Saddle P : " <<nn<<endl;
	nn=0;
	nm=0;
	mm=0;

}

void MeshViewerLegacy::findReebPoints()
{
	auto laplacianSmoother = [&](const doubles_t &val, HDS_Mesh *mesh) {
		const double lambda = 0.25;
		const double sigma = 1.0;
		unordered_map<HDS_Vertex*, double> L(mesh->verts().size());
		doubles_t newval(mesh->verts().size());
		for (auto vi : mesh->verts()) {
			auto neighbors = vi->neighbors();

			double denom = 0.0;
			double numer = 0.0;

			for (auto vj : neighbors) {
				//double wij = 1.0 / (vi->pos.distanceToPoint(vj->pos) + sigma);
				double wij = 1.0 / neighbors.size();
				denom += wij;
				numer += wij * val[vj->index];
			}

			L.insert(make_pair(vi, numer / denom - val[vi->index]));
		}
		for (auto p : L) {
			newval[p.first->index] = val[p.first->index] + lambda * p.second;
		}

		return newval;
	};

	int nverts = heMesh->vertSet.size();
	int nedges = heMesh->heSet.size() / 2;
	int nfaces = heMesh->faceSet.size();
	int genus = (2 - (nverts - nedges + nfaces)) / 2;
	cout << "genus = " << genus << endl;

	// find the seeds of the laplacian smoothing
	// pick a few vertices based on their curvatures

	auto dists = doubles_t();
	int isSmoothingOnActualMeshChecked = cp_smoothing_type;
	/* if(isSmoothingOnActualMeshChecked==NULL)        //later added;
	{
		dists = doubles_t(heMesh->verts().size());
		QVector3D pnormal = heMesh->vertMap[lastSelectedIndex]->normal;
		for (auto v : heMesh->verts()) {
		dists[v->index] = QVector3D::dotProduct(v->pos, pnormal);
		}
	}
*/                                            //later added;

	switch (isSmoothingOnActualMeshChecked) {
	case Qt::Checked: //bugs with this checked operation
	{
		// get the function values from the smoothed meshes
		int lev0 = cp_smoothing_times / 10;
		int lev1 = lev0 + 1;
		const int maxLev = 10;
		if (lev1 > 10) return;

		double ratio = (cp_smoothing_times-lev0)/10.0;
		switch (cmode) {
		case Geodesics: {
			dists = MeshManager::getInstance()->getInterpolatedGeodesics(lastSelectedIndex, lev0, lev1, ratio);
			break;
		}
		case Z: {
			dists = MeshManager::getInstance()->getInterpolatedZValue(lev0, lev1, ratio);
			break;
		}
		case PointNormal: {
			QVector3D pnormal = heMesh->vertMap[lastSelectedIndex]->normal;
			dists = MeshManager::getInstance()->getInterpolatedPointNormalValue(lev0, lev1, ratio, pnormal);
			break;
		}
		case Curvature:
		{
			dists = MeshManager::getInstance()->getInterpolatedCurvature(lev0, lev1, ratio);
			break;
		}
		case Random:
		{
			dists = doubles_t(heMesh->verts().size());
			for (auto &x : dists) {
				x= rand()/ (double)RAND_MAX - 0.5;
			}
			cout<<dists.front()<<"random number binded"<<endl;
			break;
		}
		}
		break;
	}
	case Qt::Unchecked:
	{
		switch (cmode) {
		case Geodesics: {
			// use geodesic distance
			if(heMesh->verts().size()>10){
				dists = MeshManager::getInstance()->gcomp->distanceTo(lastSelectedIndex);

				cout<<"Geodesics method on "<<lastSelectedIndex<<"..."<<endl;
			}
			else
				break;
			break;
		}
		case Z: {
			cout<<"Z value method..."<<endl;
			dists = doubles_t(heMesh->verts().size());
			for (auto v : heMesh->verts()) {
				dists[v->index] = v->pos.z();//here pos.z() can be changed to pos.y(); later
			}
			break;
		}
		case PointNormal: {
			cout<<"Point normal method..."<<endl;

			dists = doubles_t(heMesh->verts().size());
			QVector3D pnormal = heMesh->vertMap[lastSelectedIndex]->normal;//last selected point normal dotproduct other points vector;
			for (auto v : heMesh->verts()) {
				dists[v->index] = QVector3D::dotProduct(v->pos, pnormal);
			}
			break;
		}
		case Curvature: {
			cout<<"Curvature method..."<<endl;

			dists = doubles_t(heMesh->verts().size());
			for (auto v : heMesh->verts()) {
				dists[v->index] = v->curvature;
			}
			break;
		}
		case Random:
		{
			cout<<"Random method..."<<endl;

			/*   dists = doubles_t(heMesh->verts().size());
		for (auto &x : dists) {
			x= rand()/ (double)RAND_MAX - 0.5;
		}
	*/
			//doubles_t y(heMesh->verts().size());
			int ty=0;
			int tt=0;
			int ts=0;
			int ta=heMesh->verts().size();
			int tq;
			tq=ta;
			int id=0;
			doubles_t memo(ta);
			for(int i=0;i<ta;i++){
				memo[i]=(double)i;

			}


			dists = doubles_t(heMesh->verts().size());
			for (auto &x : dists) {




				id = rand()%tq;
				x= memo[id];
				for(int j=id;j<tq;j++){
					memo[j]=memo[j+1];
					//     cout<<"random number = "<<j<<endl;
				}
				memo.pop_back();
				tq-=1;
				cout<<"dists["<<tt<<"] = "<<x<<endl;


				tt+=1;

				cout<<"--------------------------"<<endl;


				//   x = rand()/ (double)RAND_MAX -0.5;
				//     cout<<"random number = "<<x<<endl;
				//       srand((unsigned)time(0));
				/*      if(tt<=heMesh->verts().size())
			{
			x=rand()%(heMesh->verts().size());
			cout<<"dists["<<tt<<"] = "<<x<<endl;
			y[tt]=x;
			ts+=1;
			if(tt>=1){

			for(int ii=0;ii<tt;ii++){
				if(y[ii]==y[tt])
					ty+=1;
		//       cout<<"ty1 = "<<ty<<endl;
				if(ty==1){
					y.erase(y.begin()+tt-1,y.end());
					x=rand()%(heMesh->verts().size());
					cout<<"dists["<<tt<<"] = "<<x<<endl;
					y[tt]=x;
					ts+=1;
				ty=0;
				for(int ij=0;ij<ii;ij++){
					if(y[ij]==y[tt])
						ty+=1;
					if(ty==1){
						y.erase(y.begin()+tt-1,y.end());
						x=rand()%(heMesh->verts().size());
						cout<<"dists["<<tt<<"] = "<<x<<endl;
						y[tt]=x;
						ts+=1;
						ty=0;
						for(int jj=0;jj<ii;jj++){
						if(y[jj]==y[tt])
							ty+=1;
						if(ty==1){
							y.erase(y.begin()+tt-1,y.end());
							x=rand()%(heMesh->verts().size());
							cout<<"dists["<<tt<<"] = "<<x<<endl;
							y[tt]=x;
							ts+=1;
						ty=0;
			}


			}
			tt+=1;
			ty=0;
			cout<<"----------------------------"<<endl;
	//         cout<<"tt = "<<tt<<endl;
		}
			else
			break;*/
			}
			memo.clear();

			cout<<"sum of vertexes = "<<heMesh->verts().size()<<endl;
			cout<<"cout times = "<<ts<<endl;
			cout<<dists.front()<<"random number binded"<<endl;
			cout<<"dists[7] = "<<dists[7]<<endl;
			cout<<"heMesh->verts().size() = "<<heMesh->verts().size()<<" and heMesh->faces().size() = "<<heMesh->faces().size()<<" and halfedges().size() = "<<heMesh->halfedges().size()<<endl;
			break;
		}
		case Quadratic:
		{
			cout<<"Quadratic method..."<<endl;

			// compute the center of the shape
			QVector3D c(0, 0, 0);
			for (auto &v : heMesh->vertSet) {
				c += v->pos;
			}
			c /= heMesh->vertSet.size();
			// compute per-vertex distance
			dists = doubles_t(heMesh->verts().size());
			for (auto &v : heMesh->vertSet) {
				dists[v->index] = heMesh->vertMap[0]->pos.distanceToPoint(v->pos);
			}
			break;
		}

		}


		// find the points to keep
		int niters = cp_smoothing_times;// pow(2, cp_smoothing_times);
		cout << "smoothing = " << niters << endl;
		for (int i = 0; i < niters; ++i)
			dists = laplacianSmoother(dists, heMesh);

		break;
	}
	}

#if USE_REEB_GRAPH
	MeshManager::getInstance()->updateReebGraph(dists);
#endif
	reebPoints = heMesh->getReebPoints(dists);



	int sum_cp = 0;
	for (auto cp : reebPoints) {
		if (cp->rtype == HDS_Vertex::Maximum) sum_cp += 1;
		else if (cp->rtype == HDS_Vertex::Minimum) sum_cp += 1;
		else sum_cp -= cp->sdegree;
	}
	cout << "sum = " << sum_cp << endl;

	// generate morse-smale complex
	msc = MorseSmaleComplex(reebPoints);
	CPdistances = dists;
}

void MeshViewerLegacy::findCutLocusPoints()
{
	auto dists = doubles_t();

	if(heMesh->verts().size()>10){
		switch (lmode) {
		case GeodesicsDist: {
			// use geodesic distance
			dists = MeshManager::getInstance()->gcomp->distanceTo(lastSelectedIndex);

			cout<<"Geodesics method on "<<lastSelectedIndex<<"..."<<endl;

			break;
		}
		case GraphDist:
			cout<<"Graph Distance method on vertex "<<lastSelectedIndex<<"..."<<endl;

			if (heMesh->getSelectedVertices().empty()) {
				heMesh->selectVertex(0);
				selectedElementsIdxQueue.push(0);
			}
			dists = MeshManager::getInstance()->dis_gcomp->discreteDistanceTo(heMesh->getSelectedVertices());
			cout << "Graph distance calculated."<<endl;

			break;
		}

		//same as findReebPoints

#if USE_REEB_GRAPH
		MeshManager::getInstance()->updateReebGraph(dists);
#endif
		//get min max saddle points
		reebPoints = heMesh->getReebPoints(dists);
		// generate morse-smale complex
		msc = MorseSmaleComplex(reebPoints);
	}
	else
		cout<<"Can't do cut locus on this obj, too few vertices."<<endl;
	CLdistances = dists;
}

void MeshViewerLegacy::selectCutLocusEdges()
{

	resetEdges();
#if 0
	for (auto i = 0; i < rbgraph->E.size(); ++i) {
		int s = rbgraph->E[i].s;
		int t = rbgraph->E[i].t;
		int v0 = rbgraph->V[s].idxref;
		int v1 = rbgraph->V[t].idxref;

		GLUtils::drawLine(heMesh->vertMap[v0]->pos, heMesh->vertMap[v1]->pos);
	}
#else
#if 0
	for (auto i = 0; i < rbgraph->Es.size(); ++i) {
		int v0 = rbgraph->Es[i].s;
		int v1 = rbgraph->Es[i].t;

		GLUtils::drawLine(heMesh->vertMap[v0]->pos, heMesh->vertMap[v1]->pos, Qt::red);
	}
#else
	bool hasSaddle = false;
	for (auto cp : reebPoints) {
		if (cp->rtype == HDS_Vertex::Saddle)
			hasSaddle = true;
	}

	if (hasSaddle){
		//cout<<"drawing path..."<<endl;
		auto maxpaths = msc.getMaxPaths();
		auto minpaths = msc.getMinPaths();
		auto paths = maxpaths;
		if (showMultCut)
			paths.insert( paths.end(), minpaths.begin(), minpaths.end() );
		for (auto p : paths) {
			//auto p = paths.front();
			for (auto i = 0; i < p.edges.size(); ++i) {
				auto v0 = p.edges[i].s;
				auto v1 = p.edges[i].t;
				//cout<<" path edge: "<<v0->pos<<" "<<v1->pos<<endl;
				//GLUtils::drawLine(v0->pos, v1->pos, Qt::green);
				HDS_HalfEdge *curHE = v0->he;
				do {
					if (curHE->flip->v == v1) {
						curHE->setPicked(true);
						break;
					}
					curHE = curHE->flip->next;

				} while( curHE != v0->he );

			}
		}
	}else {
		cout<<"Has no saddle, no cut locus defined."<<endl;
	}
#endif
#endif
}

void MeshViewerLegacy::toggleLightingSmooth()
{
	lightingState = Smooth;
}

void MeshViewerLegacy::toggleLightingFlat()
{
	lightingState = Flat;
}

void MeshViewerLegacy::toggleLightingWireframe()
{
	lightingState = Wireframe;
}

void MeshViewerLegacy::toggleText()
{
	showText = !showText;
}


void MeshViewerLegacy::toggleCriticalPoints() {

	showReebPoints = !showReebPoints;
	updateGL();

}

void MeshViewerLegacy::toggleCutLocusPoints(int state) {
	if(state == Qt::Unchecked) {
		showReebPoints = false;
	}else {
		showReebPoints = true;
	}
	updateGL();
}

void MeshViewerLegacy::toggleCutLocusCut() {
	showCut = !showCut;
	resetEdges();
	if (showCut)
		selectCutLocusEdges();

	updateGL();
}

void MeshViewerLegacy::toggleCutLocusCutMode() {
	showMultCut = !showMultCut;
	selectCutLocusEdges();
	updateGL();
}


void MeshViewerLegacy::showCriticalPoints() {
	showReebPoints = true;
	showCPDistance = true;
	showVIndex = false;

}

void MeshViewerLegacy::showCutLocusPoints() {
	showReebPoints = true;
	showCLDistance = true;
	showVIndex = false;

}

void MeshViewerLegacy::showCutLocusCut() {
	showCut = true;
	selectCutLocusEdges();

	updateGL();
}

void MeshViewerLegacy::hideCriticalPoints() {
	showReebPoints = false;

}

void MeshViewerLegacy::setCriticalPointsMethod(int midx)
{
	cmode = (CriticalPointMode)midx;
	isCriticalPointModeSet = true;
	cout<<"setCriticalPointsMethod midx = "<<midx<<endl;
	cout<<"setCriticalPointsMethod cmode = "<<cmode<<endl;
	findReebPoints();
}

void MeshViewerLegacy::setCriticalPointsSmoothingTimes(int times)
{
	cp_smoothing_times = times;
	findReebPoints();
}

void MeshViewerLegacy::setCriticalPointsSmoothingType(int t)
{
	cp_smoothing_type = t;
	cout<<"setCriticalPointssmoothingtype cp_smoothing_type"<<cp_smoothing_type<<endl;
	findReebPoints();
}

void MeshViewerLegacy::setCutLocusMethod(int midx)
{
	lmode = (CutLocusMode)midx;
	isCutLocusModeset = true;
	cout<<"setCutLocusMethod midx = "<<midx<<endl;
	cout<<"setCutLocusMethod lmode = "<<lmode<<endl;
	findCutLocusPoints();

}



void MeshViewerLegacy::bindReebGraph(SimpleGraph *g)
{
	rbgraph = g;
}

//////////////////////////////////////////////////////////////////////////
