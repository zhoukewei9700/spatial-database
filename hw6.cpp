// hw6.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "common.h"
#include "Geometry.h"
#include "shapelib/shapefil.h"
#include "RTree_2.h"

#define NDEBUG
#include <GL/freeglut.h>       // Glut��ͷ�ļ�

#include <iostream>
#include <vector>
#include <cstdio> 
#include <ctime>
#include <map>
#include <list>
#include<set>
using namespace std;

extern void test(int t);
extern void QuadTreeAnalysis();

int screenWidth  = 640;
int screenHeight = 480;

double pointSize = 2.0;

int mode;
int searchmode;

vector<hw6::Feature> features;
vector<hw6::Feature> roads;
bool showRoad = true;

hw6::QuadTree pointQTree;
hw6::QuadTree roadQTree;
hw6::RTree rtree;
hw6::RTree road_rtree;
bool showQuadTree = false;
bool showRTree = false;
hw6::Feature nearestFeature;

bool firstPoint = true;
hw6::Point corner[2];
hw6::Envelope selectedRect;
vector<hw6::Feature> selectedFeatures;

/*
 * shapefile�ļ���name��geometry���Զ�ȡ
 */
vector<string> readName(const char* filename)
{
	DBFHandle file = DBFOpen(filename, "r");

	vector<string> res;
	int cct = DBFGetRecordCount(file);
	res.reserve(cct);
	for (int i = 0; i < cct; ++i){
		string a = DBFReadStringAttribute(file, i, 0);
		res.push_back(a);
	}

	DBFClose(file);

	return res;
}

vector<hw6::Geometry *> readGeom(const char *filename)
{
	SHPHandle file = SHPOpen(filename, "r");

	int pnEntities, pnShapeType;
	double padfMinBound[4], padfMaxBound[4];
	SHPGetInfo(file, &pnEntities, &pnShapeType, padfMinBound, padfMaxBound);
	
	vector<hw6::Point> points;
	vector<hw6::Geometry *> geoms;
	geoms.reserve(pnEntities);
	switch (pnShapeType){
	case SHPT_POINT:
		for (int i = 0; i < pnEntities; ++i) {
			SHPObject *pt = SHPReadObject(file, i);
			geoms.push_back(new hw6::Point(pt->padfY[0], pt->padfX[0]));
			SHPDestroyObject(pt);
		}
		break;

	case SHPT_ARC:
		for (int i = 0; i < pnEntities; ++i) {
			points.clear();
			SHPObject *pt = SHPReadObject(file, i);
			for (int j = 0; j < pt->nVertices; ++j){
				points.push_back(hw6::Point(pt->padfY[j], pt->padfX[j]));
			}
			SHPDestroyObject(pt);
			geoms.push_back(new hw6::LineString(points));
		}
		break;

	case SHPT_POLYGON:
		for (int i = 0; i < pnEntities; ++i) {
			points.clear();
			SHPObject *pt = SHPReadObject(file, i);
			for (int j = 0; j < pt->nVertices; ++j){
				points.push_back(hw6::Point(pt->padfY[j], pt->padfX[j]));
			}
			SHPDestroyObject(pt);
			hw6::LineString line(points);
			hw6::Polygon *poly = new hw6::Polygon(line);
			geoms.push_back(new hw6::Polygon(line));
		}
		break;
	}
	
	SHPClose(file);
	return geoms;
}

/*
 * ���������Ϣ
 */
void printGeom(vector<hw6::Geometry *>& geom)
{
	cout << "Geometry:" << endl;
	for (vector<hw6::Geometry *>::iterator it = geom.begin(); it != geom.end(); ++it) {
		(*it)->print();
	}
}

/*
 * ɾ��������Ϣ
 */
void deleteGeom(vector<hw6::Geometry *>& geom)
{
	for (vector<hw6::Geometry *>::iterator it = geom.begin(); it != geom.end(); ++it) {
		delete *it;
		*it = NULL;
	}
	geom.clear();
}

/*
 * ��ȡŦԼ��·����
 */
