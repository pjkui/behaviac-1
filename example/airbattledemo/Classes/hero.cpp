#include "hero.h"
#include "PlaneHero.h"
#include "GameScene.h"
#include "enemy.h"
#include "PlaneEnemy.h"
USING_NS_CC;

/******************************************************************
Function    : Hero::Hero()
Date        : 2015-11-18 17:46:09
Author      : Quinn Pan
Parameter   : 
Return      : 
Desc        : construct function
******************************************************************/
Hero::Hero()
{
    m_plane = PlaneHero::create();
    this->btload("hero");
    this->btsetcurrent("hero");
    waitTime = 20;
    m_isAI = true;
	nearestEnemy = 0;
}

Hero::~Hero()
{

}

/******************************************************************
Function    : void findAnNearestEnemy()
Date        : 2015-11-18 10:28:29
Author      : Quinn Pan
Parameter   :
Return      :
Desc        : find an nearest enemy
******************************************************************/
void Hero::findAnNearestEnemy()
{
    m_plane->findAnNearestEnemy();
}

/******************************************************************
Function    : bool Hero::isWillBeCrashOnNearestEnemy()
Date        : 2015-11-18 17:46:54
Author      : Quinn Pan
Parameter   : 
Return      : 
Desc        : tell whether the hero will be crash on the nearest enemy
******************************************************************/
bool Hero::isWillBeCrashOnNearestEnemy()
{
    return m_plane->isWillBeCrashOnNearestEnemy();
}

/******************************************************************
Function    : void Hero::EscapeEnemyCrash()
Date        : 2015-11-18 17:49:09
Author      : Quinn Pan
Parameter   : 
Return      : 
Desc        : escape crash on enemy
******************************************************************/
void Hero::EscapeEnemyCrash()
{
    return m_plane->EscapeEnemyCrash();
}


bool Hero::hasEnemyInFrontCanAttack()
{
    return m_plane->hasEnemyInFrontCanAttack();
}

void Hero::findAnEnemyAndAttack()
{
    m_plane->findAnEnemyAndAttack();
}

void Hero::AdjustHeroPosition()
{
    m_plane->AdjustHeroPosition(this->m_plane);
}
bool Hero::wait20Frame(){
    waitTime--;
    if (waitTime == 0)
    {
        waitTime = 20;
        return true;
    }
    return false;

}

/******************************************************************
Function    : void  Hero::setAI(bool value)
Date        : 2015-11-18 17:51:01
Author      : Quinn Pan
Parameter   : 
Return      : 
Desc        : enable AI 
******************************************************************/
void  Hero::setAI(bool value)
{
    this->m_plane->clearScreenDebugInfo();
    m_isAI = value;
    this->SetActive(value);
    //return m_plane->setAI(value);
}

//AI action
void  Hero::AIAction(float level)
{
    return m_plane->AIAction(level);
}

PlaneHero* Hero::getPlane()
{
    //assert(this->m_plane->getTag() == GameScene::HERO_TAG);
    return m_plane;
}

void Hero::setPlane(PlaneHero* heroPlane)
{
    assert(heroPlane->getTag() == GameScene::HERO_TAG);
    this->m_plane = heroPlane;
}