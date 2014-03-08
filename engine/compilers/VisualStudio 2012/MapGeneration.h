#ifndef _MAP_GENERATION_H_
#define _MAP_GENERATION_H_

#ifndef _SIMBASE_H_  
#include "sim/simBase.h"  
#endif 

#ifndef _TAML_H_  
#include "persistence/taml/taml.h"  
#endif

#ifndef SIMPLEX_H_
#include "perlinNoise/simplexnoise.h"
#endif

#ifndef _VECTOREXT_H
#include "collection\vectorQueue.h"
#endif

#include "graphics/VoronoiDiagramGenerator.h"

class Island : public SimObject
{
private:
	typedef SimObject Parent;

public:
	Island();
	virtual ~Island() {};
	virtual bool onAdd();

	static void initPersistFields();

	void assignBiomes(float x1, float y1, float x2, float y2, float* xValues, float* yValues, int vertcount);
	void assignLandBiome(char* vert);
	//S32 compare(char * const  *a, char* const  *b);
	/*void fillOcean(HashTable<char*, float*> DEdges, float x, float y, int vertCount, float* xValues, float* yValues);
	char* findVert(float x, float y, int vertCount, float* xValues, float* yValues);*/

	DECLARE_CONOBJECT(Island);
};


#endif