void loadRoadData()
{
	vector<hw6::Geometry *> geom = readGeom(".//data/highway");

	roads.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		roads.push_back(hw6::Feature(to_string(i), geom[i]));

	cout << "road number: " << geom.size() << endl;
	roadQTree.setCapacity(20);
	roadQTree.constructQuadTree(roads);
	vector<hw6::Feature*>f;
	for (size_t i = 0; i < roads.size(); i++)f.push_back(&roads[i]);
	road_rtree.Construction(f, 64);
}


/*
 * ��ȡŦԼ���г����޵�����
 */
void loadStationData()
{
	vector<hw6::Geometry *> geom = readGeom(".//data/station");
	vector<string> name = readName(".//data/station");

	features.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		features.push_back(hw6::Feature(name[i], geom[i]));

	cout << "station number: " << geom.size() << endl;
	pointQTree.setCapacity(5);
	pointQTree.constructQuadTree(features);
	vector<hw6::Feature*>f;
	for (size_t i = 0; i < features.size(); i++)f.push_back(&features[i]);
	rtree.Construction(f, 8);
}

/*
 * ��ȡŦԼ���⳵�򳵵�����
 */
void loadTaxiData()
{
	vector<hw6::Geometry *> geom = readGeom(".//data/taxi");
	vector<string> name = readName(".//data/taxi");

	features.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		features.push_back(hw6::Feature(name[i], geom[i]));

	cout << "taxi number: " << geom.size() << endl;
	pointQTree.setCapacity(100);
	pointQTree.constructQuadTree(features);
	vector<hw6::Feature*>f;
	for (size_t i = 0; i < features.size(); i++)f.push_back(&features[i]);
	rtree.Construction(f, 256);

}

/*
 * �����ѯ
 */
void rangeQuery()
{
	selectedFeatures.clear();
	vector<hw6::Feature> candidateFeatures;

	// filter step (ʹ���Ĳ�����ò�ѯ����ͼ���������Χ���ཻ�ĺ�ѡ����
	if (mode == RANGEPOINT) {

		if (searchmode == QUADQUERY)pointQTree.rangeQuery(selectedRect, candidateFeatures);
		else if (searchmode == RTREEQUERY)rtree.RangeQuery(selectedRect, candidateFeatures);
	}
	else if (mode == RANGELINE)
		if (searchmode == QUADQUERY)roadQTree.rangeQuery(selectedRect, candidateFeatures);
		else if (searchmode == RTREEQUERY)road_rtree.RangeQuery(selectedRect, candidateFeatures);
		

	// refine step (��ȷ�ж�ʱ����Ҫȥ�أ������ѯ����ͼ��ζ�����ظ�����)
	// write your here to update selectedFeatures
	
		vector<hw6::Feature>feature1;
		set<hw6::Feature>featureMap;
		for (hw6::Feature f : candidateFeatures) {
			if (featureMap.find(f) == featureMap.end()) {
				if (f.getGeom()->intersects(selectedRect))
					feature1.push_back(f);
				featureMap.insert(f);
			}
		}
		selectedFeatures = feature1;
	
	//else selectedFeatures = candidateFeatures;
}


/*
 * �ڽ���ѯ
 */
void NNQuery(hw6::Point p)
{
	vector<hw6::Feature> candidateFeatures;

	// filter step (ʹ���Ĳ�����þ���Ͻ��ļ���������ѡ��)
	if (mode == NNPOINT) {
		if (searchmode == QUADQUERY) pointQTree.NNQuery(p.getX(), p.getY(), candidateFeatures);
		else if (searchmode == RTREEQUERY) rtree.NNQuery(p.getX(), p.getY(), candidateFeatures);
	}
	else if (mode == NNLINE) {
		if (searchmode == QUADQUERY) roadQTree.NNQuery(p.getX(), p.getY(), candidateFeatures);
		else if (searchmode == RTREEQUERY) road_rtree.NNQuery(p.getX(), p.getY(), candidateFeatures);
	}

	// refine step (��ȷ�����ѯ���뼸�ζ���ľ���)
	// write your here to update nearestFeature
	if (!candidateFeatures.empty()) {
		size_t nearestFeatureIndex = 0;
		double minDist = INFINITY;
		for (size_t j = 0; j < candidateFeatures.size(); ++j) {
			if (candidateFeatures[j].distance(p.getX(), p.getY()) < minDist) {
				minDist = candidateFeatures[j].distance(p.getX(), p.getY());
				nearestFeatureIndex = j;
			}
		}
		nearestFeature = candidateFeatures[nearestFeatureIndex];
	}
		
}

