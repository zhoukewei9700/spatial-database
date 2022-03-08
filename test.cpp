#include "common.h"
#include "Geometry.h"
#include "time.h"
#include "RTree_2.h"
using namespace hw6;

extern int mode;
extern vector<Geometry *> readGeom(const char *filename);
extern vector<string> readName(const char* filename);

void transformValue(double &res, const char* format = "%.2lf"){

	char buf[20];
	sprintf(buf, format, res);
	sscanf(buf, "%lf", &res);
}

void wrongMessage(Envelope e1, Envelope e2, bool cal)
{
	cout << "Your answer is " << cal << " for test between ";
	e1.print();
	cout << " and ";
	e2.print();
	cout << ", but the answer is " << !cal << endl;
}

void wrongMessage(const Point& pt1, const Point& pt2, double dis, double res)
{
	cout << "Your answer is " << dis << " for test ";
	pt1.print();
	cout << " and ";
	pt2.print();
	cout << ", but the answer is " << res << endl;
}

void wrongMessage(Envelope e1, Envelope e2, Envelope cal, Envelope res)
{
	cout << "Your answer is ";
	cal.print();
	cout << " for test between ";
	e1.print();
	cout << " and ";
	e2.print();
	cout << ", but the answer is ";
	res.print();
	cout << endl;
}

void QuadTreeAnalysis()
{
	vector<Feature> features;
	vector<Geometry*> geom = readGeom(".//data/taxi");
	vector<string> name = readName(".//data/taxi");

	features.clear();
	features.reserve(geom.size());
	for (size_t i = 0; i < geom.size(); ++i)
		features.push_back(Feature(name[i], geom[i]));

	cout << "taxi number: " << geom.size() << endl;

	srand(time(NULL));
	for (int cap = 70; cap <= 200; cap += 10) {
		QuadTree* qtree = new QuadTree();
		// Task ¹¹ÔìËÄ²æÊ÷£¬Êä³öËÄ²æÊ÷µÄ½ÚµãÊýÄ¿ºÍ¸ß¶È
		// Write your code here
	    clock_t start_time = clock();
		qtree->setCapacity(cap);
		qtree->constructQuadTree(features);
		clock_t end_time = clock();

		int height = 0, interiorNum = 0, leafNum = 0;
		// Write your code here
		qtree->countHeight(height);
		qtree->countQuadNode(interiorNum, leafNum);

		cout << "Capacity " << cap << "\n";
		cout << "Height: " << height << " \tInterior node number: " << interiorNum << " \tLeaf node number: " << leafNum << "\n";
		cout << "Construction time: " << (end_time - start_time) / 1000.0 << "s" << endl;

		double x, y;
		vector<hw6::Feature> candidateFeatures;
		start_time = clock();

		for (int i = 0; i < 10000; ++i) {
			x = -((rand() % 225) / 1000.0 + 73.9812);
			y = (rand() % 239) / 1000.0 + 40.7247;
			candidateFeatures.clear();
			//clock_t s = clock();
			qtree->NNQuery(x, y, candidateFeatures);
			//clock_t e = clock();
			//cout <<i<<" "<< (e - s)<<" ms" << endl;
			// refine step
			// write your code here
			Feature nearestFeature;
			if (!candidateFeatures.empty()) {
				size_t nearestFeatureIndex = 0;
				double minDist = INFINITY;
				for (size_t j = 0; j < candidateFeatures.size(); ++j) {
					if (candidateFeatures[j].distance(x, y) < minDist) {
						minDist = candidateFeatures[j].distance(x, y);
						nearestFeatureIndex = j;
					}
				}
				nearestFeature = candidateFeatures[nearestFeatureIndex];
			}
		}
		end_time = clock();
		cout << "NNQuery time: " << (end_time - start_time) / 1000.0 << "s" << endl << endl;

		delete qtree;
	}
}
void RTreeAnalysis() {
	vector<Feature> features;
	vector<Feature*> f;
	RTree rtree2;
	vector<hw6::Geometry*> geom = readGeom(".//data/taxi");
	cout << "taxi number: " << geom.size() << endl;
	vector<string> name = readName(".//data/taxi");

	srand(time(NULL));
	//features.clear();
	for (size_t i = 0; i < geom.size(); ++i)
		features.push_back(Feature(name[i], geom[i]));
	for (size_t i = 0; i < features.size(); i++) {
		f.push_back(&features[i]);
	}

	
	for (int i = 16; i <= 128; i*=2) {
		clock_t start_time = clock();
		rtree2.Construction(f, i);
		clock_t end_time = clock();
		//rtree.All_area();
		cout << "MaxFill: " << rtree2.MaxFill << endl;
		cout << "Height: "; rtree2.getHeight();
		cout << "Feature Number: " << rtree2.elememtCount() << endl;;
		cout << "Construction time: " << (end_time - start_time) / 1000.0 << "s" << endl;

		double x, y;
		vector<hw6::Feature> candidateFeatures;
		start_time = clock();
		for (int i = 0; i < 1000; i++) {
			x = -((rand() % 225) / 1000.0 + 73.9812);
			y = (rand() % 239) / 1000.0 + 40.7247;
			rtree2.NNQuery(x, y, candidateFeatures);
			Feature nearestFeature;
			if (!candidateFeatures.empty()) {
				size_t nearestFeatureIndex = 0;
				double minDist = INFINITY;
				for (size_t j = 0; j < candidateFeatures.size(); ++j) {
					if (candidateFeatures[j].distance(x, y) < minDist) {
						minDist = candidateFeatures[j].distance(x, y);
						nearestFeatureIndex = j;
					}
				}
				nearestFeature = candidateFeatures[nearestFeatureIndex];
			}
			candidateFeatures.clear();
		}
		end_time = clock();
		cout << "NN-Query time: " << (end_time - start_time) / 1000.0 << "s" << endl;
	}
	//test NN query
	

}

