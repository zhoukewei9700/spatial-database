#include "Geometry.h"
#include <cmath>
#include <gl/freeglut.h> 

#define NOT_IMPLEMENT -1.0

namespace hw6 {

/*
 * Envelope functions
 */
double Envelope::distance(double x, double y) const
{
	if (contain(x, y)) return 0;
	else if (minX <= x && x <= maxX) {
		return min(abs(minY - y), abs(maxY - y));
	}
	else if (minY <= y && y <= maxY) {
		return min(abs(minX - x), abs(maxX - x));
	}
}

bool Envelope::contain(double x, double y) const
{
	return x >= minX && x <= maxX && y >= minY && y <= maxY;
}

bool Envelope::contain(const Envelope& envelope) const
{
	// Task 测试Envelope是否包含关系
	// Write your code here
	return contain(envelope.getMaxX(), envelope.getMaxY())
		&& contain(envelope.getMinX(), envelope.getMaxY())
		&& contain(envelope.getMaxX(), envelope.getMinY())
		&& contain(envelope.getMinX(), envelope.getMinY());
}

bool Envelope::intersect(const Envelope& envelope) const
{
	// Task 测试Envelope是否相交
	// Write your code here
	bool flag = (envelope.getMaxX() < minX) || (envelope.getMinX() > maxX)
		|| (envelope.getMaxY() < minY) || (envelope.getMinY() > maxY);
	return !flag;
}

Envelope Envelope::unionEnvelope(const Envelope& envelope) const
{
	// Task 合并两个Envelope生成一个新的Envelope
	// Write your code here
	return Envelope(min(this->minX, envelope.minX), max(this->maxX, envelope.maxX), min(this->minY, envelope.minY), max(this->maxY, envelope.maxY));
}

void Envelope::draw() const
{
	glBegin(GL_LINE_STRIP);

	glVertex2d(minX, minY);
	glVertex2d(minX, maxY);
	glVertex2d(maxX, maxY);
	glVertex2d(maxX, minY);
	glVertex2d(minX, minY);

	glEnd();
}


/*
 * Points functions
 */
double Point::distance(const Point* point) const
{
	return sqrt((x - point->x) * (x - point->x) + (y - point->y) * (y - point->y));
}

double Point::distance(const LineString* line) const
{
	double mindist = line->getPointN(0).distance(this);
	for (size_t i = 0; i < line->numPoints() - 1; ++i) {
		double dist = 0;
		double x1 = line->getPointN(i).getX();
		double y1 = line->getPointN(i).getY();
		double x2 = line->getPointN(i + 1).getX();
		double y2 = line->getPointN(i + 1).getY();
		// Task calculate the distance between Point P(x, y) and Line [P1(x1, y1), P2(x2, y2)] (less than 10 lines)
		// Write your code here
		double k = (y2 - y1) / (x2 - x1);//直线斜率
		double h = (x2 * y1 - x1 * y2) / (x2 - x1);//直线截距
		dist = abs((k * x + h - y) / sqrt(k * k + 1));//点到直线欧氏距离
		double a1, a2, b1, b2;//向量(P-P1):(a1,b1),向量(P2-P1):(a2,b2)
		a1 = x - x1; b1 = y - y1;
		a2 = x2 - x1; b2 = y2 - y1;
		double inner_product = a1 * a2 + b1 * b2;
		double judge = a2 * a2 + b2 * b2;
		//投影点在线段外，取点到线段两端点距离的最小值
		if (inner_product<0 || inner_product>judge) { dist = min(line->getPointN(i).distance(this), line->getPointN(i + 1).distance(this)); }
		
		if (dist < mindist)
			mindist = dist;
	}
	return mindist;
}

double Point::distance(const Polygon* polygon) const
{
	LineString line = polygon->getExteriorRing();
	size_t n = line.numPoints();

	bool inPolygon = false;
	// Task whether Point P(x, y) is within Polygon (less than 15 lines)
	// write your code here
	//判断点在外环内外
	int count = 0;//计数器
	for (size_t i = 0; i < n-1; i++)
	{
		double x1 = line.getPointN(i).getX();double y1 = line.getPointN(i).getY();
		double x2 = line.getPointN(i + 1).getX();double y2 = line.getPointN(i + 1).getY();
		double x0 = (y-y1)*(x1-x2)/(y1-y2)+x1;//射线水平向右
		//特殊情况处理，在边界上
		if ((x == x1 && y == y1) || (x == x2 && y == y2)) { inPolygon = false; break; }
		else {
			if (y1 == y2)continue;//线段水平，肯定无交点
			if (y >= max(y1, y2) || y < min(y1, y2))continue;//超出纵坐标范围则无交点
			if (x0 > x)count++;//射线和线段有交点，计数器+1
		}
	}
	if (count % 2 != 0)inPolygon = true;
	else inPolygon = false;
	double mindist = 0;
	if (!inPolygon) {
		mindist = this->distance(&line); return mindist;
	}
	else if(polygon->hasinterirorRing){
		if (!polygon->hasinteriorGroup) {
			//判断点在内环内外
			LineString line2 = polygon->gerInteriorRing();
			int n2 = line2.numPoints();
			bool inInteriorRing = false;
			int count2 = 0;
			for (int i = 0; i < n2 - 1; i++)
			{
				double x1 = line2.getPointN(i).getX(); double y1 = line2.getPointN(i).getY();
				double x2 = line2.getPointN(i + 1).getX(); double y2 = line2.getPointN(i + 1).getY();
				double x0 = (y - y1) * (x1 - x2) / (y1 - y2) + x1;//射线水平向右
				//特殊情况处理，在边界上
				if ((x == x1 && y == y1) || (x == x2 && y == y2)) { inInteriorRing = false; break; }
				else {
					if (y1 == y2)continue;//线段水平，肯定无交点
					if (y >= max(y1, y2) || y < min(y1, y2))continue;//超出纵坐标范围则无交点
					if (x0 > x)count2++;//射线和线段有交点，计数器+1
				}
			}
			if (count2 % 2 != 0)inInteriorRing = true;
			else inInteriorRing = false;
			if (inInteriorRing == true) {
				mindist = this->distance(&line2); return mindist;
			}
		}
		else {
			bool inInteriorRing = false;
			for (int i = 0; i < polygon->getInteriorRing().size(); i++) {
				vector<LineString> l = polygon->getInteriorRing();
				LineString line2 = l[i];
				int n2 = line2.numPoints();
				int count2 = 0;
				for (int i = 0; i < n2 - 1; i++)
				{
					double x1 = line2.getPointN(i).getX(); double y1 = line2.getPointN(i).getY();
					double x2 = line2.getPointN(i + 1).getX(); double y2 = line2.getPointN(i + 1).getY();
					double x0 = (y - y1) * (x1 - x2) / (y1 - y2) + x1;//射线水平向右
					//特殊情况处理，在边界上
					if ((x == x1 && y == y1) || (x == x2 && y == y2)) { inInteriorRing = false; break; }
					else {
						if (y1 == y2)continue;//线段水平，肯定无交点
						if (y >= max(y1, y2) || y < min(y1, y2))continue;//超出纵坐标范围则无交点
						if (x0 > x)count2++;//射线和线段有交点，计数器+1
					}
				}
				if (count2 % 2 != 0){inInteriorRing = true; break;}
			}
			if (inInteriorRing) {
				vector<LineString> l = polygon->getInteriorRing();
				mindist = this->distance(&(l[0]));
				for (int i = 1; i < l.size(); i++) {
					if (this->distance(&(l[i])) < mindist)mindist = this->distance(&(l[i]));
				}
				return mindist;
			}
		}
	}
	return mindist;
}

bool Point::intersects(const Envelope& rect)  const
{
	return (x >= rect.getMinX()) && (x <= rect.getMaxX()) && (y >= rect.getMinY()) && (y <= rect.getMaxY());
}

void Point::draw()  const
{
	glBegin(GL_POINTS);
	glVertex2d(x, y);
	glEnd();
}


/*
 * LineString functions
 */
void LineString::constructEnvelope()
{
	double minX, minY, maxX, maxY;
	maxX = minX = points[0].getX();
	maxY = minY = points[0].getY();
	for (size_t i = 1; i < points.size(); ++i) {
		maxX = max(maxX, points[i].getX());
		maxY = max(maxY, points[i].getY());
		minX = min(minX, points[i].getX());
		minY = min(minY, points[i].getY());
	}
	envelope = Envelope(minX, maxX, minY, maxY);
}

double LineString::distance(const Point* point) const
{
	return point->distance(this);
}

double LineString::distance(const LineString* line) const
{
	cout << "to be implemented: LineString::distance(const LineString* line)\n";
	return NOT_IMPLEMENT;
}

double LineString::distance(const Polygon* polygon) const
{
	cout << "to be implemented: LineString::distance(const Polygon* polygon)\n";
	return NOT_IMPLEMENT;
}

typedef int OutCode;

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)
// ASSUME THAT xmax, xmin, ymax and ymin are global constants.
OutCode ComputeOutCode(double x, double y, double xmin, double xmax, double ymin, double ymax)
{
	OutCode code;

	code = INSIDE;          // initialised as being inside of [[clip window]]

	if (x < xmin)           // to the left of clip window
		code |= LEFT;
	else if (x > xmax)      // to the right of clip window
		code |= RIGHT;
	if (y < ymin)           // below the clip window
		code |= BOTTOM;
	else if (y > ymax)      // above the clip window
		code |= TOP;
	
	return code;
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (xmin, ymin) to (xmax, ymax).
bool intersectTest(double x0, double y0, double x1, double y1, double xmin, double xmax, double ymin, double ymax)
{
	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	OutCode outcode0 = ComputeOutCode(x0, y0, xmin, xmax, ymin, ymax);
	OutCode outcode1 = ComputeOutCode(x1, y1, xmin, xmax, ymin, ymax);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) {
			// bitwise OR is 0: both points inside window; trivially accept and exit loop
			accept = true;
			break;
		}
		else if (outcode0 & outcode1) {
			// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
			// or BOTTOM), so both must be outside window; exit loop (accept is false)
			break;
		}
		else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			double x, y;

			// At least one endpoint is outside the clip rectangle; pick it.
			OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

			// Now find the intersection point;
			// use formulas:
			//   slope = (y1 - y0) / (x1 - x0)
			//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
			//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
			// No need to worry about divide-by-zero because, in each case, the
			// outcode bit being tested guarantees the denominator is non-zero
			if (outcodeOut & TOP) {           // point is above the clip window
				x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
				y = ymax;
			}
			else if (outcodeOut & BOTTOM) { // point is below the clip window
				x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
				y = ymin;
			}
			else if (outcodeOut & RIGHT) {  // point is to the right of clip window
				y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
				x = xmax;
			}
			else if (outcodeOut & LEFT) {   // point is to the left of clip window
				y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
				x = xmin;
			}
			
			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = ComputeOutCode(x0, y0, xmin, xmax, ymin, ymax);
			}
			else {
				x1 = x;
				y1 = y;
				outcode1 = ComputeOutCode(x1, y1, xmin, xmax, ymin, ymax);
			}
		}
	}
	return accept;
}

