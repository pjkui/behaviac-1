#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "cocos2d.h"
#include "behaviac/common/container/vector.h"
USING_NS_CC;

class PlaneEnemy;
class PlaneHero;
class NPC;
class Hero;
namespace behaviac{
    class Agent;
}

class GameScene : public Layer
{
public:
    enum NodeTag
    {
        HERO_TAG = 100,
        ENEMY_TAG = 101,
        HERO_BULLET_TAG = 102,
        SCORE_LABEL = 103,
        PAUSE_MENU = 104,
        AI_TAG = 105,
    };

    enum ContactMaskBit
    {
        ENEMY_CONTACTMASKBIT = 0x01 || 0x02,
        HERO_BULLET_CONTACTMASKBIT = 0x01,
        HERO_CONTACTMASKBIT = 0x02
    };

    enum Level
    {
        LEVEL1,
        LEVEL2,
        LEVEL3,
        LEVEL4,
        LEVEL5
    };

    //enum LevelUp_Score
    //{
    //       Level1Up_Score = 0,
    //	Level2Up_Score = 200,
    //	Level3Up_Score = 500,
    //	Level4Up_Score = 1000
    //};

    static const float refresh_delay[]; 

public:
    static Scene* createScene();
    CREATE_FUNC(GameScene);
    bool init();

public:
    static GameScene* sharedGameLayer() { return m_gamelayer; } 

private:
    static GameScene* m_gamelayer; 
public:
    SpriteBatchNode* getBulletBox() { return m_bulletBox; }	 
    void AdjustHeroPosition(Node* hero);
    void gameover();
private:
    SpriteBatchNode *m_bulletBox; 

private:
    int m_level; 
    int m_score; 

private:
    void playBackground(); 
    void publishScore(); 

    void pauseButtonCallBack(Ref* pSender); 
    void AIButtonCallBack(Ref* pSender); 

    bool dealWithContact(PhysicsContact&); 
    void hitEnemy(PlaneEnemy* enemy);
    void hitHero(PlaneHero* hero);

    void levelUp(Level level);
    void testLevel(float dt); 
    bool m_isAI;
    void resetBoss(float dt);
    NPC * m_NPC;
    Hero* m_Hero;
public:
    // //create Update for behaviac
    void Update(float dt);
    static void addBehaviacAgentDeleteQueue(behaviac::Agent* pAgent);
private:
    static void cleanBehaviacAgentDeleteQueue();
private:
    /******************************************************************
    Date        : 2015-11-13 16:40:05
    Author      : Quinn Pan
    Desc        : add some behaviac into the queue , and delete it
    ******************************************************************/
    static behaviac::vector<behaviac::Agent*> m_bt_agent_delete_queue;
};

#endif