/*
 * ����Ļ����ת������������
 */
void transfromPt(hw6::Point &pt)
{
	const hw6::Envelope bbox = pointQTree.getEnvelope();
;
	double width = bbox.getMaxX() - bbox.getMinX() + 0.002;
	double height = bbox.getMaxY() - bbox.getMinY() + 0.002;

	double x = pt.getX() * width / screenWidth + bbox.getMinX() - 0.001;
	double y = pt.getY() * height / screenHeight + bbox.getMinY() - 0.001;
	
	x = max(bbox.getMinX(), x);
	x = min(bbox.getMaxX(), x);
	y = max(bbox.getMinY(), y);
	y = min(bbox.getMaxY(), y);
	pt = hw6::Point(x, y);
}

/*
 * ���ƴ���
 */
void display()
{
	//glClearColor(241 / 255.0, 238 / 255.0, 232 / 255.0, 0.0); 
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	const hw6::Envelope bbox = pointQTree.getEnvelope();
	gluOrtho2D(bbox.getMinX() - 0.001, bbox.getMaxX() + 0.001, bbox.getMinY() - 0.001, bbox.getMaxY() + 0.001);

	// ��·����
	if (showRoad) {
		glColor3d(252 / 255.0, 214 / 255.0, 164 / 255.0);
		for (size_t i = 0; i < roads.size(); ++i)
			roads[i].draw();
	}
	
	// �����
	if (!(mode == RANGELINE || mode == NNLINE)) {
		glPointSize((float)pointSize);
		glColor3d(0.0, 146 / 255.0, 247 / 255.0);
		for (size_t i = 0; i < features.size(); ++i)
			features[i].draw();
	}
	
	// �Ĳ�������
	if (showQuadTree) {
		glColor3d(0.0, 146 / 255.0, 247 / 255.0);
		if (mode == RANGELINE || mode == NNLINE)
			roadQTree.draw();
		else
			pointQTree.draw();
	}

	//R������
	if (showRTree) {
		glColor3d(0.0, 146 / 255.0, 247 / 255.0);
		rtree.draw();
	}

	// �������������
	if (mode == NNPOINT) {
		glPointSize(5.0);
		glColor3d(0.9, 0.0, 0.0);
		nearestFeature.draw();
	}

	// ����������·����
	if (mode == NNLINE) {
		glLineWidth(3.0);
		glColor3d(0.9, 0.0, 0.0);
		nearestFeature.draw();
		glLineWidth(1.0);
	}

	// ����ѡ�����
	if (mode == RANGEPOINT || mode == RANGELINE) {
		glColor3d(0.0, 0.0, 0.0);
		selectedRect.draw();
		glColor3d(1.0, 0.0, 0.0);
		for (size_t i = 0; i < selectedFeatures.size(); ++i)
			selectedFeatures[i].draw();
	}

	glFlush();
	glutSwapBuffers();
}

/*
 * ���ͼ��̽���
 */
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (mode == RANGEPOINT || mode == RANGELINE) {
			if (firstPoint) {
				selectedFeatures.clear();
				corner[0] = hw6::Point(x, screenHeight - y);
				transfromPt(corner[0]);
			}
			else {
				corner[1] = hw6::Point(x, screenHeight - y);
				transfromPt(corner[1]);
				selectedRect = hw6::Envelope(min(corner[0].getX(), corner[1].getX()), max(corner[0].getX(), corner[1].getX()),
										     min(corner[0].getY(), corner[1].getY()), max(corner[0].getY(), corner[1].getY()));
				rangeQuery();
			}
			firstPoint = !firstPoint;
			glutPostRedisplay();
		}
	}
}