bool LineString::intersects(const Envelope& rect)  const
{
	double xmin = rect.getMinX();
	double xmax = rect.getMaxX();
	double ymin = rect.getMinY();
	double ymax = rect.getMaxY();

	for (size_t i = 1; i < points.size(); ++i)
		if (intersectTest(points[i - 1].getX(), points[i - 1].getY(), points[i].getX(), points[i].getY(), xmin, xmax, ymin, ymax))
			return true;
	return false;
}

void LineString::draw()  const
{
	glBegin(GL_LINE_STRIP);
	for (size_t i = 0; i < points.size(); ++i)
		glVertex2d(points[i].getX(), points[i].getY());
	glEnd();
}

void LineString::print() const
{
	cout << "LineString(";
	for (size_t i = 0; i < points.size(); ++i) {
		if (i != 0)
			cout << ", ";
		cout << points[i].getX() << " " << points[i].getY();
	}
	cout << ")";
}

/*
 * Polygon
 */
double Polygon::distance(const Polygon* polygon) const
{
	return min(exteriorRing.distance(polygon), polygon->getExteriorRing().distance(this));
}

bool Polygon::intersects(const Envelope& rect)  const
{
	//cout << "to be implemented: Polygon::intersects(const Envelope& box)\n";
	//Polygon最后一个点和第一个点重复，顶点数=边数
	for (int i = 0; i < this->exteriorRing.numPoints()-1; i++) {
		//逐边判断
		Point p1 = this->exteriorRing.getPointN(i);
		
		if (i+1 == this->exteriorRing.numPoints())
		    Point p2 = this->exteriorRing.getPointN(0);
		Point p2 = this->exteriorRing.getPointN(i+1);
		vector<Point> P; P.push_back(p1); P.push_back(p2);
		LineString line(P);
		if (line.intersects(rect))return true;
	}
	if (this->hasinterirorRing) {//有内环,判断多边形不与外环相交却与内环相交的情况
		if (!hasinteriorGroup){
		     for (int i = 0; i < this->interiorRing.numPoints() - 1; i++) {
			//逐边判断
			    Point p1 = this->interiorRing.getPointN(i);
			    if (i + 1 == this->interiorRing.numPoints())
				    Point p2 = this->interiorRing.getPointN(0);
			    Point p2 = this->interiorRing.getPointN(i + 1);
			    vector<Point> P; P.push_back(p1); P.push_back(p2);
			    LineString line(P);
			    if (line.intersects(rect))return true;
		     }
		}
		else {
			bool flag = false;
			for (int i = 0; i < interiorRingGroup.size(); i++) {
				for (int j = 0; j < interiorRingGroup[i].numPoints() - 1; j++) {
					Point p1 = this->interiorRingGroup[i].getPointN(j);
					if (j + 1 == this->interiorRingGroup[i].numPoints())
						Point p2 = this->interiorRingGroup[i].getPointN(0);
					Point p2 = this->interiorRingGroup[i].getPointN(j + 1);
					vector<Point> P; P.push_back(p1); P.push_back(p2);
					LineString line(P);
					if (line.intersects(rect))flag= true;
				}
			}
			if (flag)return true;
			//else return false;
		}
	}
	return false;
}

void Polygon::draw() const
{
	exteriorRing.draw();
}

void Polygon::print() const
{
	cout << "Polygon(";
	for (size_t i = 0; i < exteriorRing.numPoints(); ++i) {
		if (i != 0)
			cout << ", ";
		Point p = exteriorRing.getPointN(i);
		cout << p.getX() << " " << p.getY();
	}
	cout << ")";
}
	
}
