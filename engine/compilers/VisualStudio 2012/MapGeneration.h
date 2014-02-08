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

	DECLARE_CONOBJECT(Island);
};


#endif