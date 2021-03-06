#-------------------------------------------------
#
# Project created by QtCreator 2014-09-04T16:35:12
#
#-------------------------------------------------

QT       += core gui opengl svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Unfolding
TEMPLATE = app

RC_FILE = icons/_Unfolding.rc

CONFIG += c++11
CONFIG(release): DEFINES += NDEBUG
#CONFIG += console

QMAKE_CXXFLAGS += -std=c++11

Release:DESTDIR = bin/Release
Debug:DESTDIR = bin/Debug
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui

INCLUDEPATH += extras/colormap_editor src

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/hds_common.cpp \
    src/hds_vertex.cpp \
    src/hds_halfedge.cpp \
    src/hds_face.cpp \
    src/hds_mesh.cpp \
    src/meshmanager.cpp \
    src/meshloader.cpp \
    src/stringutils.cpp \
    src/meshcutter.cpp \
    src/meshunfolder.cpp \
    extras/colormap_editor/colormapeditor.cpp \
    src/colormap.cpp \
    src/unionfind.cpp \
    src/meshsmoother.cpp \
    src/criticalpointspanel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/BaseModel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ExactMethodForDGP.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ICHWithFurtherPriorityQueue.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ImprovedCHWithEdgeValve.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/Point3D.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/PreviousCH.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/RichModel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/stdafx.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.cpp \
    src/morsesmalecomplex.cpp \
    src/GeodesicComputer.cpp \
    src/MeshExtender.cpp \
    src/discretegeocomputer.cpp \
    src/cutlocuspanel.cpp \
	src/hds_bridger.cpp \
    src/meshhollower.cpp \
    src/BBox.cpp \
    src/ConnectorPanel.cpp \
    src/MeshDFormer.cpp \
    src/MeshConnector.cpp \
    src/meshrimface.cpp \
    src/OperationStack.cpp \
    src/rimfacepanel.cpp \
    src/Camera.cpp \
    src/MeshViewer.cpp \
	src/ViewerGrid.cpp \
    src/MeshWeaver.cpp \
    src/ViewerRuler.cpp \
    src/WeavePanel.cpp \
    src/QuadEdgePanel.cpp \
    src/BridgerPanel.cpp \
    src/GESPanel.cpp \
    forms/NeoWeavePanel.cpp \
    forms/OrigamiPanel.cpp


HEADERS  += src/mainwindow.h \
	src/UnfoldingAppException.h \
    src/hds_common.h \
    src/hds_vertex.h \
    src/hds_halfedge.h \
    src/hds_face.h \
    src/hds_mesh.h \
    src/common.h \
    src/glutils.hpp \
    src/meshmanager.h \
    src/meshloader.h \
    src/stringutils.h \
    src/mathutils.hpp \
    src/meshcutter.h \
    src/utils.hpp \
    src/meshunfolder.h \
    extras/colormap_editor/colormapeditor.h \
    src/colormap.h \
    src/unionfind.h \
    src/meshsmoother.h \
    src/criticalpointspanel.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/BaseModel.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ExactMethodForDGP.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ICHWithFurtherPriorityQueue.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ImprovedCHWithEdgeValve.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/Point3D.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/PreviousCH.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/RichModel.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/stdafx.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/targetver.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/wxnTime.h \
    src/morsesmalecomplex.h \
    src/GeodesicComputer.h \
    src/MeshExtender.h \
    src/discretegeocomputer.h \
    src/cutlocuspanel.h \
	src/hds_bridger.h \
    src/meshhollower.h \
    src/BBox.h \
    src/ConnectorPanel.h \
    src/MeshDFormer.h \
    src/MeshConnector.h \
    src/meshrimface.h \
    src/OperationStack.h \
    src/rimfacepanel.h \
    src/MeshViewer.h \
    src/Camera.h \
    src/CameraLegacy.h \
    src/ViewerGrid.h \
    src/MeshWeaver.h \
    src/ViewerRuler.h \
    src/WeavePanel.h \
    src/QuadEdgePanel.h \
    src/BridgerPanel.h \
    src/GESPanel.h \
    forms/NeoWeavePanel.h \
    forms/OrigamiPanel.h

FORMS    += extras/colormap_editor/colormapeditor.ui \
    forms/mainwindow.ui \
    forms/criticalpointspanel.ui \
    forms/cutlocuspanel.ui \
    forms/ConnectorPanel.ui \
    forms/rimfacepanel.ui \
    forms/WeavePanel.ui \
    forms/QuadEdgePanel.ui \
    forms/BridgerPanel.ui \
    forms/GESPanel.ui \
    forms/NeoWeavePanel.ui \
    forms/OrigamiPanel.ui

RESOURCES += \
    resources.qrc

#OTHER_FILES +=
#LIBS += -glut32
#LIBS += -Lc:\glut

DISTFILES += \
    shaders/edge_fs.glsl \
    shaders/edge_vs.glsl \
    shaders/face_fs.glsl \
    shaders/face_gs.glsl \
    shaders/face_vs.glsl \
    shaders/vtx_fs.glsl \
	shaders/vtx_vs.glsl \
	shaders/uid_fs.glsl \
	shaders/uid_vs.glsl

#To make it work, add make install in Build Step
debug {
install_shader.path = $${DESTDIR}/shaders
install_shader.files += shaders/*

INSTALLS += install_shader
}