void passiveMotion(int x, int y)
{
	corner[1] = hw6::Point(x, screenHeight - y);

	if ((mode == RANGEPOINT || mode == RANGELINE) && !firstPoint) {
		corner[1] = hw6::Point(x, screenHeight - y);
		transfromPt(corner[1]);
		selectedRect = hw6::Envelope(min(corner[0].getX(), corner[1].getX()), max(corner[0].getX(), corner[1].getX()),
								     min(corner[0].getY(), corner[1].getY()), max(corner[0].getY(), corner[1].getY()));
		rangeQuery();

		glutPostRedisplay();
	}
	else if (mode == NNPOINT || mode == NNLINE) {
		hw6::Point p(x, screenHeight - y);
		transfromPt(p);
		NNQuery(p);

		glutPostRedisplay();
	}
}

void changeSize(int w, int h)
{
	screenWidth = w;
	screenHeight = h;
	glViewport(0, 0, w, h);
	glutPostRedisplay();
}

void processNormalKeys(unsigned char key, int x, int y)
{
	switch(key) {
		case 27:exit(0); break;
		case 'N':
			mode = NNLINE; 
			searchmode = QUADQUERY; break;
		case 'n':
			mode = NNPOINT; 
			searchmode=QUADQUERY; break;
		case 'E':
			mode = NNLINE;
			searchmode = RTREEQUERY; break;
		case 'e':
			mode = NNPOINT;
			searchmode = RTREEQUERY; break;
		case 'S':
			mode = RANGELINE; searchmode = QUADQUERY;
			firstPoint = true;
			break;
		case 's':
			mode = RANGEPOINT; searchmode = QUADQUERY;
			firstPoint = true; 
			break;
		case 'X':
			mode = RANGELINE; searchmode = RTREEQUERY;
			firstPoint = true;
			break;
		case 'x':
			mode = RANGEPOINT; searchmode = RTREEQUERY;
			firstPoint = true;
			break;
		case 'B':
		case 'b':
			loadStationData();
			mode = Default;
			break;
		case 'T':
		case 't':
			loadTaxiData();
			mode = Default;
			break;
		case 'R':
		case 'r':
			showRoad = !showRoad;
			break;
		case 'Q':
		case 'q':
			showQuadTree = !showQuadTree;
			break;
		case 'U':
		case 'u':
			showRTree = !showRTree;
			break;
		case '+':
			pointSize *= 1.1;
			break;
		case '-':
			pointSize /= 1.1;
			break;
		case '1':
			test(TEST1); break;
		case '2':
			test(TEST2); break;
		case '3':
			test(TEST3); break;
		case '4':
			test(TEST4); break;
		case '5':
			test(TEST5); break;
		case '6':
			test(TEST6); break;
		case '7':
			test(TEST7); break;
		case '8':
			test(TEST8); break;
		case'9':
			test(TEST9); break;
		default: 
			mode = Default; break;
	}
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	cout << "Key Usage:\n"
		 << "  S  : Quadtree range search for roads\n"
		 << "  s  : QuadTree range search for stations\n"
		 << "  X  : R-Tree range search for roads\n"
	 	 << "  x  : R-Tree range search for stations\n"
		 << "  N  : QuadTree nearest road search\n"
		 << "  n  : QuadTree nearest station search\n"
		 << "  E  : R-Tree nearest search for roads\n"
		 << "  e  : R-Tree nearest search for stations\n"
		 << "  B/b: Bicycle data\n"
		 << "  T/t: Taxi data\n"
		 << "  R/r: show Road\n"
		 << "  Q/q: show QuadTree\n"
		 << "  U/u: show R-Tree\n"
		 << "  +  : increase point size\n"
		 << "  -  : decrease point size\n"
		 << "  1  : Test Envelope contain, interset and union\n"
		 << "  2  : Test distance between Point and LineString\n"
		 << "  3  : Test distance between Point and Polygon\n"
		 << "  4  : Test quadtree construction\n"
		 << "  5  : Test R-Tree construction\n"
		 << "  6  : Interior Ring test\n"
		 << "  7  : Rtree performance analysis\n"
		 << "  8  : Quadtree performance analysis\n"
		 << "  9  : QuadTree and R-Tree comparative analysis\n"
		 << "  ESC: quit\n"
		 << endl;

	loadRoadData();

	loadStationData();

	glutInit(&argc, argv);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("New York");

	glutMouseFunc(mouse);
	glutDisplayFunc(display);
	glutPassiveMotionFunc(passiveMotion);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);

	glutMainLoop();

	return 0;
}

