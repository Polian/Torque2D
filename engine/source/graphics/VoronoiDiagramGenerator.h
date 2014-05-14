/*
MapManager library for the conversion, manipulation and analysis 
of maps used in Mobile Robotics research.
Copyright (C) 2005 Shane O'Sullivan

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

email: shaneosullivan1@gmail.com
*/

#ifndef VORONOI_DIAGRAM_GENERATOR
#define VORONOI_DIAGRAM_GENERATOR

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include "console\console.h"
//#include "../logger/Logger.h"


#ifndef NULL
#define NULL 0
#endif
#define DELETED -2

#define le 0
#define re 1



/**
* \if internal_doc
* @ingroup internal
* \class VPoint
* class to handle a 2d point
* \endif
*/
class VPoint{
public:
	/// defailt ctor
	VPoint() : x(0.0), y(0.0) {}

	/// ctor with initialisation
	VPoint(double _x, double _y) : x(_x), y(_y) {}

	/// addition
	inline VPoint operator + (const VPoint &p) const{
		return VPoint(x + p.x, y + p.y);
	}

	/// subtraction
	inline VPoint operator - (const VPoint &p) const{
		return VPoint(x - p.x, y - p.y);
	}

	/// scalar multiplication
	inline VPoint operator * (const double t) const{
		return VPoint(x*t, y*t);
	}

	/// vector coordinates
	double x, y;
};


/// norm of a vector
inline double norm(const VPoint p){
	return p.x*p.x + p.y*p.y;
}


/// 2D vector product
inline double vector_product(const VPoint &p1, const VPoint &p2){
	return p1.x*p2.y - p1.y*p2.x;
}


/// scalar product
inline double scalar_product(const VPoint &p1, const VPoint &p2){
	return p1.x*p2.x + p1.y*p2.y;
}


/**
* \if internal_doc
* @ingroup internal
* \class GraphEdge
* handle an edge of the Voronoi Diagram.
* \endif
*/
class GraphEdge{
public:
	/// coordinates of the extreme points
	double x1, y1, x2, y2;

	/// indices of the parent sites that define the edge
	int point1, point2;

	/// pointer to the next edge
	GraphEdge* next;
};


/**
* \if internal_doc
* @ingroup internal
* \class Site
* structure used both for particle sites and for vertices.
* \endif
*/
class Site{
public:
	VPoint	coord;
	int sitenbr;
	int refcnt;
};



class Freenode{
public:
	Freenode *nextfree;
};


class FreeNodeArrayList{
public:
	Freenode* memory;
	FreeNodeArrayList* next;
};


class Freelist{
public:
	Freenode *head;
	int nodesize;
};

class Edge{
public:
	double a, b, c;
	Site *ep[2];
	Site *reg[2];
	int edgenbr;
};


class Halfedge{
public:
	Halfedge *ELleft, *ELright;
	Edge *ELedge;
	int ELrefcnt;
	char ELpm;
	Site *vertex;
	volatile double ystar;
	Halfedge *PQnext;
};

/**
* \if internal_doc
* @ingroup internal
* \class VoronoiDiagramGenerator
* Shane O'Sullivan C++ version of Stephan Fortune Voronoi diagram
* generator
* \endif
*/
class VoronoiDiagramGenerator{
public:
	VoronoiDiagramGenerator();
	~VoronoiDiagramGenerator();

	bool generateVoronoi(std::vector<VPoint> *_parent_sites,
		double minX, double maxX, double minY, double maxY,
		double minDist = 0);

	inline void resetIterator(){
		iteratorEdges = allEdges;
	}

	bool getNext(GraphEdge **e){
		if (iteratorEdges == 0)
			return false;

		*e = iteratorEdges;
		iteratorEdges = iteratorEdges->next;
		return true;
	}

	std::vector<VPoint> *parent_sites;
	int n_parent_sites;

private:
	void cleanup();
	void cleanupEdges();
	char *getfree(Freelist *fl);
	Halfedge *PQfind();
	int PQempty();

	Halfedge **ELhash;
	Halfedge *HEcreate(), *ELleft(), *ELright(), *ELleftbnd();
	Halfedge *HEcreate(Edge *e, int pm);

	VPoint PQ_min();
	Halfedge *PQextractmin();
	void freeinit(Freelist *fl, int size);
	void makefree(Freenode *curr, Freelist *fl);
	void geominit();
	void plotinit();

	// GS: removed the unused (always ==0) argument
	bool voronoi(/*int triangulate*/);
	void ref(Site *v);
	void deref(Site *v);
	void endpoint(Edge *e, int lr, Site * s);

	void ELdelete(Halfedge *he);
	Halfedge *ELleftbnd(VPoint *p);
	Halfedge *ELright(Halfedge *he);
	void makevertex(Site *v);

	void PQinsert(Halfedge *he, Site * v, double offset);
	void PQdelete(Halfedge *he);
	bool ELinitialize();
	void ELinsert(Halfedge *lb, Halfedge *newHe);
	Halfedge * ELgethash(int b);
	Halfedge *ELleft(Halfedge *he);
	Site *leftreg(Halfedge *he);
	bool PQinitialize();
	int PQbucket(Halfedge *he);
	void clip_line(Edge *e);
	char *myalloc(unsigned n);
	int right_of(Halfedge *el, VPoint *p);

	Site *rightreg(Halfedge *he);
	Edge *bisect(Site *s1, Site *s2);
	double dist(Site *s, Site *t);

	// GS: 'p' is unused and always ==0 (see also comment by
	//     S. O'Sullivan in the source file), so we remove it
	Site *intersect(Halfedge *el1, Halfedge *el2 /*, VPoint *p=0*/);

	Site *nextone();

	void pushGraphEdge(double x1, double y1, double x2, double y2,
		Site *s1, Site *s2);

	// Gregory Soyez: unused plotting methods
	// void openpl();
	// void circle(double x, double y, double radius);
	// void range(double minX, double minY, double maxX, double maxY);
	// 
	// void out_bisector(Edge *e);
	// void out_ep(Edge *e);
	// void out_vertex(Site *v);
	// void out_site(Site *s);
	// 
	// void out_triple(Site *s1, Site *s2,Site * s3);

	Freelist hfl;
	Halfedge *ELleftend, *ELrightend;
	int ELhashsize;

	int sorted, debug;
	double xmin, xmax, ymin, ymax, deltax, deltay;

	Site *sites;
	int nsites;
	int siteidx;
	int sqrt_nsites;
	int nvertices;
	Freelist sfl;
	Site *bottomsite;

	int nedges;
	Freelist efl;
	int PQhashsize;
	Halfedge *PQhash;
	int PQcount;
	int PQmin;

	int ntry, totalsearch;
	double pxmin, pxmax, pymin, pymax, cradius;
	int total_alloc;

	double borderMinX, borderMaxX, borderMinY, borderMaxY;

	FreeNodeArrayList* allMemoryList;
	FreeNodeArrayList* currentMemoryBlock;

	GraphEdge* allEdges;
	GraphEdge* iteratorEdges;

	double minDistanceBetweenSites;

	static LimitedWarning _warning_degeneracy;
};

int scomp(const void *p1, const void *p2);



#endif


