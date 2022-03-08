#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <vector>
#include <algorithm>
#include <iostream> 
using namespace std;

namespace hw6 {

class Point;
class LineString;
class Polygon;

class Envelope {
private:
	double minX;
	double minY;
	double maxX;
	double maxY;

public:
	Envelope() : minX(0), minY(0), maxX(0), maxY(0) {}
	Envelope(double minX, double maxX, double minY, double maxY) : minX(minX), minY(minY), maxX(maxX), maxY(maxY) {}

	double getMinX() const { return minX; }
	double getMinY() const { return minY; }
	double getMaxX() const { return maxX; }
	double getMaxY() const { return maxY; }

	double getWidth()  const { return maxX - minX; }
	double getHeight() const { return maxY - minY; }
	double getArea() const {
		if (maxX == minX && minY == maxY)return 0;
		else if (maxX == minX || minY == maxY) { if (maxX == minX)return maxY - minY; else return maxX - minX; }
	    else if(maxX!=minX&&maxY!=minY) return (maxX - minX) * (maxY - minY); 
		//return (maxX - minX) * (maxY - minY);
	}
	double distance(double x, double y) const;
	bool contain(double x, double y) const;

	void draw() const;

	void print() const { cout << "Envelope( " << minX << " " << maxX << " " << minY << " " << maxY << ") "; }

	bool operator == (const Envelope &t1) const { return (minX == t1.minX && minY == t1.minY && maxX == t1.maxX && maxY == t1.maxY); }
	bool operator != (const Envelope &t1) const { return !(*this == t1); }

	bool contain(const Envelope& envelope) const;
	bool intersect(const Envelope& envelope) const;
	Envelope unionEnvelope(const Envelope& envelope) const;
};

/*
 * Geometry hierarchy
 */
class Geometry {
protected:
	Envelope envelope;

public:
	Geometry() {}
	virtual ~Geometry() {}

	const Envelope& getEnvelope() const { return envelope; }

	virtual void     constructEnvelope() = 0;
	virtual double   distance(const Geometry* geom)   const { return geom->distance(this); } // Euclidean distance
	virtual double   distance(const Point* point)     const = 0;
	virtual double   distance(const LineString* line) const = 0;
	virtual double   distance(const Polygon* polygon) const = 0;
	virtual bool     intersects(const Envelope& rect) const = 0;
	virtual void	 draw()                           const = 0;
	virtual void     print()                          const = 0;
};


class Point : public Geometry {
private:
	double x;
	double y;

public:
	Point() : x(0), y(0) {}
	Point(double x, double y) : x(x), y(y) { constructEnvelope(); }
	virtual ~Point() {}

	double getX() const { return x; }
	double getY() const { return y; }

	virtual void constructEnvelope() { envelope = Envelope(x, x, y, y); }

	// Euclidean distance
	virtual double distance(const Point* point) const;
	virtual double distance(const LineString* line) const;
	virtual double distance(const Polygon* polygon) const;

	// intersection test with the envelope for range query
	virtual bool intersects(const Envelope& rect)  const;

	virtual void draw()  const;

	virtual void print() const { cout << "Point(" << x << " " << y << ")"; }
};


class LineString :public Geometry {
private:
	vector<Point> points;

public:
	LineString() {}
	LineString(vector<Point>& pts) : points(pts) { constructEnvelope(); }
	virtual ~LineString() {}

	size_t numPoints()         const { return points.size(); }
	Point  getStartPoint()     const { return points.front(); }
	Point  getEndPoint()       const { return points.back(); }
	Point  getPointN(size_t n) const { return points[n]; }

	virtual void constructEnvelope();

	// Euclidean distance
	virtual double distance(const Point* point) const;
	virtual double distance(const LineString* line) const;
	virtual double distance(const Polygon* polygon) const;

	// intersection test with the envelope for range query
	virtual bool intersects(const Envelope& rect)  const;

	virtual void draw()  const;

	virtual void print() const;
};


class Polygon : public Geometry {
private:
	LineString exteriorRing;
	LineString interiorRing;
	vector<LineString> interiorRingGroup;
	
public:
	bool hasinterirorRing = false;
	bool hasinteriorGroup = false;
	Polygon() {}
	Polygon(LineString& ering) : exteriorRing(ering){ constructEnvelope(); }
	Polygon(LineString& ering, LineString& iring) : exteriorRing(ering), interiorRing(iring) { hasinterirorRing=true; constructEnvelope(); }
	Polygon(LineString& ering, vector<LineString>& iring) :exteriorRing(ering), interiorRingGroup(iring) { hasinterirorRing = true; hasinteriorGroup = true; constructEnvelope(); }
	virtual ~Polygon() {}

	LineString getExteriorRing() const { return exteriorRing; }
	LineString gerInteriorRing() const { return interiorRing; }
	vector<LineString> getInteriorRing() const { return interiorRingGroup; }

	virtual void constructEnvelope() { envelope = exteriorRing.getEnvelope(); }

	// Euclidean distance
	virtual double distance(const Point* point) const { return point->distance(this); }
	virtual double distance(const LineString* line) const { return line->distance(this); }
	virtual double distance(const Polygon* polygon) const;

	// intersection test with the envelope for range query
	virtual bool intersects(const Envelope& rect)  const;

	virtual void draw()  const;

	virtual void print() const;
};

}

#endif