//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _SPRITE_H_
#include "Sprite.h"
#endif

#ifndef _DGL_H_
#include "graphics/dgl.h"
#endif

#ifndef _STRINGBUFFER_H_
#include "string/stringBuffer.h"
#endif

// Script bindings.
#include "Sprite_ScriptBinding.h"

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(Sprite);

//------------------------------------------------------------------------------

Sprite::Sprite() :
    mFlipX(false),
    mFlipY(false)
{
}

//------------------------------------------------------------------------------

Sprite::~Sprite()
{
}

//------------------------------------------------------------------------------

void Sprite::copyTo(SimObject* object)
{
    // Call to parent.
    Parent::copyTo(object);

    // Cast to sprite.
    Sprite* pSprite = static_cast<Sprite*>(object);

    // Sanity!
    AssertFatal(pSprite != NULL, "Sprite::copyTo() - Object is not the correct type.");

    /// Render flipping.
    pSprite->setFlip( getFlipX(), getFlipY() );
}

//------------------------------------------------------------------------------

void Sprite::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    /// Render flipping.
    addField("FlipX", TypeBool, Offset(mFlipX, Sprite), &writeFlipX, "");
    addField("FlipY", TypeBool, Offset(mFlipY, Sprite), &writeFlipY, "");
}

//------------------------------------------------------------------------------

void Sprite::sceneRender( const SceneRenderState* pSceneRenderState, const SceneRenderRequest* pSceneRenderRequest, BatchRender* pBatchRenderer )
{

	if (mUseCustom == true){
		mRenderOOBB[0] = mCustomPoly[0];
		mRenderOOBB[1].x = mCustomPoly[2].x;
		mRenderOOBB[1].y = mCustomPoly[0].y;
		mRenderOOBB[2].x = mCustomPoly[2].x;
		mRenderOOBB[2].y = mCustomPoly[2].y;
		mRenderOOBB[3].x = mCustomPoly[0].x;
		mRenderOOBB[3].y = mCustomPoly[2].y;

		/*Vector2 position;
		Vector2 wp0 = getWorldPoint(mCustomPoly[0]);
		Vector2 wp2 = getWorldPoint(mCustomPoly[2]);
		position.x = wp0.x - wp2.x;
		position.y = wp0.y - wp2.y;*/
		//setPosition(mCustomPoly[2]);

		CoreMath::mOOBBtoAABB(mRenderOOBB, mCurrentAABB);

		ImageFrameProvider::render(
			getFlipX(), getFlipY(),
			mCustomPoly[0],
			mCustomPoly[1],
			mCustomPoly[2],
			mCustomPoly[3],
			pBatchRenderer);
	}
	else{
		// Let the parent render.
		ImageFrameProvider::render(
			getFlipX(), getFlipY(),
			mRenderOOBB[0],
			mRenderOOBB[1],
			mRenderOOBB[2],
			mRenderOOBB[3],
			pBatchRenderer);
	}
}

//------------------------------------------------------------------------------

void Sprite::setSpritePolyCustom(const char* pCustomPolygon)
{
	//first vert
	F32 x = dAtof(Utility::mGetStringElement(pCustomPolygon, 0));
	F32 y = dAtof(Utility::mGetStringElement(pCustomPolygon, 1));

	mCustomPoly[0].Set(x, y);

	//second vert.
	x = dAtof(Utility::mGetStringElement(pCustomPolygon, 2));
	y = dAtof(Utility::mGetStringElement(pCustomPolygon, 3));

	mCustomPoly[1].Set(x, y);

	//third vert.
	x = dAtof(Utility::mGetStringElement(pCustomPolygon, 4));
	y = dAtof(Utility::mGetStringElement(pCustomPolygon, 5));

	mCustomPoly[2].Set(x, y);

	//fourth vert.
	x = dAtof(Utility::mGetStringElement(pCustomPolygon, 6));
	y = dAtof(Utility::mGetStringElement(pCustomPolygon, 7));

	mCustomPoly[3].Set(x, y);
}