void test(int t)
{
	cout << "*********************Start*********************" << endl;
	if (t == TEST1) {
		cout << "²âÊÔ1: Envelope Contain, Intersect, and Union" << endl;

		int failedCase = 0;
		Envelope e1(-1, 1, -1, 1);
		vector<Envelope> tests;
		tests.push_back(Envelope(-0.5, 0.5, -0.5, 0.5));
		tests.push_back(Envelope(-0.5, 0.5, 0.5, 1.5));
		tests.push_back(Envelope(0.5, 1.5, -0.5, 0.5));
		tests.push_back(Envelope(-1.5, -0.5, -1.5, -0.5));
		tests.push_back(Envelope(-2, -1, -0.5, 0.5));
		tests.push_back(Envelope(1, 1.5, 1, 1.5));
		tests.push_back(Envelope(-2, -1.5, -0.5, 0.5));
		tests.push_back(Envelope(-0.5, 0.5, 1.5, 2));
		tests.push_back(Envelope(-2, -1.5, 0.5, 1.5));
		tests.push_back(Envelope(0.5, 1.5, 1.5, 2));

		for (size_t i = 0; i < tests.size(); ++i) {
			if (e1.contain(tests[i]) != (i == 0)) {
				failedCase += 1;
				wrongMessage(e1, tests[i], (i != 0));
			}
			if (tests[i].contain(e1) == true) {
				failedCase += 1;
				wrongMessage(tests[i], e1, true);
			}
		}
		cout << "Envelope Contain: " << tests.size() * 2 - failedCase << " / " << tests.size() * 2 << " tests are passed" << endl;

		failedCase = 0;
		for (size_t i = 0; i < tests.size(); ++i) {
			if (e1.intersect(tests[i]) != (i < 6)) {
				failedCase += 1;
				wrongMessage(e1, tests[i], (i < 6));
			}
			if (tests[i].intersect(e1) != (i < 6)) {
				failedCase += 1;
				wrongMessage(tests[i], e1, (i < 6));
			}
		}
		cout << "Envelope Intersect: " << tests.size() * 2 - failedCase << " / " << tests.size() * 2 << " tests are passed" << endl;

		failedCase = 0;
		vector<Envelope> results;
		results.push_back(Envelope(-1, 1, -1, 1));
		results.push_back(Envelope(-1, 1, -1, 1.5));
		results.push_back(Envelope(-1, 1.5, -1, 1));
		results.push_back(Envelope(-1.5, 1, -1.5, 1));
		results.push_back(Envelope(-2, 1, -1, 1));
		results.push_back(Envelope(-1, 1.5, -1, 1.5));
		results.push_back(Envelope(-2, 1, -1, 1));
		results.push_back(Envelope(-1, 1, -1, 2));
		results.push_back(Envelope(-2, 1, -1, 1.5));
		results.push_back(Envelope(-1, 1.5, -1, 2));
		for (size_t i = 0; i < tests.size(); ++i) {
			if (e1.unionEnvelope(tests[i]) != results[i]) {
				failedCase += 1;
				wrongMessage(e1, tests[i], e1.unionEnvelope(tests[i]), results[i]);
			}
			if (tests[i].unionEnvelope(e1) != results[i]) {
				failedCase += 1;
				wrongMessage(tests[i], e1, e1.unionEnvelope(tests[i]), results[i]);
			}
		}
		cout << "Envelope Union: " << tests.size() * 2 - failedCase << " / " << tests.size() * 2 << " tests are passed" << endl;
	}
	else if (t == TEST2) {
		cout << "²âÊÔ2: Distance between Point and LineString" << endl;

		vector<Point> points;
		points.push_back(Point(0, 0));
		points.push_back(Point(10, 10));
		LineString line(points);

		points.push_back(Point(-10, -10));
		points.push_back(Point(20, 20));
		points.push_back(Point(5, 5));
		points.push_back(Point(10, 0));
		points.push_back(Point(10, -10));
		points.push_back(Point(0, 10));
		points.push_back(Point(0, 20));
		points.push_back(Point(20, 0));

		double dists[] = { 0, 0, 14.1421, 14.1421, 0, 7.07107, 14.1421, 7.07107, 14.1421, 14.1421 };

		int failedCase = 0;
		for (size_t i = 0; i < points.size(); ++i) {
			double dist = points[i].distance(&line);
			if (fabs(dist - dists[i]) > 0.0001) {
				failedCase += 1;
				cout << "Your answer is " << dist << " for test between ";
				line.print();
				cout << " and ";
				points[i].print();
				cout << ", but the answer is " << dists[i] << endl;
			}
		}
		cout << "Distance between Point and LineString: " << points.size() - failedCase << " / " << points.size() << " tests are passed" << endl;
	}
	else if (t == TEST3) {
		cout << "²âÊÔ3: Distance between Point and Polygon" << endl;

		vector<Point> points;
		points.push_back(Point(5, 0));
		points.push_back(Point(3, 6));
		points.push_back(Point(2, 4));
		points.push_back(Point(-2, 4));
		points.push_back(Point(-3, 5));
		points.push_back(Point(-5, 0));
		points.push_back(Point(0, -3));
		points.push_back(Point(5, 0));
		LineString line(points);
		Polygon poly(line);

		points.clear();
		points.push_back(Point(5, 4));
		points.push_back(Point(3, 4));
		points.push_back(Point(0, 4));
		points.push_back(Point(-3, 4));
		points.push_back(Point(-5, 4));
		points.push_back(Point(5, 5));
		points.push_back(Point(3, 5));
		points.push_back(Point(0, 5));
		points.push_back(Point(-3, 5));
		points.push_back(Point(0, 0));

		double dists[] = { 1.26491, 0, 0, 0, 1.48556, 1.58114, 0, 1, 0, 0 };

		int failedCase = 0;
		for (size_t i = 0; i < points.size(); ++i) {
			double dist = points[i].distance(&poly);
			if (fabs(dist - dists[i]) > 0.00001) {
				failedCase += 1;
				cout << "Your answer is " << dist << " for test between ";
				poly.print();
				cout << " and ";
				points[i].print();
				cout << ", but the answer is " << dists[i] << endl;
			}
		}
		cout << "Distance between Point and Polygon: " << points.size() - failedCase << " / " << points.size() << " tests are passed" << endl;
	}
	else if (t == TEST4) {
		cout << "²âÊÔ4: QuadTree Construction" << endl;
		int ncase, cct;
		ncase = cct = 2;
		QuadTree qtree;
		vector<Geometry *> geom = readGeom(".//data/polygon");
		vector<Feature> features;

		for (size_t i = 0; i < geom.size(); ++i)
			features.push_back(Feature("", geom[i]));

		qtree.setCapacity(1);
		qtree.constructQuadTree(features);

		int height, interiorNum, leafNum;
		qtree.countHeight(height);
		qtree.countQuadNode(interiorNum, leafNum);

		if (!(height == 6 && interiorNum == 8 && leafNum == 25)){
			cout <<"Case 1: "<< "Your answer is height: " << height << ", interiorNum: " << interiorNum <<
				", leafNum: " << leafNum << " for case1, but the answer is height: 6, interiorNum: 8, leafNum: 25\n";
			--cct;
		}

		features.clear();
		for (size_t i = 0; i < geom.size(); ++i)
			delete geom[i];
		geom.clear();

		vector<Geometry *> geom2 = readGeom(".//data/highway");
		vector<Feature> features2;
		QuadTree qtree2;

		for (size_t i = 0; i < geom2.size(); ++i)
			features2.push_back(Feature("", geom2[i]));

		qtree2.setCapacity(20);
		qtree2.constructQuadTree(features2);

		int height2, interiorNum2, leafNum2;
		qtree2.countHeight(height2);
		qtree2.countQuadNode(interiorNum2, leafNum2);

	    if (!(height2 == 11 && interiorNum2 == 1386 && leafNum2 == 4159)){
			cout <<"Case 2: "<< "Your answer is height: " << height2 << ", interiorNum: " << interiorNum2 <<
				", leafNum: " << leafNum2 << " for case2, but the answer is height: 11, interiorNum: 1386, leafNum: 4159\n";
			--cct;
		}

		features2.clear();
		for (size_t i = 0; i < geom2.size(); ++i)
			delete geom2[i];
		geom2.clear();

		cout << "QuadTree Construction: " << cct << " / " << ncase << " tests are passed" << endl;
	}
	else if (t == TEST5) {
	cout << "²âÊÔ5: RTree Construction" << endl;
	Point a(4, 3);  
	Point b(2, 3);
	Point c(2, 2);
	Point d(2, 7);
	Point e(3, 8);
	Point f(4, 8);
	Point g(8, 7);
	Point h(6, 6);
	Point i(8, 6);
	Point j(2, 4);
	Point k(1, 6);
	Point l(7, 9);
	Point m(9, 8);
	cout << "test sample 1: " << endl;
		vector<Geometry*> geom = {&a,&b,&c,&d,&e,&f,&g,&h,&i,&j,&k,&l,&m};
	    vector<Feature> features;

	    for (size_t i = 0; i < geom.size(); ++i)
		    features.push_back(Feature("", geom[i]));
		vector<Feature*> F;
		for (int i = 0; i < features.size(); i++) {
			F.push_back(&features[i]);
		}

		RTree rtree;
		rtree.Construction(F,3);
		//RTree rtree(F);
		cout << "Height: "; rtree.getHeight();
		//cout << rtree.MBR.getMinX() << " " << rtree.MBR.getMinY() << " " << rtree.MBR.getMaxX() << " " << rtree.MBR.getMaxY() << endl;
		//rtree.All_area();
		cout << "feature number: "<<rtree.elememtCount() << endl;
		cout << "**********print begin*********" << endl;
		rtree.printTree();
		cout << "**********print end***********" << endl;

    }
	else if (t == TEST6) {
	    cout << "²âÊÔ6: Interior Ring test" << endl;

		Point O1(12, 10);Point O2(14, 11); Point O3(16, 10); Point O4(17, 8); Point O5(16, 7);
		Point O6(13, 7); Point O7(12, 8); Point I1(13.5, 9.5); Point I2(15.5, 9.5); Point I3(14, 8);
		vector<Point> OutPt;
		vector<Point>InPt;
		OutPt.push_back(O1); OutPt.push_back(O2); OutPt.push_back(O3); OutPt.push_back(O4); OutPt.push_back(O5);
		OutPt.push_back(O6); OutPt.push_back(O7); InPt.push_back(I1); InPt.push_back(I2); InPt.push_back(I3);
		OutPt.push_back(O1);
		InPt.push_back(I1);
		LineString Outer(OutPt);
		LineString Inner(InPt);
		Polygon PolywithIn(Outer, Inner);

		cout << "distance function test : " << endl;
		Point T1(14, 4); Point T2(15, 8); Point T3(14, 9);
		cout << "dist T1 to Polygon: " << T1.distance(&PolywithIn) << endl;
		cout << "dist T2 to Polygon: " << T2.distance(&PolywithIn) << endl;
		cout << "dist T3 to Polygon: " << T3.distance(&PolywithIn) << endl;

		cout << endl << "Polygon intersect test : " << endl;
		Envelope rect1(15, 18.5, 4,7.5);
		Envelope rect2(16, 18, 12, 14);
		Envelope rect3(12.5, 14, 8, 9.5);
		Envelope rect4(14.3, 14.6, 9, 9.3);
		if (PolywithIn.intersects(rect1))cout << "rect1 Intersect     suppose to be: intersect" << endl;
		else cout << "rect1 Disjoint     suppose to be: intersect" << endl;
		if (PolywithIn.intersects(rect2))cout << "rect2 Intersect     suppose to be: disjoint" << endl;
		else cout << "rect2 Disjoint     suppose to be: disjoint" << endl;
		if (PolywithIn.intersects(rect3))cout << "rect3 Intersect     suppose to be: intersect" << endl;
		else cout << "rect3 Disjoint     suppose to be: intersect" << endl;
		if (PolywithIn.intersects(rect4))cout << "rect4 Intersect     suppose to be: disjoint" << endl;
		else cout << "rect4 Disjoint     suppose to be: disjoint" << endl;

		cout << "test case 2:" << endl;
		Point o1(0, 0); Point o2(0, 15); Point o3(20, 15); Point o4(20, 12); Point o5(15, 5); Point o6(15, 0);
		Point i11(2, 1); Point i12(2, 5); Point i13(5, 5);
		Point i21(2, 10); Point i22(2, 13); Point i23(10, 12); Point i24(10, 10);
		Point i31(8, 6); Point i32(13, 10); Point i33(13, 2);
		OutPt.clear();
		OutPt.push_back(o1); OutPt.push_back(o2); OutPt.push_back(o3); OutPt.push_back(o4); OutPt.push_back(o5); OutPt.push_back(o6); OutPt.push_back(o1);
		vector<Point> In1; In1.push_back(i11); In1.push_back(i12); In1.push_back(i13); In1.push_back(i11);
		vector<Point> In2; In2.push_back(i21); In2.push_back(i22); In2.push_back(i23); In2.push_back(i24); In2.push_back(i21);
		vector<Point> In3; In3.push_back(i31); In3.push_back(i32); In3.push_back(i33); In3.push_back(i31);
		LineString outer(OutPt); LineString i1(In1); LineString i2(In2); LineString i3(In3);
		vector<LineString>inner; inner.push_back(i1); inner.push_back(i2); inner.push_back(i3);
		Polygon P2(outer, inner);
		Envelope e21(3, 9, 1, 7); Envelope e22(3, 4.5, 10.5, 12); Envelope e23(12, 14.5, 5, 7.5);
		if (P2.intersects(e21))cout << "rect1 Intersect     suppose to be: intersect" << endl;
		else cout << "rect1 Disjoint     suppose to be: intersect" << endl;
		if (P2.intersects(e22))cout << "rect2 Intersect     suppose to be: disjoint" << endl;
		else cout << "rect2 Disjoint     suppose to be: disjoint" << endl;
		if (P2.intersects(e23))cout << "rect3 Intersect     suppose to be: intersect" << endl;
		else cout << "rect3 Disjoint     suppose to be: intersect" << endl;


    }
	else if (t == TEST7) {
	    cout << "²âÊÔ7: RTreeAnalysis" << endl;
	    RTreeAnalysis();
		
    }
	else if (t == TEST8) {
		cout << "²âÊÔ8: QuadTreeAnalysis" << endl;
		QuadTreeAnalysis();
	}
	else if (t == TEST9) {
	    cout << "²âÊÔ9£ºQuadTree and R-Tree comparative analysis" << endl;
		cout << "**********Test taxi data********** " << endl;
		vector<Feature> features;
		vector<Geometry*> geom = readGeom(".//data/taxi");
		vector<string> name = readName(".//data/taxi");
		vector<Feature*> f;
		features.clear();
		features.reserve(geom.size());
		for (size_t i = 0; i < geom.size(); ++i)
			features.push_back(Feature(name[i], geom[i]));
		cout << "taxi number: " << geom.size() << endl;
		for (size_t i = 0; i < features.size(); i++) {
			f.push_back(&features[i]);
		}
		clock_t s1, s2, e1, e2;
		double t1, t2;
		srand(time(NULL));
		for (size_t i = 128; i <= 512; i *= 2) {
			QuadTree* qtree = new QuadTree();
			cout << "Capacity: " << i << endl;
			s1 = clock();
			qtree->setCapacity(i);
			qtree->constructQuadTree(features);
			e1 = clock();
			t1 = (e1 - s1) / 1000.0;
			RTree rtree;
			s2 = clock();
			rtree.Construction(f,i);
			e2 = clock();
			t2 = (e2 - s2) / 1000.0;
			cout << " Construction test:" << endl << "QuadTree construction time: " << t1<<"s"
				<< "    R-Tree construction time: " << t2 << "s"<<endl;

			double x, y;
			vector<hw6::Feature> candidateFeatures;
			s1 = clock();
			for (int i = 0; i < 1000; ++i) {
				x = -((rand() % 225) / 1000.0 + 73.9812);
				y = (rand() % 239) / 1000.0 + 40.7247;
				candidateFeatures.clear();
				qtree->NNQuery(x, y, candidateFeatures);
				Feature nearestFeature;
				if (!candidateFeatures.empty()) {
					size_t nearestFeatureIndex = 0;
					double minDist = INFINITY;
					for (size_t j = 0; j < candidateFeatures.size(); ++j) {
						if (candidateFeatures[j].distance(x, y) < minDist) {
							minDist = candidateFeatures[j].distance(x, y);
							nearestFeatureIndex = j;
						}
					}
					nearestFeature = candidateFeatures[nearestFeatureIndex];
				}
			}
			e1 = clock();
			s2 = clock();
			for (int i = 0; i < 1000; i++) {
				x = -((rand() % 225) / 1000.0 + 73.9812);
				y = (rand() % 239) / 1000.0 + 40.7247;
				rtree.NNQuery(x, y, candidateFeatures);
				Feature nearestFeature;
				if (!candidateFeatures.empty()) {
					size_t nearestFeatureIndex = 0;
					double minDist = INFINITY;
					for (size_t j = 0; j < candidateFeatures.size(); ++j) {
						if (candidateFeatures[j].distance(x, y) < minDist) {
							minDist = candidateFeatures[j].distance(x, y);
							nearestFeatureIndex = j;
						}
					}
					nearestFeature = candidateFeatures[nearestFeatureIndex];
				}
				candidateFeatures.clear();
			}
			e2 = clock();
			t1 = (e2 - s1)/1000.0;
			t2 = (e2 - s2)/1000.0;
			cout<<" NN-Query time: "<<endl<< "QuadTree NN-Query time: " << t1 << "s"
				<< "    R-Tree NN-Query time: " << t2 << "s" << endl<<endl;

			delete qtree;
		}

		cout << "**********Test station data********** " << endl;
		geom.clear(); name.clear();
		geom = readGeom(".//data/station");
		name = readName(".//data/station");
		features.clear();
		features.reserve(geom.size());
		for (size_t i = 0; i < geom.size(); ++i)
			features.push_back(Feature(name[i], geom[i]));
		cout << "station number: " << geom.size() << endl;
		for (size_t i = 0; i < features.size(); i++) {
			f.push_back(&features[i]);
		}
		srand(time(NULL));
		for (size_t i = 10; i <= 20; i +=2 ) {
			QuadTree* qtree = new QuadTree();
			cout << "Capacity: " << i << endl;
			s1 = clock();
			qtree->setCapacity(i);
			qtree->constructQuadTree(features);
			e1 = clock();
			t1 = (e1 - s1) / 1000.0;
			RTree rtree;
			s2 = clock();
			rtree.Construction(f, i);
			e2 = clock();
			t2 = (e2 - s2) / 1000.0;
			cout << " Construction test:" << endl << "QuadTree construction time: " << t1 << "s"
				<< "    R-Tree construction time: " << t2 << "s" << endl;

			double x, y;
			vector<hw6::Feature> candidateFeatures;
			s1 = clock();
			for (int i = 0; i < 1000; ++i) {
				x = -((rand() % 225) / 1000.0 + 73.9812);
				y = (rand() % 239) / 1000.0 + 40.7247;
				candidateFeatures.clear();
				qtree->NNQuery(x, y, candidateFeatures);
				Feature nearestFeature;
				if (!candidateFeatures.empty()) {
					size_t nearestFeatureIndex = 0;
					double minDist = INFINITY;
					for (size_t j = 0; j < candidateFeatures.size(); ++j) {
						if (candidateFeatures[j].distance(x, y) < minDist) {
							minDist = candidateFeatures[j].distance(x, y);
							nearestFeatureIndex = j;
						}
					}
					nearestFeature = candidateFeatures[nearestFeatureIndex];
				}
			}
			e1 = clock();
			s2 = clock();
			for (int i = 0; i < 1000; i++) {
				x = -((rand() % 225) / 1000.0 + 73.9812);
				y = (rand() % 239) / 1000.0 + 40.7247;
				rtree.NNQuery(x, y, candidateFeatures);
				Feature nearestFeature;
				if (!candidateFeatures.empty()) {
					size_t nearestFeatureIndex = 0;
					double minDist = INFINITY;
					for (size_t j = 0; j < candidateFeatures.size(); ++j) {
						if (candidateFeatures[j].distance(x, y) < minDist) {
							minDist = candidateFeatures[j].distance(x, y);
							nearestFeatureIndex = j;
						}
					}
					nearestFeature = candidateFeatures[nearestFeatureIndex];
				}
				candidateFeatures.clear();
			}
			e2 = clock();
			t1 = (e2 - s1) / 1000.0;
			t2 = (e2 - s2) / 1000.0;
			cout << " NN-Query time: " << endl << "QuadTree NN-Query time: " << t1 << "s"
				<< "    R-Tree NN-Query time: " << t2 << "s" << endl << endl;

			delete qtree;
		}


		cout << "**********Test road data********** " << endl;
		geom.clear(); 
		geom = readGeom(".//data/highway");
		features.clear();
		features.reserve(geom.size());
		for (size_t i = 0; i < geom.size(); ++i)
			features.push_back(Feature(to_string(i), geom[i]));
		cout << "road number: " << geom.size() << endl;
		for (size_t i = 0; i < features.size(); i++) {
			f.push_back(&features[i]);
		}
		srand(time(NULL));
		for (size_t i = 64; i <= 512; i*=2) {
			QuadTree* qtree = new QuadTree();
			cout << "Capacity: " << i << endl;
			s1 = clock();
			qtree->setCapacity(i);
			qtree->constructQuadTree(features);
			e1 = clock();
			t1 = (e1 - s1) / 1000.0;
			RTree rtree;
			s2 = clock();
			rtree.Construction(f, i);
			e2 = clock();
			t2 = (e2 - s2) / 1000.0;
			cout << " Construction test:" << endl << "QuadTree construction time: " << t1 << "s"
				<< "    R-Tree construction time: " << t2 << "s" << endl;

			double x, y;
			vector<hw6::Feature> candidateFeatures;
			s1 = clock();
			for (int i = 0; i < 1000; ++i) {
				x = -((rand() % 225) / 1000.0 + 73.9812);
				y = (rand() % 239) / 1000.0 + 40.7247;
				candidateFeatures.clear();
				qtree->NNQuery(x, y, candidateFeatures);
				Feature nearestFeature;
				if (!candidateFeatures.empty()) {
					size_t nearestFeatureIndex = 0;
					double minDist = INFINITY;
					for (size_t j = 0; j < candidateFeatures.size(); ++j) {
						if (candidateFeatures[j].distance(x, y) < minDist) {
							minDist = candidateFeatures[j].distance(x, y);
							nearestFeatureIndex = j;
						}
					}
					nearestFeature = candidateFeatures[nearestFeatureIndex];
				}
			}
			e1 = clock();
			s2 = clock();
			for (int i = 0; i < 1000; i++) {
				x = -((rand() % 225) / 1000.0 + 73.9812);
				y = (rand() % 239) / 1000.0 + 40.7247;
				rtree.NNQuery(x, y, candidateFeatures);
				Feature nearestFeature;
				if (!candidateFeatures.empty()) {
					size_t nearestFeatureIndex = 0;
					double minDist = INFINITY;
					for (size_t j = 0; j < candidateFeatures.size(); ++j) {
						if (candidateFeatures[j].distance(x, y) < minDist) {
							minDist = candidateFeatures[j].distance(x, y);
							nearestFeatureIndex = j;
						}
					}
					nearestFeature = candidateFeatures[nearestFeatureIndex];
				}
				candidateFeatures.clear();
			}
			e2 = clock();
			t1 = (e2 - s1) / 1000.0;
			t2 = (e2 - s2) / 1000.0;
			cout << " NN-Query time: " << endl << "QuadTree NN-Query time: " << t1 << "s"
				<< "    R-Tree NN-Query time: " << t2 << "s" << endl << endl;

			delete qtree;
		}
    }

	cout << "**********************End**********************" << endl;
}