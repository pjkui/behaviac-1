/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tencent is pleased to support the open source community by making behaviac available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at http://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "behaviac/common/base.h"
#include "behaviac/behaviortree/nodes/actions/action.h"
#include "behaviac/behaviortree/nodes/actions/wait.h"
#include "behaviac/agent/agent.h"
#include "behaviac/behaviortree/nodes/conditions/condition.h"
#include "behaviac/common/meta.h"
namespace behaviac
{
    Wait::Wait() : m_time(0)
    {
    }

    Wait::~Wait()
    {
        BEHAVIAC_DELETE(m_time);
    }

    void Wait::load(int version, const char* agentType, const properties_t& properties)
    {
        super::load(version, agentType, properties);

        for (propertie_const_iterator_t it = properties.begin(); it != properties.end(); ++it)
        {
            const property_t& p = (*it);

            if (StringUtils::StringEqual(p.name, "Time"))
            {
                if (StringUtils::IsValidString(p.value))
                {
                    const char* pParenthesis = strchr(p.value, '(');

                    if (pParenthesis == 0)
                    {
                        behaviac::string typeName;
                        //this->m_time_var = Condition::LoadRight(p.value, typeName);
                        this->m_time = AgentMeta::ParseProperty(p.value);
                    }
                    else
                    {
                        //this->m_time = Action::LoadMethod(p.value);
                        this->m_time = AgentMeta::ParseMethod(p.value);
                    }
                }
            }
        }
    }

    double Wait::GetTime(Agent* pAgent) const
    {
        double time = 0;
        if (this->m_time != NULL)
        {
            //return this->m_time_var->GetDoubleValue(pAgent);
            //CInstanceMember::
            //return this->m_time->GetValue(pAgent);
            if (this->m_time)
            {
                int typeNumberId = this->m_time->GetClassTypeNumberId();
                if (typeNumberId == GetClassTypeNumberId<int>())
                {
                    time = *(int*)this->m_time->GetValue(pAgent);
                }
                else if (typeNumberId == GetClassTypeNumberId<double>())
                {
                    time = *(double*)this->m_time->GetValue(pAgent);
                }
                else if (typeNumberId == GetClassTypeNumberId<float>())
                {
                    time = *(float*)this->m_time->GetValue(pAgent);
                }
                else{
                    BEHAVIAC_ASSERT(false);
                }
            }
        }

        return time;
    }

    BehaviorTask* Wait::createTask() const
    {
        WaitTask* pTask = BEHAVIAC_NEW WaitTask();

        return pTask;
    }

    WaitTask::WaitTask() : LeafTask(), m_start(0), m_time(0)
    {
    }

    void WaitTask::copyto(BehaviorTask* target) const
    {
        super::copyto(target);

        BEHAVIAC_ASSERT(WaitTask::DynamicCast(target));
        WaitTask* ttask = (WaitTask*)target;

        ttask->m_start = this->m_start;
        ttask->m_time = this->m_time;
    }

    void WaitTask::save(IIONode* node) const
    {
        super::save(node);

        if (this->m_status != BT_INVALID)
        {
            CIOID  startId("start");
            node->setAttr(startId, this->m_start);

            CIOID  timeId("time");
            node->setAttr(timeId, this->m_time);
        }
    }

    void WaitTask::load(IIONode* node)
    {
        super::load(node);

        if (this->m_status != BT_INVALID)
        {
            CIOID  startId("start");
            behaviac::string attrStr;
            node->getAttr(startId, attrStr);
            StringUtils::ParseString(attrStr.c_str(), this->m_start);

            CIOID  timeId("time");
            node->getAttr(timeId, attrStr);
            StringUtils::ParseString(attrStr.c_str(), this->m_time);
        }
    }

    WaitTask::~WaitTask()
    {
    }

    double WaitTask::GetTime(Agent* pAgent) const
    {
        const Wait* pWaitNode = Wait::DynamicCast(this->GetNode());

        return pWaitNode ? pWaitNode->GetTime(pAgent) : 0;
    }

    bool WaitTask::onenter(Agent* pAgent)
    {
        BEHAVIAC_UNUSED_VAR(pAgent);

        this->m_start = Workspace::GetInstance()->GetTimeSinceStartup() * 1000;
        this->m_time = this->GetTime(pAgent);

        if (this->m_time <= 0)
        {
            return false;
        }

        return true;
    }

    void WaitTask::onexit(Agent* pAgent, EBTStatus s)
    {
        BEHAVIAC_UNUSED_VAR(pAgent);
        BEHAVIAC_UNUSED_VAR(s);
    }

    EBTStatus WaitTask::update(Agent* pAgent, EBTStatus childStatus)
    {
        BEHAVIAC_UNUSED_VAR(pAgent);
        BEHAVIAC_UNUSED_VAR(childStatus);

        double time = Workspace::GetInstance()->GetTimeSinceStartup();

        if (time * 1000 - this->m_start >= this->m_time)
        {
            return BT_SUCCESS;
        }

        return BT_RUNNING;
    }
}
